#include "sim.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "malloc.h"
#include "errno.h"
#include "string_number.h"

void print_prop(Properties prop){
    printf("+---------------------------+\n");
    printf("|         PROPERTIES        |\n");
    printf("+---------------------------+\n");
    printf("  Lethality: %f\n", prop.lethality);
    printf("  Infectivity: %f\n", prop.infectivity);
    printf("  Duration: %f\n", prop.infectivity);
    printf("  Vaccine-modifier: %f\n", prop.vaccine_modifier);
    printf("  Max-steps: %d\n", prop.max_steps);
    printf("  Seed: %d\n", prop.seed);
    if (prop.verbose == 0)
        printf("  Verbose: OFF\n");
    else
        printf("  Verbose: ON\n");

}

void print_world(World world){
    printf("+------------------------------------------------------------------------------+\n");
    printf("|                                     AGENTS                                   |\n");
    printf("+------------------------------------------------------------------------------+\n");
    for (int i = 0; i < world.agent_count; i++){
        printf("| ID: %d | IS VACCINATED: %d | IMMUNITY: %f | INFECTED: %d | LOCATION: %d |\n",
               world.agents[i].id, world.agents[i].is_vaccinated,
               world.agents[i].immunity, world.agents[i].is_infected, world.agents[i].place_id);
    }
    printf("+-------------------------------------------------------------------------------+\n");
    for (int i = 0; i < world.place_count; i++){
        printf("%d: ", world.places[i].id);
        for (int j = 0; j < world.agent_count; j++){
            if (world.agents[j].place_id == world.places[i].id){
                printf("%d ", world.agents[j].id);
            }
        }
        printf("\n");
    }

}

int count_lines(char *path){
    if (path == NULL){
        fprintf(stderr, "No PATH");
        return -1;
    }
    int count = 0;
    FILE *fp;
    char c;
    fp = fopen(path,"r");
    if (fp == NULL){
        perror("fopen");
        return -1;
    }
    for (c = getc(fp); c != EOF; c = getc(fp)){
        if (c == '\n'){
            count++;
        }
    }
    fclose(fp);
    return count;
}

Place fill_place(char *string, int *error) {
    Place place;
    int token_count = 0;
    char *token;
    token = strtok(string, ";");
    while (token != NULL){
        switch (token_count) {
            case 0:
                if (is_string_float(token) != 1){
                    *error = -1;
                }
                place.id = strtol(token,NULL, 10);
                break;
            case 1:
                strcpy(place.name, token);
                break;
            case 2:
                place.exposure = strtof(token, NULL);
                break;
        }
        token = strtok(NULL, ";");
        token_count++;
    }
    if (token_count != 3)
        *error = -1;
    place.infected = 0;
    return place;
}

int rout_length(char* rout){
    int len = strlen(rout);
    int count = 0;
    for (int i = 0; i < len; i++){
        if (rout[i] == '-')
            count++;
    }
    return count+1;
}

Agent fill_agent(char *string, int *error) {
    Agent agent;
    int token_count = 0;
    char *token;
    token = strtok(string, ";");
    while (token != NULL){
        switch (token_count) {
            case 0:
                if (is_string_float(token) != 1){
                    *error = -1;
                }
                agent.id = strtol(token,NULL, 10);
                break;
            case 1:
                strcpy(agent.route, token);
                break;
            case 2:
                agent.is_vaccinated = strtol(token, NULL, 10);
                break;
            case 3:
                agent.immunity = strtof(token, NULL);
                break;
            case 4:
                agent.is_infected = strtol(token, NULL, 10);
                break;
        }
        token = strtok(NULL, ";");
        token_count++;
    }
    if (token_count != 5)
        *error = -1;
    int *rout_list = rout_to_id_list(agent.route, rout_length(agent.route));
    agent.place_id = rout_list[0];
    free(rout_list);
    agent.position = 0;
    return agent;
}

