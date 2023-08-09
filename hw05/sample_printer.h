#ifndef PRINTER_SAMPLE_H
#define PRINTER_SAMPLE_H

#include <stdio.h>

/**
 * Writes a sample output checkperms list into the specified file.
 **/
typedef struct sample_entry
{
    char *file_name;
    char *owner;
    char *group;
    char permissions[10];
    char flags[4];
} sample_entry;

void print_sample_output(FILE *file);

void print_sample_entry(FILE *file, const sample_entry *entry);

#endif
