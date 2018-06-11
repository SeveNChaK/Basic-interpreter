#include <stdio.h>
#include <ctype.h>
#include <mem.h>
#include <setjmp.h>
#include <stdlib.h>

//Мои штуки
#include "constants.h"

//-------------------------------------------------------------

//Объявление переменных
#define LENGTH_LABEL 2
#define NUM_LABEL 100
#define GOSUB_NESTING 25

char *program;
struct lexem {
    char name[80]; //Строковое представление лексемы
    int id; //Внутреннее представление лексемы
    int type; //Тип лексемы
} token;


char *gStack[GOSUB_NESTING]; //Стек подпрограмм GOSUB
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
    char name[LENGTH_LABEL]; //Имя метки
    char *p; //Указатель на место размещения в программе
};
struct label labels[NUM_LABEL];

int countV = 0;
struct variable {
    char name[80];
    int value;
} *p_variable;

//Объявление функций
void getToken(); //Достает очередную лексему
int isWhite(char);

void putBack(); //Возвращает лексему во взожной поток
void findEol(); //Переходит на следующую строку
int isDelim(char); //Проверяет является ли символ разделителем
void printError(char *); //Сделать ошибки более информативными
int getIntCommand(char *); //Возвращает внутреннее представление команды

void assignment(); //Присваивает значение переменной
void level2(int *), level3(int *), level4(int *), level5(int *); //Уровни анализа арифметической операции
void value(int *); //Определение значения переменной
void unary(char, int *); //Изменение знака
void arith(char, int *, int *); //Примитивные операции
void getExp(int *); //Начало анализа арифметического выражения
struct variable *findV(char *); //Поиск переменной по имени
struct variable *addV(char *); //Добавление новой переменной

int getNextLabel(char *); //Возвращает следующую метку
void scanLabels(); //Находит все метки
void labelInit(); //Заполняет массив с метками нулями
char *findLabel(char *); //Возвращает метку

void basicPrint(), basicInput(), basicIf(),
        basicGoto(), basicGosub(),
        skipElse(), basicReturn();

void gPush(char *);

char *gPop();

