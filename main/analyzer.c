#include <stdio.h>
#include <ctype.h>
#include <mem.h>
#include <setjmp.h>
#include <stdlib.h>

//Мои штуки
#include "constants.h"

//-------------------------------------------------------------

//Объявление переменных
#define LAB_LEN 2
#define NUM_LAB 100
#define SUB_NEST 25

char *program;
char token[80]; //Строковое представление лексемы
int token_int; //Внутреннее представление лексемы
int token_type; //Тип лексемы
jmp_buf e_buf;

char *gStack[SUB_NEST]; //Стек подпрограмм GOSUB
int gIndex; //Индекс верхней части стека

struct command {
    char name[10];
    int token_int;
} tableCommand[] = {
        "PRINT", PRINT,
        "INPUT", INPUT,
        "IF", IF,
        "THEN", THEN,
        "ELSE", ELSE,
        "FI", FI,
        "GOTO", GOTO,
        "GOSUB", GOSUB,
        "RETURN", RETURN,
        "END", END};

struct label {
    char name[LAB_LEN]; //Имя метки
    char *p; //Указатель на место размещения в программе
};
struct label labels[NUM_LAB];

int countV = 0;
struct variable {
    char name[80];
    int value;
} *p_variable;

//Объявление функций
int getToken(); //Достает очередную лексему

void putBack(); //Возвращает лексему во взожной поток
void findEol(); //Переходит на следующую строку
int isDelim(char); //Проверяет является ли символ разделителем
void sError(int); //Сделать ошибки более информативными
int getIntCommand(char *); //Возвращает внутреннее представление команды

void assignment(); //Присваивает значение переменной
void level2(int *), level3(int *), level4(int *), level5(int *); //Уровни анализа арифметической операции
void value(int *); //Определение значения переменной
void unary(char, int *); //Изменение знака
void arith(char, int *, int *); //Примитивные операции
void getExp(int *); //Начало анализа арифметического выражения
struct variable *findV(char[]); //Поиск переменной по имени

int getNextLabel(char *); //Возвращает следующую метку
void scanLabels(); //Находит все метки
void labelInit(); //Заполняет массив с метками нулями
char *findLabel(char *); //Возвращает метку

void basicPrint();
void basicInput();
void basicIf();
void skipElse();
void basicGoto();
void basicGosub();
void basicReturn();
void gPush(char *);
char *gPop();

void start(char *p) {
    program = p;

    if (setjmp(e_buf)) exit(1); //Инициализация буфера нелокальных переходов

    scanLabels();

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
                case INPUT:
                    basicInput();
                    break;
                case IF:
                    basicIf();
                    break;
                case ELSE:
                    skipElse();
                    break;
                case GOTO:
                    basicGoto();
                    break;
                case GOSUB:
                    basicGosub();
                    break;
                case RETURN:
                    basicReturn();
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

    while (isspace(*program))
        program++; //Пропускаем пробелы; Перенести в начало, реализовав свою функцию, чтоб \n не удалялся TODO

    //Проверка на разделитель
    if (strchr("+-*/%=:,()><", *program)) {
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
        if (!token_int) {
            token_type = VARIABLE;
        } else
            token_type = COMMAND;
        return token_type;
    }
    sError(0);
}

int isDelim(char c) {
    if (strchr(" !;,+-<>\'/*%=()\"", c) || c == 9 || c == '\r' || c == 0 || c == '\n')
        return 1;
    return 0;
}

//Чем я думал? Можно же просто аргументом строку подавать...
//Обязательно когда-нибудь перепишу
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
    longjmp(e_buf, 1); //Возврат в точку сохранения
}

int getIntCommand(char *t) {

    //Поиск лексемы в таблице операторов
    for (int i = 0; *tableCommand[i].name; i++) {
        if (!strcmp(tableCommand[i].name, t))
            return tableCommand[i].token_int;
    }

    return 0; //Незнакомый оператор
}

