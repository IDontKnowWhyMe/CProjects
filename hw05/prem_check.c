#include "sample_printer.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "prem_check.h"
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

void set_entry(sample_entry *entry, char* filename, char *file_path){
    entry->file_name = filename;
    entry->owner = get_owner(file_path);
    entry->group = get_group(file_path);
    set_permissions(file_path, entry);
    flag_b(file_path, entry);
}

int export_mode(settings settings){
    sample_entry entry;
    entry.file_name = ".";
    entry.owner = get_owner(settings.dir_path);
    entry.group = get_group(settings.dir_path);
    if (entry.owner == NULL || entry.group == NULL){
        fprintf(stderr, "Invalid dir path\n");
        return 1;
    }
    set_permissions(settings.dir_path, &entry);
    flag_b(settings.dir_path, &entry);
    FILE* permissions_file = fopen(settings.prem_path, "w");
    if (!permissions_file) {
        perror("fopen");
        return 1;
    }
    print_sample_entry(permissions_file, &entry);
    if (sort_files(settings.dir_path, permissions_file, "") != 0){
        fclose(permissions_file);
        return 1;
    }
    fclose(permissions_file);
    return 0;
}

char *get_owner(char *path){
    struct stat info;
    if (stat(path, &info)<0){
        return NULL;
    }
    struct passwd *owner = getpwuid(info.st_uid);
    return owner->pw_name;
}

char *get_group(char *path){
    struct stat info;
    if (stat(path, &info) < 0){
        return NULL;
    }
    struct group *group = getgrgid(info.st_gid);
    return group->gr_name;
}

void set_permissions(char *path, sample_entry *entry){
    char buffer[10];
    struct stat info;
    stat(path, &info);
    if( info.st_mode & S_IRUSR )
        buffer[0] = 'r';
    else
        buffer[0] = '-';
    if( info.st_mode & S_IWUSR )
        buffer[1] = 'w';
    else
        buffer[1] = '-';
    if( info.st_mode & S_IXUSR )
        buffer[2] = 'x';
    else
        buffer[2] = '-';
    if( info.st_mode & S_IRGRP )
        buffer[3] = 'r';
    else
        buffer[3] = '-';
    if( info.st_mode & S_IWGRP )
        buffer[4] = 'w';
    else
        buffer[4] = '-';
    if( info.st_mode & S_IXGRP )
        buffer[5] = 'x';
    else
        buffer[5] = '-';
    if( info.st_mode & S_IROTH )
        buffer[6] = 'r';
    else
        buffer[6] = '-';
    if( info.st_mode & S_IWOTH )
        buffer[7] = 'w';
    else
        buffer[7] = '-';
    if( info.st_mode & S_IXOTH )
        buffer[8] = 'x';
    else
        buffer[8] = '-';
    buffer[9] = '\0';
    strcpy(entry->permissions, buffer);
}

void free_namelist(struct dirent **namelist, int start, int end){
    for (int i = start + 1; i <= end; i++){
        free(namelist[i]);
    }
}

int sort_files(const char *name, FILE *file, char *filename){
    struct dirent **namelist;
    int n;
    struct sample_entry data;
    n = scandir(name, &namelist, 0, alphasort);
    int n_copy = n - 1;
    if (n < 0){
        perror("scandir");
        return 1;
    }
    else {
        dir_sort(namelist, n, name);
        while (n--) {
            if (strcmp(namelist[n_copy - n]->d_name, ".") == 0 || strcmp(namelist[n_copy - n]->d_name, "..") == 0){
                free(namelist[n_copy - n]);
                continue;
            }
            struct stat statbuff;
            char file_path[1024];
            char n_file_name[1024];
            snprintf(file_path, sizeof (file_path), "%s/%s", name, namelist[n_copy - n]->d_name);
            if (strcmp(filename, "") != 0)
                snprintf(n_file_name, sizeof (n_file_name), "%s/%s", filename, namelist[n_copy - n]->d_name);
            else
                snprintf(n_file_name, sizeof (n_file_name), "%s", namelist[n_copy - n]->d_name);
            stat(file_path, &statbuff);
            set_entry(&data, n_file_name, file_path);
            free(namelist[n_copy - n]);
            if (S_ISDIR(statbuff.st_mode)){
                fprintf(file, "\n");
                print_sample_entry(file, &data);
                if(sort_files(file_path, file, n_file_name) == 1){
                    free_namelist(namelist, n_copy - n, n);
                    free(namelist);
                    return 1;
                }
            }
            else if(S_ISREG(statbuff.st_mode)) {
                fprintf(file, "\n");
                print_sample_entry(file, &data);
            }
            else{
                fprintf(stderr, "Invalid file type");
                free_namelist(namelist, n_copy - n, n);
                free(namelist);
                return 1;
            }
        }
        free(namelist);
    }
    return 0;
}


