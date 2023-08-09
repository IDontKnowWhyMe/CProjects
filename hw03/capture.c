#include <stdlib.h>
#include <memory.h>
#include "capture.h"

/* TODO implement the functionality */
#define UNUSED(x) ((void) x)
int insert_to_end(struct capture_t *capture, struct capture_node_t *node) {
    if (capture->head == NULL) {
        capture->head = node;
        return 0;
    }
    struct capture_node_t *temp = capture->head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = node;
    return 0;
}

int deep_copy_node(struct capture_node_t *node,struct capture_node_t *new_node) {
    new_node->packet = malloc(sizeof(struct packet_t));
    if (copy_packet(node->packet, new_node->packet) != PCAP_SUCCESS) {
        destroy_packet(new_node->packet);
        free(new_node->packet);
        return -1;
    }
    new_node->next = NULL;
    return 0;
}

int load_capture(struct capture_t *capture, const char *filename) {
    struct pcap_context context[1];
    if (init_context(context, filename) != PCAP_SUCCESS) {
        return -1;
    }
    if (load_header(context, &capture->header) != PCAP_SUCCESS) {
        destroy_context(context);
        return -1;
    }
    capture->packet_count = 0;
    capture->head = NULL;
    int exit_code;
    struct packet_t* packet = malloc(sizeof (struct packet_t));
    while ((exit_code = load_packet(context, packet)) != PCAP_FILE_END) {
        if (exit_code != PCAP_SUCCESS) {
            destroy_context(context);
            destroy_packet(packet);
            free(packet);
            destroy_capture(capture);
            return -1;
        }
        capture->packet_count += 1;
        struct capture_node_t *node = malloc(sizeof(struct capture_node_t));
        node->packet = malloc(sizeof(struct packet_t));
        copy_packet(packet, node->packet);
        node->next = NULL;
        insert_to_end(capture, node);
        destroy_packet(packet);
    }
    destroy_context(context);
    free(packet);
    return 0;
}

void destroy_capture(struct capture_t *capture) {
    struct capture_node_t *remove = capture->head;
    while (capture->head != NULL) {
        capture->head = capture->head->next;
        destroy_packet(remove->packet);
        free(remove->packet);
        free(remove);
        remove = capture->head;
    }

}

const struct pcap_header_t *get_header(const struct capture_t *const capture) {
    return &capture->header;
}

struct packet_t *get_packet(
        const struct capture_t *const capture,
        size_t index) {
    struct capture_node_t *actual = capture->head;
    for (size_t i = 0; i < index; i++) {
        if (actual == NULL)
            return NULL;
        actual = actual->next;
    }
    return actual->packet;
}

size_t packet_count(const struct capture_t *const capture) {
    return capture->packet_count;
}

size_t data_transfered(const struct capture_t *const capture) {
    struct capture_node_t *actual = capture->head;
    size_t packet_sum = 0;
    while (actual != NULL) {
        packet_sum += actual->packet->packet_header->orig_len;
        actual = actual->next;
    }
    return packet_sum;

}

//FILTER SECTION
uint32_t mask_create(uint8_t mask_length){
    uint32_t mask = 0;
    for (int i = 0; i < mask_length; i++){
        mask = mask << 1;
        mask++;
    }
    return mask;
}

void mask_apply(uint8_t mask_length, uint8_t ip[4], uint8_t masked[4]){
    uint8_t mask[4];
    *(uint32_t*)&mask = mask_create(mask_length);
    for (int i = 0; i < 4; i++){
        masked[i] = ip[i] & mask[i];
    }

}

bool mask_check(uint8_t mask_length, uint8_t ip_1[4], uint8_t ip_2[4]){
    uint8_t masked_1[4];
    uint8_t masked_2[4];
    mask_apply(mask_length, ip_1, masked_1);
    mask_apply(mask_length, ip_2, masked_2);
    return memcmp(masked_1, masked_2, 4*sizeof(uint8_t)) == 0;
}

int filtered_insert_copy(struct capture_t *filtered, struct capture_node_t *actual){
    filtered->packet_count += 1;
    struct capture_node_t *node_copy = malloc(sizeof (struct capture_node_t));
    if (deep_copy_node(actual, node_copy) == -1) {
        destroy_capture(filtered);
        destroy_packet(node_copy->packet);
        free(node_copy->packet);
        free(node_copy);
        return -1;
    }
    insert_to_end(filtered, node_copy);
    return 0;
}


void init_filtered(struct capture_t *filtered,
                   const struct capture_t *const original){
    filtered->header = original->header;
    filtered->head = NULL;
    filtered->packet_count = 0;
}

