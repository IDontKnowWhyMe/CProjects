#ifndef HW05_PREM_CHECK_H
#define HW05_PREM_CHECK_H
#include <stdio.h>
#include <dirent.h>

typedef enum {export, import} switcher;

typedef struct {
    switcher switcher;
    char *prem_path;
    char *dir_path;
} settings;

void init_settings(int argc, char** argv, settings *settings);

int export_mode(settings settings);

char *get_owner(char *path);

char *get_group(char *path);

void set_permissions(char *path, sample_entry *entry);

void set_entry(sample_entry *entry, char* filename, char *file_path);

int sort_files(const char *name, FILE *file, char *filename);

void dir_sort(struct dirent **namelist, int n, const char *path);

void flag_b(char *path, sample_entry *entry);

void clear_settings(settings *settings);

int import_mode(settings settings);

int parse_input(FILE *file, char *root);

int partial_parse(FILE *file, sample_entry *entry);

int set_file(char *path, sample_entry entry);

int chmod_set(char *path, sample_entry entry);

#endif //HW05_PREM_CHECK_H
