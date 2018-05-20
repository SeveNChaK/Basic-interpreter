#ifndef INTERPRETERBASIC_ANALYZER_H
#define INTERPRETERBASIC_ANALYZER_H

#include "global_var.h"

char token[80]; //Строковое представление лексемы
int token_int; //Внутреннее представление лексемы
int token_type; //Тип лексемы

char *prog; //Указатель на анализируемое выражение

int getToken();

#endif //INTERPRETERBASIC_ANALYZER_H