int filter_protocol(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint8_t protocol) {
    init_filtered(filtered, original);
    struct capture_node_t *actual = original->head;
    while (actual != NULL) {
        if (actual->packet->ip_header->protocol == protocol) {
            if(filtered_insert_copy(filtered, actual) != 0)
                return -1;
        }
        actual = actual->next;
    }
    return 0;
}

int filter_larger_than(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint32_t size) {
    init_filtered(filtered, original);
    struct capture_node_t *actual = original->head;
    while (actual != NULL) {
        if (actual->packet->packet_header->orig_len > size) {
            if(filtered_insert_copy(filtered, actual) != 0)
                return -1;
        }
        actual = actual->next;
    }
    return 0;
}

int filter_from_to(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint8_t source_ip[4],
        uint8_t destination_ip[4]) {
    init_filtered(filtered, original);
    struct capture_node_t *actual = original->head;
    while (actual != NULL) {
        uint8_t *control_src = actual->packet->ip_header->src_addr;
        uint8_t *control_dst = actual->packet->ip_header->dst_addr;
        if (memcmp(control_src, source_ip, 4 * sizeof(uint8_t)) == 0 &&
            memcmp(control_dst, destination_ip, 4 * sizeof(uint8_t)) == 0) {
            if(filtered_insert_copy(filtered, actual) != 0)

                return -1;
        }
        actual = actual->next;
    }
    return 0;
}


int filter_from_mask(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint8_t network_prefix[4],
        uint8_t mask_length) {
    init_filtered(filtered, original);
    struct capture_node_t *actual = original->head;
    while (actual != NULL) {
        if (mask_check(mask_length, actual->packet->ip_header->src_addr, network_prefix)) {
            if(filtered_insert_copy(filtered, actual) != 0)
                return -1;
        }
        actual = actual->next;
    }
    return 0;
}

int filter_to_mask(
        const struct capture_t *const original,
        struct capture_t *filtered,
        uint8_t network_prefix[4],
        uint8_t mask_length) {
    init_filtered(filtered, original);
    struct capture_node_t *actual = original->head;
    while (actual != NULL) {
        if (mask_check(mask_length, actual->packet->ip_header->dst_addr, network_prefix)) {
            if(filtered_insert_copy(filtered, actual) != 0)
                return -1;
        }
        actual = actual->next;
    }
    return 0;
}

// FLOW

struct flow{
    uint8_t from[4];
    uint8_t to[4];
    bool set;
};

void empty_flow_list(struct flow* flow_list, size_t length){
    for (size_t i = 0; i < length; i++){
        flow_list[i].set = false;
        memset(flow_list[i].from, 0, 4 * sizeof (uint8_t));
        memset(flow_list[i].to, 0, 4 * sizeof (uint8_t));
    }
}

bool used_flow(struct flow flow, struct flow* used, size_t size){
    for (size_t i = 0; i < size; i++){
        if (used[i].set &&
            memcmp(flow.from, used[i].from, 4 * sizeof (uint8_t)) == 0 &&
            memcmp(flow.to, used[i].to, 4 * sizeof (uint8_t)) == 0){
            return true;
        }
    }
    return false;
}

int print_single_flow(const struct capture_t *const capture, struct flow flow){
    struct capture_t* filtered = malloc(sizeof(struct capture_t));
    if (filter_from_to(capture, filtered, flow.from, flow.to) != 0){
        free(filtered);
        return -1;
    }
    printf("%u.%u.%u.%u -> %u.%u.%u.%u : %ld\n", flow.from[0], flow.from[1],
                                                flow.from[2], flow.from[3],
                                                flow.to[0], flow.to[1],
                                                flow.to[2], flow.to[3],
                                                filtered->packet_count);
    free(filtered);
    return 0;
}

int print_flow_stats(const struct capture_t *const capture) {
    struct flow used[capture->packet_count];
    size_t free_index = 0;
    empty_flow_list(used, capture->packet_count);
    struct capture_node_t *actual = capture->head;
    while (actual != NULL){
        struct flow actual_flow;
        actual_flow.set = true;
        memcpy(actual_flow.from, actual->packet->ip_header->src_addr, 4* sizeof (uint8_t));
        memcpy(actual_flow.to, actual->packet->ip_header->dst_addr, 4* sizeof (uint8_t));
        if (used_flow(actual_flow, used, capture->packet_count)){
            actual = actual->next;
            continue;
        }
        used[free_index].set = true;
        memcpy(used[free_index].from, actual_flow.from, 4 * sizeof (uint8_t));
        memcpy(used[free_index].to, actual_flow.to, 4 * sizeof (uint8_t));
        free_index++;
        if(print_single_flow(capture, actual_flow) == -1){
            return -1;
        }
        actual = actual->next;
    }
    return 0;
}

int print_longest_flow(const struct capture_t *const capture) {
    UNUSED(capture);
    return -1;
}