void dir_sort(struct dirent **namelist, int n, const char *path){
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n - 1; j++){
            char new_path1[1024];
            char new_path2[1024];
            struct stat statbuff1;
            struct stat statbuff2;
            snprintf(new_path1, sizeof (new_path1), "%s/%s", path, namelist[j]->d_name);
            snprintf(new_path2, sizeof (new_path2), "%s/%s", path, namelist[j + 1]->d_name);
            stat(new_path1, &statbuff1);
            stat(new_path2, &statbuff2);
            if (S_ISREG(statbuff1.st_mode) && S_ISDIR(statbuff2.st_mode)){
                struct dirent *temp = namelist[j];
                namelist[j] = namelist[j+1];
                namelist[j+1] = temp;
            }
        }
    }
}

void init_settings(int argc, char** argv, settings *settings){
    settings->prem_path = NULL;
    if (argc == 1){
        fprintf(stderr, "Wrong number of arguments\n");
        goto end_with_help;
    }
    if(argc == 2){
        if (strcmp(argv[1], "-i") == 0)
            fprintf(stderr, "option requires an argument -- 'i'\n");
        else if (strcmp(argv[1], "-e") == 0)
            fprintf(stderr, "option requires an argument -- 'e'\n");
        else if (argv[1][0] == '-' && strlen(argv[1]) == 2)
            fprintf(stderr, "invalid option -- '%c'\n", argv[1][1]);
        else
            fprintf(stderr, "Wrong number of arguments given.\n");
        goto end_with_help;
    }
    if (argc > 4){
        fprintf(stderr, "Wrong number of arguments given.\n");
        goto end_with_help;
    }
    else if (argc >= 3){
        if (strcmp(argv[1], "-e") == 0)
            settings->switcher = export;
        else if (strcmp(argv[1], "-i") == 0)
            settings->switcher = import;
        else if (argv[1][0] == '-' && strlen(argv[1]) == 2){
            fprintf(stderr, "invalid option -- '%c'\n", argv[1][1]);
            goto end_with_help;
        }
        else{
            fprintf(stderr, "Wrong number of arguments given.\n");
            goto end_with_help;
        }
        settings->prem_path = malloc(strlen(argv[2]) * sizeof (char) + 1);
        strcpy(settings->prem_path, argv[2]);
        if (argc == 4){
            settings->dir_path = malloc(strlen(argv[3]) * sizeof (char) + 1);
            strcpy(settings->dir_path, argv[3]);
        } else{
            char buffer[1024];
            if (!getcwd(buffer, 1024)){
                perror("getcwd");
                clear_settings(settings);
                free(settings);
                exit(EXIT_FAILURE);
            }
            settings->dir_path = malloc(sizeof (char) * strlen(buffer) + 1 );
            strcpy(settings->dir_path, buffer);
        }
    }
    return;
end_with_help:
    fprintf(stdout, "Modes of operation:\n"
                    " -e, --export <PERMISSIONS_FILE>   read and save permissions\n"
                    " -i, --import <PERMISSIONS_FILE>   compare and correct permissions\n");
    clear_settings(settings);
    free(settings);
    exit(EXIT_FAILURE);
}

void flag_b(char *path, sample_entry *entry){
    char buffer[4];
    struct stat info;
    stat(path, &info);
    if( info.st_mode & S_ISUID )
        buffer[0] = 's';
    else
        buffer[0] = '-';
    if( info.st_mode & S_ISGID )
        buffer[1] = 's';
    else
        buffer[1] = '-';
    if( info.st_mode & 512)
        buffer[2] = 't';
    else
        buffer[2] = '-';
    buffer[3] = '\0';
    strcpy(entry->flags, buffer);
}

