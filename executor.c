#include <ctype.h>
#include <mem.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"

int isWhite(char c), isDelim(char c);

int getIdCommand(char *command);

void basicPrint(struct Program *program),
        basicInput(struct Program *program),
        basicIf(struct Program *program),
        basicGoto(struct Program *program),
        basicGosub(struct Program *program),
        basicReturn(struct Program *program);
char *findLabel(struct Program *program, char *s);
int getNextLabel(struct Program *program, char *s);
void scanLabels(struct Program *program);
void gPush(struct Program *program, char *s);
char *gPop(struct Program *program);

void execute(struct Program *program) {
    scanLabels(program);
    do {
        getToken(program);

        //Проверка на присваивание
        if (program->token.type == VARIABLE) {
            putBack(program);
            assignment(program);
        }

        //Проверка на команду
        if (program->token.type == COMMAND) {
            switch (program->token.id) {
                case PRINT:
                    basicPrint(program);
                    break;
                case INPUT:
                    basicInput(program);
                    break;
                case IF:
                    basicIf(program);
                    break;
                case GOTO:
                    basicGoto(program);
                    break;
                case GOSUB:
                    basicGosub(program);
                    break;
                case RETURN:
                    basicReturn(program);
                    break;
                case END:
                    exit(0);
                    break;
                default:
                    break;
            }
        }
    } while (program->token.id != FINISHED);
}

void getToken(struct Program *program) {
    free(program->token.name);
    program->token.id = 0;
    program->token.type = 0;

    while (isWhite(*program->currentChar))
        program->currentChar++;

    //Проверка конца программы
    if (*program->currentChar == 0) {
        program->token.id = FINISHED;
        program->token.type = DELIMITER;
        return;
    }
    char *tempStart = program->currentChar;

    //Проверка конца строки
    if (*program->currentChar == '\n') {
        program->currentChar++;
        program->token.name = mallocAndCopy(tempStart, 1);
        program->token.id = EOL;
        program->token.type = DELIMITER;
        return;
    }

    //Првоерка разделителя
    if (strchr(",=+-*/%()<>", *program->currentChar)) {
        program->token.name = mallocAndCopy(tempStart, 1);
        program->currentChar++;
        program->token.type = DELIMITER;
        return;
    }

    //Проверяем на кавычки
    if (*program->currentChar == '"') {
        program->currentChar++;
        tempStart++;
        int counter = 0;
        while (*program->currentChar != '"' && *program->currentChar != '\n') {
            program->currentChar++;
            counter++;
        }
        if (*program->currentChar == '\n')
            printError("Unpaired parentheses");
        program->currentChar++;
        program->token.name = mallocAndCopy(tempStart, counter);
        program->token.type = QUOTE;
        return;
    }

    //Проверка на число
    if (isdigit(*program->currentChar)) {
        int counter = 0;
        while (!isDelim(*program->currentChar)) {
            program->currentChar++;
            counter++;
        }
        program->token.name = mallocAndCopy(tempStart, counter);
        program->token.type = NUMBER;
        return;
    }

    //Проверка на букву
    if (isalpha(*program->currentChar)) {
        int counter = 0;
        while (!isDelim(*program->currentChar)) {
            program->currentChar++;
            counter++;
        }
        program->token.name = mallocAndCopy(tempStart, counter);
        program->token.id = getIdCommand(program->token.name);
        if (!program->token.id)
            program->token.type = VARIABLE;
        else
            program->token.type = COMMAND;
        return;
    }
    printError("Syntax error! Unknown token.");
}

char *mallocAndCopy(char *source, int steps) {
    char *resultPointer = (char *) malloc((size_t) sizeof(char) * (steps + 1));
    char *tempPointer = resultPointer;
    for (int i = 0; i < steps; i++) {
        *tempPointer++ = *source++;
    }
    *tempPointer = '\0';
    return resultPointer;
}
int isWhite(char c) {
    if (c == ' ' || c == '\t') return 1;
    else return 0;
}
int isDelim(char c) {
    if (strchr(" ,+-<>/*%=()\"", c) || c == '\r' || c == '\n')
        return 1;
    return 0;
}
int getIdCommand(char *command) {
    struct command {
        char name[10]; // не больше 10 симовлов
        int token_int;
    } tableCommand[] = {
            "PRINT", PRINT,
            "INPUT", INPUT,
            "IF", IF,
            "THEN", THEN,
            "ELSE", ELSE,
            "ENDFI", ENDFI,
            "GOTO", GOTO,
            "GOSUB", GOSUB,
            "RETURN", RETURN,
            "END", END};
    //Поиск лексемы в таблице операторов
    for (int i = 0; *tableCommand[i].name; i++) {
        if (!strcmp(tableCommand[i].name, command))
            return tableCommand[i].token_int;
    }
    return 0; //Незнакомый оператор
}

