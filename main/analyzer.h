//
// Created by Алеша on 20.05.2018.
//

#ifndef INTERPRETERBASIC_ANALYZER_H
#define INTERPRETERBASIC_ANALYZER_H

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
#define END 11

char token[80]; //Строковое представление лексемы
int token_int; //Внутреннее представление лексемы
int token_type; //Тип лексемы

char *prog; //Указатель на анализируемое выражение

int getToken();

#endif //INTERPRETERBASIC_ANALYZER_H