void putBack() {
    char *t;
    t = token;
    for (; *t; t++) program--;
}

//Сложение или вычитание
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

//Вычисление произведения или частного
void level3(int *result) {
    char operation;
    int hold;

    level4(result);

    while ((operation = *token) == '/' || operation == '%' || operation == '*') {
        getToken();
        level4(&hold);
        arith(operation, result, &hold);
    }
}

//Унарный + или -
void level4(int *result) {
    char operation;
    operation = 0;
    if ((token_type == DELIMITER) && *token == '+' || *token == '-') {
        operation = *token;
        getToken();
    }
    level5(result);
    if (operation)
        unary(operation, result);
}

//Обработка выражения в круглых скобках
void level5(int *result) {
    if ((*token == '(') && (token_type == DELIMITER)) {
        getToken();
        level2(result);
        if (*token != ')')
            sError(1);
        getToken();
    } else
        value(result);
}

//Определение значения переменной по ее имени
void value(int *result) {
    switch (token_type) {
        case VARIABLE:
            *result = findV(token)->value;
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

void getExp(int *result) {
    getToken();
//    if (!*token) {
//        sError(2);
//        return;
//    }
    level2(result);
    putBack();
}

struct variable *findV(char n[]) {
    int i = 1;
    struct variable *t = p_variable;
    while (i <= countV) {
        if (!strcmp(n, t->name)) {
            return t;
        }
        i++;
        t++;
    }
    return NULL;
}

void setValue(struct variable *var, int v) {
    var->value = v;
}

struct variable *addV(char n[]) {
    struct variable *t = NULL;
    if (countV != 0)
        t = p_variable;

    countV++;
    p_variable = (struct variable *) realloc(p_variable, sizeof(struct variable) * countV);
    if (t != NULL) {
        p_variable = t;
    }
    int i = 1;
    while (i < countV) {
        p_variable++;
        i++;
    }
    struct variable *r = p_variable;
    strcpy(p_variable->name, n);
    i = 1;
    while (i < countV) {
        p_variable--;
        i++;
    }
    return r;
}

//Присваивание значения переменной
void assignment() {

    int value;
    getToken(); //Получаем имя переменной

    struct variable *var;
    if ((var = findV(token)) != NULL) {
        getToken(); //Считываем равно
        getExp(&value);
        setValue(var, value);
    } else {
        var = addV(token);
        getToken(); // Считываем равно
        getExp(&value);
        setValue(var, value);
    }
}

//Переход на следующую строку программы
void findEol() {
    while (*program != '\n' && *program != '\0')
        program++;
    if (*program)
        program++;
}

void basicPrint() {
    int answer;
    char lastDelim = 0;

    do {
        getToken(); //Получаем следующий элемент
        if (token_int == EOL || token_int == FINISHED) break;

        if (token_type == QUOTE) {
            printf(token);
            getToken();
        } else { //Значит выражение
            putBack();
            getExp(&answer);
            getToken();
            printf("%d", answer);
        }
        lastDelim = *token;

        if (*token != ',' && token_int != EOL && token_int != FINISHED)
            sError(0);
    } while (*token == ',');

    if (token_int == EOL || token_int == FINISHED) {
        if (lastDelim != ';' && lastDelim != ',') printf("\n");
        else sError(0);
    } else sError(0); //Отсутствует ',' или ';'
}

void basicIf() {
    int x, y, cond;
    char operation;
    getExp(&x); //Получаем левое выражение
    getToken(); //Получаем оператор
    if (!strchr("=<>", *token)) {
        sError(0);      //Недопустимый оператор
        return;
    }
    operation = *token;
    getExp(&y);  //Получаем правое выражение

    //Определяем результат
    cond = 0;
    switch (operation) {
        case '=':
            if (x == y) cond = 1;
            break;
        case '<':
            if (x < y) cond = 1;
            break;
        case '>':
            if (x > y) cond = 1;
            break;
        default:
            break;
    }
    if (cond) {  //Если значение IF "истина"
        getToken();
        if (token_int != THEN) {
            sError(8);
            return;
        }
    } else {
        getToken(); //Пропускаем THEN
        getToken();
        if (strchr("\n", *token)) {
            do {
                getToken();
                if (token_int == END) {
                    sError(0); //Прописать ошибку (Ожидался ELSE) TODO
                }
            } while (token_int != ELSE);
        } else findEol(); //Если ложь - переходим на следующую строку
    }
}

void skipElse() {
    do {
        getToken();
        if (token_int == END) {
            getToken();
            if (token_int != FI)
                sError(0); //Прописать ошибку (Ожидался END FI) TODO
        }
    } while (token_int != FI);
}

void basicGoto() {
    char *location;

    getToken(); //Получаем метку перехода

    //Поиск местоположения метки
    location = findLabel(token);
    if (location == '\0')
        sError(7); //Метка не обнаружена
    else program = location; //Старт программы с указанной точки
}

//Инициализация массива хранения меток
void labelInit() {
    for (int i = 0; i < NUM_LAB; i++)
        labels[i].name[0] = '\0';
}

//Поиск всех меток
void scanLabels() {
    int location;
    char *temp;

    labelInit();  //Инициализация массива меток
    temp = program;   //Указатель на начало программы

    getToken();
    //Если лексема является меткой
    if (token_type == NUMBER) {
        strcpy(labels[0].name, token);
        labels[0].p = program;
    }

    findEol();
    do {
        getToken();
        if (token_type == NUMBER) {
            location = getNextLabel(token);
            if (location == -1 || location == -2) {
                if (location == -1)
                    sError(5);
                else
                    sError(6);
            }
            strcpy(labels[location].name, token);
            labels[location].p = program; //Текущий указатель программы
        }
        //Если строка не помечена, переход к следующей
        if (token_int != EOL) findEol();
    } while (token_int != FINISHED);
    program = temp; //Восстанавливаем начальное значение
}

char *findLabel(char *s) {
    for (int i = 0; i < NUM_LAB; i++)
        if (!strcmp(labels[i].name, s))
            return labels[i].p;
    return '\0'; //Ошибка
}

//Возвращает индекс на следующую свободную позицию массива меток
//  -1, если массив переполнен
//  -2, если дублирование меток
int getNextLabel(char *s) {

    for (int i = 0; i < NUM_LAB; i++) {
        if (labels[i].name[0] == 0) return i;
        if (!strcmp(labels[i].name, s)) return -2;
    }
    return -1;
}

void basicGosub() {
    char *location;

    getToken();

    //Поиск метки вызова
    location = findLabel(token);
    if (location == '\0')
        sError(7); //Метка не определена
    else {
        gPush(program); //Запомним место, куда вернемся
        program = location; //Старт программы с указанной точки
    }
}

//Возврат из подпрограммы
void basicReturn() {
    program = gPop();
}

//Помещает данные в стек GOSUB
void gPush(char *s) {
    gIndex++;
    if (gIndex == SUB_NEST) {
        sError(12);
        return;
    }
    gStack[gIndex] = s;
}

//Достает данные из стека GOSUB
char *gPop() {
    if (gIndex == 0) {
        sError(13);
        return '\0';
    }
    return (gStack[gIndex--]);
}

void basicInput() {
    int i;
    struct variable *var;

    getToken(); //Анализ наличия символьной строки
    if (token_type == QUOTE) {
        printf(token); //Если строка есть, проверка запятой
        getToken();
        if (*token != ',') sError(1);
        getToken();
    } else printf("Write data: ");
    if ((var = findV(token)) == NULL)
        var = addV(token);
    scanf("%d", &i);   //Чтение входных данных
    setValue(var, i);  //Сохранение данных
}



