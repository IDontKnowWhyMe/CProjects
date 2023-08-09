#include <stdlib.h>
#include <string.h>

#include "capture.h"

int main(int argc, char *argv[])
{
    if (argc != 5){
        return EXIT_FAILURE;
    }
    struct capture_t *capture = malloc(sizeof (struct capture_t));
    if(load_capture(capture, argv[1]) == -1){
        destroy_capture(capture);
        free(capture);
        return  EXIT_FAILURE;
    }
    uint8_t from_ip[4];
    uint8_t to_ip[4];
    uint8_t mask_from_l;
    uint8_t mask_to_l;
    sscanf(argv[2], "%hhu.%hhu.%hhu.%hhu/%hhu", &from_ip[0], &from_ip[1], &from_ip[2],&from_ip[3], &mask_from_l);
    sscanf(argv[3], "%hhu.%hhu.%hhu.%hhu/%hhu", &to_ip[0], &to_ip[1], &to_ip[2],&to_ip[3], &mask_to_l);
    struct capture_t* filtered = malloc(sizeof (struct capture_t));
    if (filter_from_mask(capture, filtered, from_ip, mask_from_l) == -1){
        destroy_capture(capture);
        destroy_capture(filtered);
        free(filtered);
        free(capture);
        return EXIT_FAILURE;
    }
    struct capture_t* filtered_2 = malloc(sizeof (struct capture_t));
    if (filter_to_mask(filtered, filtered_2, to_ip, mask_to_l) == -1){
        destroy_capture(capture);
        destroy_capture(filtered);
        destroy_capture(filtered_2);
        free(filtered);
        free(filtered_2);
        free(capture);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[4], "flowstats") == 0)
    {
        print_flow_stats(filtered_2);
    }
    destroy_capture(capture);
    destroy_capture(filtered);
    destroy_capture(filtered_2);
    free(filtered);
    free(filtered_2);
    free(capture);
    return EXIT_SUCCESS;
}
