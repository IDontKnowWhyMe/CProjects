#include "string_number.h"
#include "string.h"
#include "sim.h"

int is_string_float(char *string){
    int flag = 0;
    int i = 0;
    while(string[i] != '\0'){
        if(string[i] >= '0' && string[i] <= '9'){
            i++;
            continue;
        }
        else if (string[i] == '.' && i != 0 && flag == 0)
            flag = 1;
        else
            return -1;
        i++;
    }
    if (flag == 0)
        return 1;
    return 0;
}



