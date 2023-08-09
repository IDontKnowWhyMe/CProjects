#ifndef HW06_SIM_H
#define HW06_SIM_H

typedef enum {
    place, agent
}Object;

typedef struct {
    float lethality;
    float infectivity;
    float duration;
    float vaccine_modifier;
    int max_steps;
    int seed;
    int verbose;
} Properties;

typedef struct {
    int position;
    int place_id;
    int id;
    char route[8192];
    int is_vaccinated;
    float immunity;
    int is_infected;
} Agent;

typedef struct {
    int id;
    char name[128];
    float exposure;
    int infected;
} Place;


typedef struct {
    int place_count;
    int agent_count;
    Place *places;
    Agent *agents;
} World;

void print_prop(Properties prop);

int in_list(int  *list, int count, int check_item);
int *rout_to_id_list(char *rout, int size);
void infection_phase(World world, Properties prop);
void progress_phase(World world, Properties prop);
World world_init(char *world_path, char *agent_path);
void print_world(World world);
void move_phase(World world);
int overall_infected(World world, int *total_infected, int *inf);
int world_check(World world, int *total_deaths);

#endif //HW06_SIM_H
