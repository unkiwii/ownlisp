#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 2048

static char input[BUFFER_SIZE];

/* readline for windows */
char* readline(char* prompt)
{
    fputs(prompt, stdout);
    fgets(input, BUFFER_SIZE, stdin);
    char* cpy = malloc(strlen(input) + 1);
    strcpy(cpy, input);
    cpy[strlen(cpy) - 1] = '\0';
    return cpy;
}

/* Fake add_history function */
void add_history(char* ignored) { }
