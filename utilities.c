#include <stdlib.h>
#include <stdio.h>

#include "main.h"

void printError(char *error){
    printf(error);
    exit(1);
}

void putBack(struct Program *program) {
    char *t = program->token.name;
    char *pointer = program->currentChar;
    for (; *t; t++) pointer--;
    if (program->token.type == QUOTE)
        pointer -= 2;
    program->currentChar = pointer;
}

int length(char *name){
    int counter = 0;
    char *temp = name;
    for (; *temp; temp++) {
        counter++;
    }
    return counter;
}