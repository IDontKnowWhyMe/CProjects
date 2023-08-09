#include "sample_printer.h"
#include "prem_check.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define UNUSED(x) ((void) x)

int main(int argc, char** argv) {
    settings *setting = malloc(sizeof (settings));
    init_settings(argc, argv, setting);
    if (setting->switcher == export){
        if (export_mode(*setting) != 0 ){
            clear_settings(setting);
            free(setting);
            return EXIT_FAILURE;
        }
    } else
    {
        if(import_mode(*setting) != 0){
            clear_settings(setting);
            free(setting);
            return EXIT_FAILURE;
        }
    }
    clear_settings(setting);
    free(setting);
    return EXIT_SUCCESS;
}
