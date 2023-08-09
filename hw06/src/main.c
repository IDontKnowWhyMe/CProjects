#include <stdio.h>
#include "sim.h"
#include "time.h"
#include "getopt.h"
#include "string_number.h"
#include "stdlib.h"

#define UNUSED(var) \
    ((void) var)

int main(int argc, char *argv[])
{
    char *world_path = NULL;
    char *agent_path = NULL;
    Properties prop = {0.5,0.5,0.5,1.2,-1,time(NULL), 0};
    int c;
    if (argc < 3){
        fprintf(stderr, "Invalid arg\n");
        return -1;
    }
    for (int i = 1; i < argc - 2;i++){
        if (argv[i][0] == '-' && argv[i][1] != '-'){
            fprintf(stderr, "Invalid arg\n");
            return -1;
        }
    }
    while (1) {
        static struct option long_options[]={
                {"lethality", required_argument, 0, 'l'},
                {"infectivity", required_argument, 0, 'i'},
                {"duration", required_argument, 0, 'd'},
                {"vaccine-modifier", required_argument, 0, 'v'},
                {"max-steps", required_argument, 0, 'm'},
                {"random-seed", required_argument, 0, 's'},
                {"verbose", no_argument, 0, 'p'},
                {0,0,0,0}
        };
        int option_index = 0;
        c = getopt_long_only(argc, argv, "l:i:d:v:m:s:p", long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
            case 'l':
                if (is_string_float(optarg) == -1){
                    fprintf(stderr, "lethality parameter is not float");
                    return -1;
                }
                prop.lethality = strtof(optarg, NULL);
                if (prop.lethality > 1 || prop.lethality < 0){
                    fprintf(stderr, "lethality parameter should be from range [0,1]");
                    return -1;
                }
                break;
            case 'i':
                if (is_string_float(optarg) == -1){
                    fprintf(stderr, "infectivity parameter is not float");
                    return -1;
                }
                prop.infectivity = strtof(optarg, NULL);
                if (prop.infectivity > 1 || prop.infectivity < 0){
                    fprintf(stderr, "infectivity parameter should be from range [0,1]");
                    return -1;
                }
                break;
            case 'd':
                if (is_string_float(optarg) == -1){
                    fprintf(stderr, "duration parameter is not float");
                    return -1;
                }
                prop.duration = strtof(optarg, NULL);
                if (prop.duration > 1 || prop.duration < 0){
                    fprintf(stderr, "duration parameter should be from range [0,1]");
                    return -1;
                }
                break;
            case 'v':
                if (is_string_float(optarg) == -1){
                    fprintf(stderr, "vaccine-modifier parameter is not float");
                    return -1;
                }
                prop.vaccine_modifier = strtof(optarg, NULL);
                if (prop.vaccine_modifier < 0){
                    fprintf(stderr, "vaccine-modifier parameter should be greater then 0");
                    return -1;
                }
                break;
            case 'm':
                if (is_string_float(optarg) != 1){
                    fprintf(stderr, "max-steps parameter is not int");
                    return -1;
                }
                prop.max_steps = strtol(optarg, NULL, 10);
                if (prop.max_steps < 0) {
                    fprintf(stderr, "max-steps parameter should be greater then 0");
                    return -1;
                }
                break;
            case 's':
                if (is_string_float(optarg) != 1){
                    fprintf(stderr, "seed parameter is not int");
                    return -1;
                }
                prop.seed = strtol(optarg, NULL, 10);
                break;
            case 'p':
                prop.verbose = 1;
                break;
            default:
                fprintf(stderr, "Invalid parameter");
                return -1;
        }

    }
    if (argc - optind == 1){
        world_path = argv[optind];
    }
    else if(argc - optind == 2){
        world_path = argv[argc - 1];
        agent_path = argv[optind];
    }
    else if (argc - optind > 2){
        fprintf(stderr, "Unexpected number of parameters");
    }
    World world;
    world = world_init(world_path, agent_path);
    int iteration = 0;
    int total_deaths = 0;
    int total_infections = 0;
    srand(prop.seed);
    printf("Random seed: %d\n", prop.seed);
    while (1){
        iteration++;
        if (prop.max_steps != -1 && iteration > prop.max_steps){
            printf("Step limit expired.\n");
            break;
        }
        int check = world_check(world, &total_deaths);
        if (check == 1){
            printf("Virus is extinct.\n");
            break;
        } else if (check == 0){
            printf("Population is extinct.\n");
            break;
        }
        if (prop.verbose == 1)
            printf("\n*** STEP %d ***\n", iteration);
        move_phase(world);
        progress_phase(world, prop);
        infection_phase(world, prop);

    }
    int max_index;
    int infec = 0;
    max_index = overall_infected(world, &total_infections, &infec);
    printf("Statistics:\n");
    printf("\tTotal infections: %d\n", total_infections);
    printf("\tTotal deaths: %d\n", total_deaths);
    printf("\tNumber of survivors: %d\n", world.agent_count - total_deaths);
    printf("Most infectious location:\n");
    if (infec != -1)
        printf("\t- %s: %d infections\n", world.places[max_index].name, infec);
    else
        printf("\tMultiple\n");
    printf("Simulation terminated after %d steps.\n", iteration-1);
    free(world.places);
    free(world.agents);
    return 0;
}
