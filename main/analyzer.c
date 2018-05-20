#include <stdio.h>
#include <ctype.h>
#include <mem.h>

//Типы лексем
#define DELIMITER  1 //Разделитель
#define VARIABLE   2 //Переменная
#define NUMBER     3 //Число
#define COMMAND    4 //Команда
#define STRING     5 //Временное представление анализируемого выражениея в get_token()
#define QUOTE      6 //Кавычки

//Внутренние представления лексем
#define PRINT 8
#define EOL        9 //Конец файла
#define FINISHED   10 //Конец программы

char token[80]; //Строковое представление лексемы
int token_int; //Внутреннее представление лексемы
int token_type; //Тип лексемы

char *prog; //Указатель на анализируемое выражение

int isdelim(char c) {
    if(strchr(" !;,+-<>'/*%^=()", c) || c == 9 ||
       c == '\r' || c == 0) return 1;
    return 0;
}

/*
 * Получаем лексему
 */
int getToken() {
    char *temp; //Указатель на лексему

    token_type = 0;
    token_int = 0;
    temp = token;

    //Проверка закончился ли файл интерпретируемой программы
    if (*prog == '\0') {
        *token = '\0';
        token_int = FINISHED;
        return (token_type = DELIMITER);
    }

    while (isspace(*prog)) prog++; //Пропускаем пробелы

    //Проверка на конец строки программы
    if (*prog == '\r') {
        prog += 2; //Тут были возможно значемые строки
        token_int = EOL;
        token[0] = '\r';
        token[1] = '\n';
       //
        return (token_type = DELIMITER);
    }

    //Проверка переменная или команда
    if (isalpha(*prog)) {
        while (!isdelim(*prog))
            prog++;
        token_type = STRING;
    }

    if (token_type == STRING) {
//        token_int = look_up(token); //Проверка на команду
//        if (!token_int) token_type = VARIABLE;
//        else token_type = COMMAND;
        token_type = COMMAND; //Заглушка
        token_int = PRINT;
        return token_type;
    }
    return token_type = FINISHED; //Заглушка
}