void parse_lines(char *path, World world, Object obj, int obj_count){
    int *used_id = malloc(sizeof (int) * obj_count);
    FILE *file;
    file = fopen(path, "r");
    if (!file){
        perror("fopen");
        free(world.places);
        free(world.agents);
        exit(-1);
    }
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int count = 0;
    while ((read = getline(&line, &len, file)) != -1){
        int error = 0;
        if (obj == place){
            Place place = fill_place(line, &error);
            if (error == -1){
                fprintf(stderr, "Invalid line in file");
                free(world.places);
                free(world.agents);
                fclose(file);
                free(used_id);
                exit(-1);
            }
            if (in_list(used_id, count,place.id)){
                fprintf(stderr, "Duplicate ID");
                free(world.places);
                free(world.agents);
                fclose(file);
                free(used_id);
                exit(-1);
            }
            used_id[count] = place.id;
            world.places[count] = place;
        }
        if (obj == agent){
            Agent agent = fill_agent(line, &error);
            if (error == -1){
                fprintf(stderr, "Invalid line in agent file");
                free(world.places);
                free(world.agents);
                fclose(file);
                free(used_id);
                exit(-1);
            }
            if (in_list(used_id, count,agent.id)){
                fprintf(stderr, "Duplicate ID");
                free(world.places);
                free(world.agents);
                fclose(file);
                free(used_id);
                exit(-1);
            }
            used_id[count] = agent.id;
            world.agents[count] = agent;
        }
        count ++;
    }
    if (line)
        free(line);
    free(used_id);
    fclose(file);

}

World world_init(char *world_path, char *agent_path){
    World  world;
    int place_count = count_lines(world_path);
    int agent_count = count_lines(agent_path);
    if (place_count == -1 || agent_count == -1){
        exit(-1);
    }
    if (place_count == 0){
        fprintf(stderr, "World file is EMPTY!");
        exit(-1);
    }
    if (agent_count == 0){
        fprintf(stderr, "Agents file is EMPTY!");
        exit(-1);
    }
    world.places = malloc(sizeof (Place) * place_count);
    if (!world.places) {
        perror("malloc");
        exit(-1);
    }
    world.agents = malloc(sizeof (Agent) * agent_count);
    if (!world.agents) {
        perror("malloc");
        exit(-1);
    }
    world.place_count = place_count;
    world.agent_count = agent_count;
    parse_lines(world_path,world, place, place_count);
    parse_lines(agent_path,world, agent, agent_count);
    return world;
}

int *rout_to_id_list(char *rout, int size){
    char *rout_copy = malloc(sizeof(char) * strlen(rout)+1);
    strcpy(rout_copy, rout);
    int *id_list = malloc(sizeof (int ) * size);
    int place_id;
    int i = 0;
    char *token;
    token = strtok(rout_copy, "-");
    while (token != NULL){
        place_id = strtol(token, NULL, 10);
        id_list[i] = place_id;
        token = strtok(NULL, "-");
        i++;
    }
    free(rout_copy);
    return id_list;
}

void move_phase(World world){
    for (int i = 0; i < world.agent_count; i++){
        if (world.agents[i].is_infected == -1)
            continue;
        int len = rout_length(world.agents[i].route);
        int *rout_list = rout_to_id_list(world.agents[i].route, len);
        world.agents[i].position++;
        if(world.agents[i].position >= len)
            world.agents[i].position = 0;
        world.agents[i].place_id = rout_list[world.agents[i].position];
        free(rout_list);
    }
}