void basicPrint(struct Program *program) {
    int answer;
    char lastDelim = 0;

    do {
        getToken(program); //Получаем следующий элемент
        if (program->token.id == EOL || program->token.id == FINISHED) break;

        if (program->token.type == QUOTE) {
            printf(program->token.name);
            getToken(program);
        } else { //Значит выражение
            putBack(program);
            calcExpression(program, &answer);
            getToken(program);
            printf("%d", answer);
        }
        lastDelim = *program->token.name;
        if (*program->token.name != ',' && program->token.id != EOL && program->token.id != FINISHED)
            printError("Syntax error");
    } while (*program->token.name == ',');
    if (lastDelim != '\n')
        printError("Syntax error");
    printf("\n");
}
void basicInput(struct Program *program) {
    int i;
    struct Variable *var;

    getToken(program); //Анализ наличия символьной строки
    if (program->token.type == QUOTE) {
        printf(program->token.name); //Если строка есть, проверка запятой
        getToken(program);
        if (*program->token.name != ',')
            printError("Syntax error");
        getToken(program);
    } else printf("Write data: ");
    if ((var = findVariable(program, program->token.name)) == NULL)
        var = addVariable(program, program->token.name);
    scanf("%d", &i);   //Чтение входных данных
    var->value = i;
}
void basicIf(struct Program *program){
    int left, right, condition;
    char operation;
    calcExpression(program, &left);
    getToken(program); //Получаем оператор
    if (!strchr("=<>", *program->token.name))
        printError("Syntax error!"); //TODO
    operation = *program->token.name;
    calcExpression(program, &right);
    condition = 0;
    switch (operation) {
        case '=':
            if (left == right) condition = 1;
            break;
        case '<':
            if (left < right) condition = 1;
            break;
        case '>':
            if (left > right) condition = 1;
            break;
        default:
            break;
    }
    if (condition){
        getToken(program); //Считываем THEN
        if (program->token.id != THEN)
            printError("Wait THEN"); //TODO
        getToken(program);
        if (program->token.id != EOL)
            printError("Syntax error!"); //TODO
        do{
            getToken(program);
            //Проверка на присваивание
            if (program->token.type == VARIABLE) {
                putBack(program);
                assignment(program);
            }

            //Проверка на команду
            if (program->token.type == COMMAND) {
                switch (program->token.id) {
                    case PRINT:
                        basicPrint(program);
                        break;
                    case INPUT:
                        basicInput(program);
                        break;
                    case IF:
                        basicIf(program);
                        program->token.id = 0;
                        break;
                    case GOTO:
                        basicGoto(program);
                        return;
                        break;
                    case GOSUB:
                        basicGosub(program);
                        break;
                    case RETURN:
                        basicReturn(program);
                        return;
                        break;
                    default:
                        break;
                }
            }
        } while (program->token.id != ELSE && program->token.id != ENDFI);
        if (program->token.id == ENDFI)
            return;

        //Пропускаем ELSE
        int countIf = 0;
        do{
            getToken(program);
            if (program->token.id == IF)
                countIf++;
            if (program->token.id == ENDFI && countIf != 0)
                countIf--;
        } while (program->token.id != ENDFI || countIf !=0);
    } else{
        //Пропускаем THEN
        int countIf = 0;
        do{
            getToken(program);
            if (program->token.id == IF)
                countIf++;
            if (program->token.id == ENDFI && countIf != 0)
                countIf--;
        } while ((program->token.id != ENDFI || countIf !=0) && (program->token.id !=ELSE || countIf != 0));
        if (program->token.id == ENDFI)
            return;
        do{
            getToken(program);
            //Проверка на присваивание
            if (program->token.type == VARIABLE) {
                putBack(program);
                assignment(program);
            }

            //Проверка на команду
            if (program->token.type == COMMAND) {
                switch (program->token.id) {
                    case PRINT:
                        basicPrint(program);
                        break;
                    case INPUT:
                        basicInput(program);
                        break;
                    case IF:
                        basicIf(program);
                        program->token.id = 0;
                        break;
                    case GOTO:
                        basicGoto(program);
                        return;
                        break;
                    case GOSUB:
                        basicGosub(program);
                        break;
                    case RETURN:
                        basicReturn(program);
                        return;
                        break;
                    default:
                        break;
                }
            }
        } while (program->token.id != ENDFI);
    }
}
void basicGoto(struct Program *program) {
    char *location;
    getToken(program); //Получаем метку перехода
    //Поиск местоположения метки
    location = findLabel(program, program->token.name);
    if (location == '\0')
        printError("Undefined label"); //Метка не обнаружена
    else program->currentChar = location; //Старт программы с указанной точки
}
//Инициализация массива хранения меток
void labelInit(struct Program *program) {
    for (int i = 0; i < NUM_LABELS; i++)
        program->labels[i].name[0] = '\0';
}
//Переход на следующую строку программы
void findEol(struct Program *program) {
    while (*program->currentChar != '\n' && *program->currentChar != '\0')
        program->currentChar++;
    if (*program->currentChar)
        program->currentChar++;
}
char *findLabel(struct Program *program, char *s) {
    for (int i = 0; i < NUM_LABELS; i++)
        if (!strcmp(program->labels[i].name, s))
            return program->labels[i].pointer;
    return '\0'; //Ошибка
}
//Поиск всех меток
void scanLabels(struct Program *program) {
    int location;
    char *temp;

    labelInit(program);  //Инициализация массива меток
    temp = program->currentChar;   //Указатель на начало программы

    getToken(program);
    //Если лексема является меткой
    if (program->token.type == NUMBER) {
        strcpy(program->labels[0].name, program->token.name);
        program->labels[0].pointer = program->currentChar;
    }

    findEol(program);
    do {
        getToken(program);
        if (program->token.type == NUMBER) {
            location = getNextLabel(program, program->token.name);
            if (location == -1 || location == -2) {
                if (location == -1)
                    printError("Label table is full");
                else
                    printError("Duplicate labels");
            }
            strcpy(program->labels[location].name, program->token.name);
            program->labels[location].pointer = program->currentChar; //Текущий указатель программы
        }
        //Если строка не помечена, переход к следующей
        if (program->token.id != EOL) findEol(program);
    } while (program->token.id != FINISHED);
    program->currentChar = temp; //Восстанавливаем начальное значение
}
//Возвращает индекс на следующую свободную позицию массива меток
//  -1, если массив переполнен
//  -2, если дублирование меток
int getNextLabel(struct Program *program, char *s) {

    for (int i = 0; i < NUM_LABELS; i++) {
        if (program->labels[i].name[0] == 0) return i;
        if (!strcmp(program->labels[i].name, s)) return -2;
    }
    return -1;
}

void basicGosub(struct Program *program) {
    char *location;
    getToken(program);
    //Поиск метки вызова
    location = findLabel(program, program->token.name);
    if (location == '\0')
        printError("Undefined label"); //Метка не определена
    else {
        gPush(program, program->currentChar); //Запомним место, куда вернемся
        program->currentChar = location; //Старт программы с указанной точки
    }
}

//Возврат из подпрограммы
void basicReturn(struct Program *program) {
    program->currentChar = gPop(program);
}

//Помещает данные в стек GOSUB
void gPush(struct Program *program, char *s) {
    program->gosub.index++;
    if (program->gosub.index == GOSUB_NESTING) {
        printError("Label table is full");
        return;
    }
    program->gosub.stack[program->gosub.index] = s;
}

//Достает данные из стека GOSUB
char *gPop(struct Program *program) {
    if (program->gosub.index == 0) {
        printError("Undefined label");
        return '\0';
    }
    return (program->gosub.stack[program->gosub.index--]);
}