void clear_settings(settings *settings){
    if (settings == NULL)
        return;
    if (settings->prem_path != NULL){
        free(settings->prem_path);
    }
    if (settings->dir_path != NULL){
        free(settings->dir_path);
    }
}

int import_mode(settings settings){
    FILE* permissions_file = fopen(settings.prem_path, "r");
    if (!permissions_file) {
        perror("fopen");
        return 1;
    }
    parse_input(permissions_file, settings.dir_path);
    fclose(permissions_file);
    return 0;

}

int parse_input(FILE *file, char *root){
    sample_entry entry;
    entry.file_name = malloc(sizeof (char) * 1024);
    entry.owner = malloc(sizeof (char) * 32);
    entry.group = malloc(sizeof (char) * 32);
    while (partial_parse(file, &entry) == 1){
        int len = strlen(entry.file_name) + strlen(root) + 2;
        char full_p[len];
        snprintf(full_p, len, "%s/%s", root, entry.file_name);
        if (strcmp(entry.file_name, ".") == 0){
            set_file(root, entry);

        } else{
            set_file(full_p, entry);
        }
    }
    free(entry.file_name);
    free(entry.owner);
    free(entry.group);
    return 0;
}

int set_file(char *path, sample_entry entry){
    char *owner = get_owner(path);
    char *group = get_group(path);
    if (owner == NULL || group == NULL)
        return 1;
    if (strcmp(owner, entry.owner) != 0){
        fprintf(stderr, "User of file %s is incorrect\n", entry.file_name);
    }
    if (strcmp(group, entry.group) != 0){
        fprintf(stderr, "Group of file %s is incorrect\n", entry.file_name);
    }
    chmod_set(path, entry);
    return 0;
}

int chmod_set(char *path, sample_entry entry){
    mode_t mode = S_IRUSR;
    if(entry.permissions[0] == '-')
        mode &= ~(S_IRUSR);
    if(entry.permissions[1] != '-')
        mode |= S_IWUSR;
    if(entry.permissions[2] != '-')
        mode |= S_IXUSR;
    if(entry.permissions[3] != '-')
        mode |= S_IRGRP;
    if(entry.permissions[4] != '-')
        mode |= S_IWGRP;
    if(entry.permissions[5] != '-')
        mode |= S_IXGRP;
    if(entry.permissions[6] != '-')
        mode |= S_IROTH;
    if(entry.permissions[7] != '-')
        mode |= S_IWOTH;
    if(entry.permissions[8] != '-')
        mode |= S_IXOTH;
    if(entry.flags[0] == 's')
        mode |= S_ISUID;
    if(entry.flags[1] == 's')
        mode |= S_ISGID;
    if(entry.flags[2] == 't')
        mode |= 512;
    chmod(path, mode);
    return 0;
}


int partial_parse(FILE *file, sample_entry *entry){
    char user_r[4];
    char group_r[4];
    char other_r[4];
    if (fscanf(file, "# file: %s\n", entry->file_name) != 1){
        return 0;
    }
    if (fscanf(file, "# owner: %s\n", entry->owner) != 1){
        return 0;
    }
    if (fscanf(file, "# group: %s\n", entry->group) != 1){
        return 0;
    }
    if (fscanf(file, "# flags: %s\n", entry->flags) != 1){
        strcpy(entry->flags, "---");
    }
    if (fscanf(file, "user::%s\n", user_r) != 1 ||
        fscanf(file, "group::%s\n", group_r) != 1 ||
        fscanf(file, "other::%s\n", other_r) != 1){
        return 0;
    }
    entry->permissions[0] = user_r[0];
    entry->permissions[1] = user_r[1];
    entry->permissions[2] = user_r[2];
    entry->permissions[3] = group_r[0];
    entry->permissions[4] = group_r[1];
    entry->permissions[5] = group_r[2];
    entry->permissions[6] = other_r[0];
    entry->permissions[7] = other_r[1];
    entry->permissions[8] = other_r[2];
    return 1;

}

