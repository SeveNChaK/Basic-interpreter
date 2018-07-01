#include <stdio.h>
#include <stdlib.h>

#include "main.h"

#define PROG_SIZE 100

int load_program(struct Program *program, char *file_name); //Считывает программу

int main (int argc, char *argv[]) {
    if (argc != 3) {
        printError("Wrong format.\nUse: <program file>.txt");
    }

    char *file_name_program = argv[1];

    struct Program program;
    program.token = *((struct Lexem*) malloc(sizeof(struct Lexem)));
    program.infoVariables.vars = (struct Variable*) malloc(sizeof(struct Variable));
    program.infoVariables.countVars = 0;
    program.gosub.index = 0;

    //Выделение памяти для программы
    if (!(program.currentChar = (char *) malloc(PROG_SIZE))) {
        printError("Error allocating memory");
    }

    if(!load_program(&program, file_name_program))
        printError("Doesn't load program!");

    execute(&program);

    //Освобождаем память
    free(program.currentChar);
    free(program.token.name);
    free(program.infoVariables.vars);

    return 0;
}

int load_program(struct Program *program, char *file_name) {
    FILE *file;
    if (!(file = fopen(file_name, "r")))
        return 0;
    char *point = program->currentChar;
    int i = 0, k = 1;

    do {
        *point = (char) getc(file);
        point++;
        i++;
        if (i == k * PROG_SIZE) {
            k++;
            program->currentChar = (char *) realloc(program->currentChar, (size_t) (k * PROG_SIZE));
            point = program->currentChar;
            point += i;
        }
    } while (!feof(file));
    *(point - 1) = 0;
    fclose(file);
    return 1;
}