void start(char *p) {
    program = p;

    scanLabels();

    do {
        getToken();

        //Проверка на присваивание
        if (token.type == VARIABLE) {
            putBack();
            assignment();
        }

        //Проверка на команду
        if (token.type == COMMAND) {
            switch (token.id) {
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
    } while (token.id != FINISHED);
}

int isWhite(char c) {
    if (c == ' ' || c == '\t') return 1;
    else return 0;
}

void getToken() {
    char *temp = token.name; //Указатель на лексему
    token.id = 0;
    token.type = 0;

    //Пропускаем пробелы
    while (isWhite(*program))
        program++;

    //Проверка закончился ли файл интерпретируемой программы
    if (*program == '\0') {
        temp = '\0';
        token.id = FINISHED;
        token.type = DELIMITER;
        return;
    }

    //Проверка на конец строки программы
    if (*program == '\n') {
        *temp++ = *program++;
        *temp = '\0';
        token.id = EOL;
        token.type = DELIMITER;
        return;
    }

    //Проверка на разделитель
    if (strchr("+-*/%=:,()><", *program)) {
        *temp++ = *program++;
        *temp = '\0';
        token.type = DELIMITER;
        return;
    }

    //Проверяем на кавычки
    if (*program == '"') {
        program++;
        while (*program != '"' && *program != '\n')
            *temp++ = *program++;
        if (*program == '\n')
            printError("Unpaired parentheses");
        program++;
        *temp = '\0';
        token.type = QUOTE;
        return;
    }

    //Проверка на число
    if (isdigit(*program)) {
        while (!isDelim(*program))
            *temp++ = *program++;
        *temp = '\0';
        token.type = NUMBER;
        return;
    }

    //Переменная или команда?
    if (isalpha(*program)) {
        while (!isDelim(*program))
            *temp++ = *program++;
        *temp = 0;
        token.id = getIntCommand(token.name); //Получение внутреннего представления команды
        if (!token.id) {
            token.type = VARIABLE;
        } else
            token.type = COMMAND;
        return;
    }
    printError("Syntax error");
}

int isDelim(char c) {
    if (strchr(" !;,+-<>\'/*%=()\"", c) || c == '\r' || c == '\n')
        return 1;
    return 0;
}

//Чем я думал? Можно же просто аргументом строку подавать...
//Обязательно когда-нибудь перепишу
void printError(char *error) {
    printf(error);
    exit(1);
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
    t = token.name;
    for (; *t; t++) program--;
}

//Сложение или вычитание
void level2(int *result) {
    char op;
    int hold;

    level3(result);
    while ((op = *token.name) == '+' || op == '-') {
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

    while ((operation = *token.name) == '/' || operation == '%' || operation == '*') {
        getToken();
        level4(&hold);
        arith(operation, result, &hold);
    }
}

//Унарный + или -
void level4(int *result) {
    char operation;
    operation = 0;
    if ((token.type == DELIMITER) && *token.name == '+' || *token.name == '-') {
        operation = *token.name;
        getToken();
    }
    level5(result);
    if (operation)
        unary(operation, result);
}

//Обработка выражения в круглых скобках
void level5(int *result) {
    if ((*token.name == '(') && (token.type == DELIMITER)) {
        getToken();
        level2(result);
        if (*token.name != ')')
            printError("Unpaired parentheses");
        getToken();
    } else
        value(result);
}

//Определение значения переменной по ее имени
void value(int *result) {
    struct variable *temp = findV(token.name);
    switch (token.type) {
        case VARIABLE:
            if (temp == NULL || temp->value == NULL)
                printError("Variable not initialized");
            else
                *result = temp->value;
            getToken();
            return;
        case NUMBER:
            *result = atoi(token.name);
            getToken();
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

void getExp(int *result) {
    getToken();
    level2(result);
    putBack();
}

struct variable *findV(char *name) {
    int i = 1;
    struct variable *temp = p_variable;
    while (i <= countV) {
        if (!strcmp(name, temp->name)) {
            return temp;
        }
        i++;
        temp++;
    }
    return NULL;
}

struct variable *addV(char *name) {
    countV++;
    p_variable = (struct variable *) realloc(p_variable, sizeof(struct variable) * countV);
    struct variable *temp = p_variable;

    int i = 1;
    while (i < countV) {
        temp++;
        i++;
    }
    strcpy(temp->name, name);
    temp->value = NULL;

    return temp;
}

//Присваивание значения переменной
void assignment() {
    int value;
    getToken(); //Получаем имя переменной
    struct variable *var;
    if ((var = findV(token.name)) != NULL) {
        getToken(); //Считываем равно
        getExp(&value);
        var->value = value;
    } else {
        var = addV(token.name);
        getToken(); // Считываем равно
        getExp(&value);
        var->value = value;
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
        if (token.id == EOL || token.id == FINISHED) break;

        if (token.type == QUOTE) {
            printf(token.name);
            getToken();
        } else { //Значит выражение
            putBack();
            getExp(&answer);
            getToken();
            printf("%d", answer);
        }
        lastDelim = *token.name;
        if (*token.name != ',' && token.id != EOL && token.id != FINISHED)
            printError("Syntax error");
    } while (*token.name == ',');

    if (token.id == EOL || token.id == FINISHED) {
        if (lastDelim != ';' && lastDelim != ',') printf("\n");
        else printError("Syntax error");
    } else printError("Syntax error"); //Отсутствует ',' или ';'
}

void basicIf() {
    int x, y, cond;
    char operation;
    getExp(&x); //Получаем левое выражение
    getToken(); //Получаем оператор
    if (!strchr("=<>", *token.name)) {
        printError("Syntax error");      //Недопустимый оператор
        return;
    }
    operation = *token.name;
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
        if (token.id != THEN) {
            printError("Operator required THEN");
            return;
        }
    } else {
        getToken(); //Пропускаем THEN
        getToken();
        if (strchr("\n", *token.name)) {
            do {
                getToken();
                if (token.id == END) {
                    printError("Syntax error");
                }
            } while (token.id != ELSE);
        } else findEol(); //Если ложь - переходим на следующую строку
    }
}

void skipElse() {
    do {
        getToken();
        if (token.id == END) {
            getToken();
            if (token.id != FI)
                printError("Syntax error");
        }
    } while (token.id != FI);
}

void basicGoto() {
    char *location;
    getToken(); //Получаем метку перехода
    //Поиск местоположения метки
    location = findLabel(token.name);
    if (location == '\0')
        printError("Undefined label"); //Метка не обнаружена
    else program = location; //Старт программы с указанной точки
}

//Инициализация массива хранения меток
void labelInit() {
    for (int i = 0; i < NUM_LABEL; i++)
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
    if (token.type == NUMBER) {
        strcpy(labels[0].name, token.name);
        labels[0].p = program;
    }

    findEol();
    do {
        getToken();
        if (token.type == NUMBER) {
            location = getNextLabel(token.name);
            if (location == -1 || location == -2) {
                if (location == -1)
                    printError("Label table is full");
                else
                    printError("Duplicate labels");
            }
            strcpy(labels[location].name, token.name);
            labels[location].p = program; //Текущий указатель программы
        }
        //Если строка не помечена, переход к следующей
        if (token.id != EOL) findEol();
    } while (token.id != FINISHED);
    program = temp; //Восстанавливаем начальное значение
}

char *findLabel(char *s) {
    for (int i = 0; i < NUM_LABEL; i++)
        if (!strcmp(labels[i].name, s))
            return labels[i].p;
    return '\0'; //Ошибка
}

//Возвращает индекс на следующую свободную позицию массива меток
//  -1, если массив переполнен
//  -2, если дублирование меток
int getNextLabel(char *s) {

    for (int i = 0; i < NUM_LABEL; i++) {
        if (labels[i].name[0] == 0) return i;
        if (!strcmp(labels[i].name, s)) return -2;
    }
    return -1;
}

void basicGosub() {
    char *location;

    getToken();

    //Поиск метки вызова
    location = findLabel(token.name);
    if (location == '\0')
        printError("Undefined label"); //Метка не определена
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
    if (gIndex == GOSUB_NESTING) {
        printError("Label table is full");
        return;
    }
    gStack[gIndex] = s;
}

//Достает данные из стека GOSUB
char *gPop() {
    if (gIndex == 0) {
        printError("Undefined label");
        return '\0';
    }
    return (gStack[gIndex--]);
}

void basicInput() {
    int i;
    struct variable *var;

    getToken(); //Анализ наличия символьной строки
    if (token.type == QUOTE) {
        printf(token.name); //Если строка есть, проверка запятой
        getToken();
        if (*token.name != ',') printError("Syntax error");
        getToken();
    } else printf("Write data: ");
    if ((var = findV(token.name)) == NULL)
        var = addV(token.name);
    scanf("%d", &i);   //Чтение входных данных
    var->value = i;
}



