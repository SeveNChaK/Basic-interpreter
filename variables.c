#include <mem.h>
#include <stdlib.h>

#include "main.h"

struct Variable *findVariable(struct Program *program, char *name) {
    int i = 1;
    struct Variable *temp = program->infoVariables.vars;
    while (i <= program->infoVariables.countVars) {
        if (!strcmp(name, temp->name)) {
            return temp;
        }
        i++;
        temp++;
    }
    return NULL;
}

struct Variable *addVariable(struct Program *program, char *name) {
    program->infoVariables.countVars++;
    program->infoVariables.vars = (struct Variable *) realloc(program->infoVariables.vars,
                                                                sizeof(struct Variable) * program->infoVariables.countVars);
    struct Variable *temp = program->infoVariables.vars;
    int i = 1;
    while (i < program->infoVariables.countVars) {
        temp++;
        i++;
    }
    temp->name = mallocAndCopy(name, length(name));

    return temp;
}