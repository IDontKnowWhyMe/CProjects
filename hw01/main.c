#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void print_error_message(char *message)
{
    fprintf(stderr, "%s\n", message);
}

void print_bin(uint64_t n)
{
    if (n > 1)
        print_bin(n /2);
    printf("%lu", n % 2);
}


void execute_command(int command, uint64_t *acum, uint64_t val, char *system, uint64_t *memory, bool *zero_error)
{
    switch (command) {
        case 'P':
            *acum = val;
            printf("# %lu\n", *acum);
            break;
        case 'N':
            *acum = 0;
            printf("# %lu\n", *acum);
            break;
        case '+':
            *acum += val;
            printf("# %lu\n", *acum);
            break;
        case '-':
            *acum -= val;
            printf("# %lu\n", *acum);
            break;
        case '*':
            *acum *= val;
            printf("# %lu\n", *acum);
            break;
        case '/':
            if (val == 0)
            {
                *zero_error = true;
                break;
            }
            *acum /= val;
            printf("# %lu\n", *acum);
            break;
        case '%':
            if (val == 0)
            {
                *zero_error = true;
                break;
            }
            *acum %= val;
            printf("# %lu\n", *acum);
            break;
        case '=':
            printf("# %lu\n", *acum);
            break;
        case 'X':
            *system = 'x';
            printf("# %lX\n", *acum);
            break;
        case 'O':
            *system = 'o';
            printf("# %lo\n", *acum);
            break;
        case 'T':
            *system = 't';
            printf("# ");
            print_bin(*acum);
            printf("\n");
            break;
        case 'M':
            *memory = *acum;
            break;
        case 'R':
            *memory = 0;
            break;
        case '<':
            *acum <<= val;
            printf("# %lu\n", *acum);
            break;
        case '>':
            *acum >>= val;
            printf("# %lu\n", *acum);
            break;
    }
}

bool is_param_operator(int looking_for)
{
    char operators[] = {'P', '+', '-', '*', '/', '%', '<', '>'};
    for(int i = 0; i < 8; i++)
    {
        if(looking_for == operators[i])
            return true;
    }
    return false;
}

bool is_nular_operator(int looking_for)
{
    char operators[] = {'=', 'X', 'T', 'O', 'M', 'R', 'N'};
    for(int i = 0; i < 7; i++)
    {
        if(looking_for == operators[i])
            return true;
    }
    return false;
}

bool system_change(int last, int actual)
{
    if(actual == 'T' || actual == 'O' || actual == 'X')
    {
        if(is_param_operator(last))
            return true;
    }
    return false;
}


bool calculate(void)
{
    uint64_t acumulator = 0;
    uint64_t memory = 0;
    uint64_t buffer = 0;
    char system = 'd';
    bool zero_error = false;
    bool system_change_enabled = true;
    bool note_allowed = false;
    bool wants_param = false;
    bool param_received = false;
    int actual = 0;
    int last_command = 0;
    while ((actual = getchar()) != EOF)
    {
        if(actual == 10)
            note_allowed = false;
        if(actual == 32 || actual == 10 || note_allowed)
            continue;
        if(is_param_operator(actual) || is_nular_operator(actual))
        {
            if(last_command != 0 && (!system_change(last_command, actual) || !system_change_enabled))
            {
                if(param_received)
                    execute_command(last_command, &acumulator, buffer, &system, &memory, &zero_error);
                else
                {
                    print_error_message("SYNTAX ERROR");
                    return false;
                }
            }
            param_received = false;
            buffer = 0;
            if(is_param_operator(actual))
            {
                system_change_enabled = true;
                wants_param = true;
                last_command = actual;
                system = 'd';
            }
            else
            {
                execute_command(actual, &acumulator, buffer, &system, &memory, &zero_error);
                if(!system_change(last_command, actual) || !system_change_enabled)
                {
                    system_change_enabled = true;
                    wants_param = false;
                    system = 'd';
                    last_command = 0;
                } else
                    system_change_enabled = false;
            }
        }
        else if(actual == 'm' && is_param_operator(last_command))
        {
            execute_command(last_command, &acumulator, memory, &system, &memory, &zero_error);
            last_command = 0;
        }
        else if(actual >= 48 && actual <= 57 && wants_param && system == 'd')
        {
            param_received = true;
            buffer *= 10;
            buffer += actual - 48;
        }
        else if(actual >= 48 && actual <= 55 && wants_param && system == 'o')
        {
            param_received = true;
            buffer *= 8;
            buffer += actual - 48;
        }
        else if((actual == 48 || actual == 49) && wants_param && system == 't')
        {
            param_received = true;
            buffer *= 2;
            buffer += actual - 48;
        }
        else if(((actual >= 48 && actual <= 57) || (actual >= 65 && actual <= 70) ||
                (actual >= 97 && actual <= 102))
                && wants_param && system == 'x')
        {
            param_received = true;
            buffer *= 16;
            if (actual <= 57)
                buffer += actual - 48;
            else if (actual <= 70)
                buffer += actual - 55;
            else if (actual <= 102)
                buffer += actual - 87;
        }
        else if(actual == ';')
        {
            system_change_enabled = true;
            note_allowed = true;
            if(last_command != 0)
            {
                if(param_received)
                    execute_command(last_command, &acumulator, buffer, &system, &memory, &zero_error);
                else
                {
                    print_error_message("SYNTAX ERROR");
                    return false;
                }
            }
            system = 'd';
            last_command = 0;
            buffer = 0;
        } else
        {
            print_error_message("SYNTAX ERROR");
            return false;
        }
        if(zero_error)
        {
            print_error_message("Division by zero");
            return false;
        }
    }
    if(last_command != 0)
    {
        if(param_received)
            execute_command(last_command, &acumulator, buffer, &system, &memory, &zero_error);
        else
        {
            print_error_message("SYNTAX ERROR");
            return false;
        }
        if(zero_error)
        {
            print_error_message("Division by zero");
            return false;
        }
    }
    return true;
}

int main(void)
{
    if (!calculate()) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