void progress_phase(World world, Properties prop){
    for (int i = 0; i < world.place_count; i++){
        for (int j = 0; j < world.agent_count; j++){
            if (world.agents[j].place_id != world.places[i].id)
                continue;
            if (world.agents[j].is_infected != 1)
                continue;
            double roll = (double) rand() / RAND_MAX;
            if (world.agents[j].is_vaccinated == 1)
                roll = roll * prop.vaccine_modifier;
            if (roll < prop.lethality) {
                world.agents[j].is_infected = -1;
                if (prop.verbose == 1)
                    printf("Agent %d has died at %s.\n", world.agents[j].id, world.places[i].name);
                continue;
            }
            roll = (double) rand() / RAND_MAX;
            if (roll > prop.duration) {
                world.agents[j].is_infected = 0;
                if (prop.verbose == 1)
                    printf("Agent %d recovered at %s.\n", world.agents[j].id, world.places[i].name);
            }

        }
    }
}

int count_of_agents_in_place(World world, int place_index){
    int count = 0;
    for(int i = 0; i < world.agent_count; i++){
        if (world.agents[i].is_infected == -1)
            continue;
        if (world.places[place_index].id == world.agents[i].place_id){
            count++;
        }
    }
    return count;
}

void fill_place_agents(World world, Agent *agents, int place_index){
    int count = 0;
    for(int i = 0; i < world.agent_count; i++){
        if (world.agents[i].is_infected == -1)
            continue;
        if (world.places[place_index].id == world.agents[i].place_id){
            agents[count] = world.agents[i];
            count++;
        }
    }
}

int in_list(int  *list, int count, int check_item){
    for (int i = 0; i < count; i++){
        if (list[i] == check_item)
            return 1;
    }
    return 0;
}

void set_agent_by_id(World world, int id){
    for (int i = 0; i < world.agent_count; i++){
        if (world.agents[i].id == id){
            world.agents[i].is_infected = 1;
        }
    }
}

void infection_phase(World world, Properties prop){
    for (int i = 0; i < world.place_count; i++){
        int count = count_of_agents_in_place(world, i);
        Agent *agents = malloc(sizeof (Agent) * count);
        fill_place_agents(world, agents, i);
        int used_id[count];
        int used_pointer = 0;
        for (int j = 0; j < count; j++){
            if (agents[j].is_infected == 0){
                continue;
            }
            if (in_list(used_id, used_pointer, agents[j].id))
                continue;
            for (int k = 0; k < count; k++){
                if (agents[k].is_infected == 1)
                    continue;
                double roll = (double) rand() / RAND_MAX;
                roll *= world.places[i].exposure;
                roll *= prop.infectivity;
                double new_immunity = agents[k].immunity;
                if (agents[k].is_vaccinated == 1)
                    new_immunity *= prop.vaccine_modifier;
                if (roll > new_immunity){
                    set_agent_by_id(world, agents[k].id);
                    agents[k].is_infected = 1;
                    used_id[used_pointer] = agents[k].id;
                    used_pointer++;
                    world.places[i].infected++;
                    if (prop.verbose == 1)
                        printf("Agent %d has infected agent %d at %s.\n", agents[j].id, agents[k].id, world.places[i].name);
                }
            }
        }
        free(agents);
    }
}

int world_check(World world, int *total_deaths){
    int alive = 0;
    *total_deaths = 0;
    for (int i = 0; i < world.agent_count; i++){
        if (world.agents[i].is_infected == -1)
            *total_deaths = *total_deaths + 1;
        else if (world.agents[i].is_infected == 1)
            return -1;
        else if (world.agents[i].is_infected == 0)
            alive++;
    }
    if (alive != 0)
        return 1;
    return 0;
}

int overall_infected(World world, int *total_infected, int *inf){
    int max = 0;
    int max_count = 0;
    int max_index = -1;
    for (int i = 0; i < world.place_count; i++){
        *total_infected = *total_infected + world.places[i].infected;
        if (world.places[i].infected > max){
            max_count = 0;
            max = world.places[i].infected;
            max_index = i;
            *inf = max;
        }
        else if(world.places[i].infected == max){
            max_count++;
        }

    }
    if (max_count != 0){
        *inf = -1;
        return -1;
    }
    return max_index;

}


