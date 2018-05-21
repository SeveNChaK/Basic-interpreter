#include <stdio.h>
#include <ctype.h>
#include <mem.h>
#include <setjmp.h>
#include <stdlib.h>

//Мои штуки
#include "constants.h"

//-------------------------------------------------------------


//Объявление переменных
char *program;
char token[80]; //Строковое представление лексемы
int token_int; //Внутреннее представление лексемы
int token_type; //Тип лексемы

struct command {
    char name[10];
    int token_int;
} tableCommand[] = {
        "PRINT", PRINT,
        "INPUT", INPUT,
        "IF", IF,
        "GOTO", GOTO,
        "GOSUB", GOSUB,
        "RETURN", RETURN};

//Объявление функций
int getToken();

void putBack();

void assignment();

int isDelim(char);

void sError(int);

int getIntCommand(char *);

void level2(int *), level3(int *), level4(int *), level5(int *), level6(int *); //TODO
void primitive(int *);

void unary(char, int *);

void arith(char, int *, int *);

void getExp(int *);

void basicPrint();
basicIntput(); //TODO
basicIf(); //TODO
basicGoto(); //TODO
basicGosub(); //TODO
basicReturn(); //TODO


//Начало работы анализатора
void start(char *p) {
    program = p;
    do {
        token_type = (char) getToken();

        //Проверка на присваивание
        if (token_type == VARIABLE) {
            putBack();
            assignment();
        }

        //Проверка на команду
        if (token_type == COMMAND) {
            switch (token_int) {
                case PRINT:
                    basicPrint();
                    break;
                case END:
                    exit(0);
                default:
                    break;
            }
        }

    } while (token_int != FINISHED);
}

int getToken() {
    char *temp; //Указатель на лексему

    token_type = 0;
    token_int = 0;
    temp = token;

    //Проверка закончился ли файл интерпретируемой программы
    if (*program == '\0') {
        *token = '\0';
        token_int = FINISHED;
        return (token_type = DELIMITER);
    }

    //Проверка на конец строки программы
    if (*program == '\n') {
        program++;
        token_int = EOL;
        *token = '\n';
        temp++;
        *temp = 0;
        return (token_type = DELIMITER);
    }

    while (isspace(*program)) program++; //Пропускаем пробелы; Перенести в начало, реализовав свою функцию, чтоб \n не удалялся TODO

    //Проверка на разделитель
    if (strchr("!+-*/%=;(),><", *program)) {
        *temp = *program;
        program++;
        temp++;
        *temp = 0;
        return (token_type = DELIMITER);
    }

    //Проверяем на кавычки
    if (*program == '"') {
        program++;
        while (*program != '"' && *program != '\n') *temp++ = *program++;
        if (*program == '\n') sError(1);
        program++;
        *temp = 0;
        return (token_type = QUOTE);
    }

    //Проверка на число
    if (isdigit(*program)) {
        while (!isDelim(*program)) {
            *temp++ = *program++;
        }
        *temp = 0;
        return (token_type = NUMBER);
    }

    //Переменная или команда?
    if (isalpha(*program)) {
        while (!isDelim(*program))
            *temp++ = *program++;
        *temp = 0;
        token_int = getIntCommand(token); //Получение внутреннего представления команды
        if (!token_int)
            token_type = VARIABLE;
        else
            token_type = COMMAND;
        return token_type;
    }
    return token_type = FINISHED; //Заглушка
}

int isDelim(char c) {
    if (strchr(" !;,+-<>\'/*%=()\"", c) || c == 9 || c == '\r' || c == 0 || c == '\n')
        return 1;
    return 0;
}

//Переписать ошибки с русского TODO
void sError(int error) {
    static char *e[] = {
            "Syntax error",
            "Unpaired parentheses",
            "This is not an expression",
            "The symbol of equality",
            "Not variable",
            "Label table is full",
            "Duplicate labels",
            "Undefined label",
            "Operator required THEN",
            "The level of nesting GOSUB is too large",
            "RETURN does not match GOSUB"
    };
    printf("%s\n", e[error]);
    //longjmp(e_buf, 1); //Возврат в точку сохранения
}

int getIntCommand(char *t) {

    //Поиск лексемы в таблице операторов
    for (int i = 0; *tableCommand[i].name; i++) {
        if (!strcmp(tableCommand[i].name, t)) return tableCommand[i].token_int;
    }

    return 0; //Незнакомый оператор
}

void putBack() {
    char *t;
    t = token;
    for (; *t; t++) program--;
}

//Сложение или вычитание двух термов
void level2(int *result) {
    char op;
    int hold;

    level3(result);
    while ((op = *token) == '+' || op == '-') {
        getToken();
        level3(&hold);
        arith(op, result, &hold);
    }
}

//Вычисление произведения или частного двух фвкторов
void level3(int *result) {
    char op;
    int hold;

    level4(result);

    while ((op = *token) == '+' || op == '/' || op == '%' || op == '*') {
        getToken();
        level4(&hold);
        arith(op, result, &hold);
    }
}

//Обработка степени числа (целочисленной) (По заданию не надо)
void level4(int *result) {
    level5(result);
}

//Унарный + или -
void level5(int *result) {
    char operation;
    operation = 0;
    if ((token_type == DELIMITER) && *token == '+' || *token == '-') {
        operation = *token;
        getToken();
    }
    level6(result);
    if (operation)
        unary(operation, result);
}

//Обработка выражения в круглых скобках
void level6(int *result) {
    if ((*token == '(') && (token_type == DELIMITER)) {
        getToken();
        level2(result);
        if (*token != ')')
            sError(1);
        getToken();
    } else
        primitive(result);
}

//Определение значения переменной по ее имени
void primitive(int *result) {
    switch (token_type) {
        case VARIABLE:
            //*result = findVar(token); //Разобраться с переменными TODO
            getToken();
            return;
        case NUMBER:
            *result = atoi(token);
            getToken();
            return;
        default:
            sError(0);
    }
}

//Изменение знака
void unary(char o, int *r) {
    if (o == '-') *r = -(*r);
}

//Выполнение специфицированной арифметики
void arith(char o, int *r, int *h) {
    int t, ex;

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

void getExp(int *result) {
    getToken();
    if (!*token) {
        sError(2);
        return;
    }
    level2(result);
    putBack(); //Возвращает последнюю считанную лексему во входной поток
}

void assignment() {

    int value;
    getToken(); //Получаем имя переменной

    if (!isalpha(*token)) {
        sError(4);
        return;
    }

    //Поиск переменной
    //TODO

    getToken(); //Считываем символ равенства
    if (*token != '=') {
        sError(3);
        return;
    }

    getExp(&value); //Считать присваемое значение

    //Присвоить значение
    //TODO

}

void basicPrint() {
    int answer;
    int len = 0;
    char last_delim = 0;

    do {
        getToken(); //Получаем следующий элемент
        if (token_int == EOL || token_int == FINISHED) break;

        if (token_type == QUOTE) {
            printf(token);
            len += strlen(token);
            getToken();
        } else { //Значит выражение
            putBack();
            getExp(&answer);
            getToken();
            len += printf("%d", answer);
        }
        last_delim = *token;

        if (*token != ',' && token_int != EOL && token_int != FINISHED)
            sError(0);
    } while (*token == ',');

    //Поотлаживаем и удалим, не нужно
    if (token_int == EOL || token_int == FINISHED) {
        if (last_delim != ';' && last_delim != ',') printf("\n");
    } else sError(0); //Отсутствует ',' или ';'
}