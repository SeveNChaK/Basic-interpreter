#include <mem.h>
#include <stdlib.h>
#include "main.h"

void level2(struct Program *program, int *result),
        level3(struct Program *program, int *result),
        level4(struct Program *program, int *result),
        level5(struct Program *program, int *result),
        value(struct Program *program, int *result),
        unary(char o, int *r),
        arith(char o, int *r, int *h);

void assignment(struct Program *program){
    int value;
    getToken(program); //Получаем имя переменной
    char *var_name = mallocAndCopy(program->token.name, length(program->token.name));
    getToken(program); //Считываем равно
    if (strcmp(program->token.name, "="))
        printError("Wait =");
    calcExpression(program, &value);
    struct Variable *var = findVariable(program, var_name);
    if (var == NULL){
        var = addVariable(program, var_name);
        var->value = value;
    } else
        var->value = value;
    free(var_name);
}

void calcExpression(struct Program *program, int *result){
    getToken(program);
    level2(program, result);
    putBack(program);
}

//Сложение или вычитание
void level2(struct Program *program, int *result){
    char operation;
    int hold;

    level3(program, result);
    while ((operation = *program->token.name) == '+' || operation == '-') {
        getToken(program);
        level3(program, &hold);
        arith(operation, result, &hold);
    }
}

//Вычисление произведения или частного
void level3(struct Program *program, int *result) {
    char operation;
    int hold;

    level4(program, result);

    while ((operation = *program->token.name) == '/' || operation == '%' || operation == '*') {
        getToken(program);
        level4(program, &hold);
        arith(operation, result, &hold);
    }
}

//Унарный + или -
void level4(struct Program *program, int *result) {
    char operation;
    operation = 0;
    if ((program->token.type == DELIMITER)
        && *program->token.name == '+'
        || *program->token.name == '-') {
        operation = *program->token.name;
        getToken(program);
    }
    level5(program, result);
    if (operation)
        unary(operation, result);
}

//Обработка выражения в круглых скобках
void level5(struct Program *program, int *result) {
    if ((*program->token.name == '(') && (program->token.type == DELIMITER)) {
        getToken(program);
        level2(program, result);
        if (*program->token.name != ')')
            printError("Unpaired parentheses");
        getToken(program);
    } else
        value(program, result);
}

//Определение значения переменной по ее имени
void value(struct Program *program, int *result) {
    struct Variable *temp = findVariable(program, program->token.name);
    switch (program->token.type) {
        case VARIABLE:
            if (temp == NULL)
                printError("Var not init"); //TODO
            *result = temp->value;
            getToken(program);
            return;
        case NUMBER:
            *result = atoi(program->token.name);
            getToken(program);
            return;
        default:
            printError("Syntax error");
    }
}

//Изменение знака
void unary(char o, int *r) {
    if (o == '-') *r = -(*r);
}

//Выполнение специфицированной арифметики
void arith(char o, int *r, int *h) {
    int t;
    switch (o) {
        case '-':
            *r = *r - *h;
            break;
        case '+':
            *r = *r + *h;
            break;
        case '*':
            *r = *r * *h;
            break;
        case '/':
            *r = (*r) / (*h);
            break;
        case '%':
            t = (*r) / (*h);
            *r = *r - (t * (*h));
            break;
        default:
            break;
    }
}