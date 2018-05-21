#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <mem.h>
#include <ctype.h>

//Мои штуки
#include "analyzer.h"

//-------------------------------------------------------------

#define PROG_SIZE 10000

//Объявление переменных
char *program;
jmp_buf e_buf; //Буфер среды функции longjmp()

//Объявление функций
int loadProgram(char*, char*); //Считывает программу

/*
 * Переменных пока нет!
 * Реализован IF <, >, = THEN: Без ELSE; Команда записывается в одну строчку
 * Реализован PRINT: параметры перечисляются через запятую
 * Реализованы + - / % * и выражения в скобках
 */

int main(int argc, char *argv[]) {
    char *p_buf; //Указатель начала буфера программы
    char *file_name = argv[1]; //Имя файла программы

    if (argc != 2) {
        printf("Используйте формат: <исполняемый файл>.exe <файл программы>.txt");
        exit(1);
    }

    //Выделение памяти для программы
    if (!(p_buf = (char *) malloc(PROG_SIZE))) {
        printf("Ошибка при выделении памяти ERROR");
        exit(1);
    }

    //Загрузка программы
    if (!loadProgram(p_buf, file_name)) exit(1);

//    if (setjmp(e_buf)) exit(1); //Инициализация буфера нелокальных переходов

    program = p_buf;

    start(program);

    //TODO
}

int loadProgram(char *p, char *fname) {
    FILE *file;

    if (!(file = fopen(fname, "r"))) //Открываем только на чтение
        return 0;

    //Считываем текст программы в память
    int i = 0;
    do {
        *p = (char) getc(file);
        p++;
        i++;
    } while (!feof(file) && i < PROG_SIZE);

    *(p - 1) = '\0'; //Символ конца программы
    fclose(file);
    return 1;
}

