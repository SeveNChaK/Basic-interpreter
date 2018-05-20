#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#define PROG_SIZE 10000

extern char *prog; //Буфер анализируемого выражения
extern jmp_buf e_buf; //Буфер среды функции longjmp()

int main(int argc, char *argv) {
    char *p_buf; //Указатель начала буфера программы
    char *file_name = &argv[1]; //Имя файла программы

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

    if (setjmp(e_buf)) exit(1); //Инициализация буфера нелокальных переходов

    prog = p_buf;

    do {
        token_type = (char) get_token();

        //Проверка на присваивание
        if (token_type == VARIABLE) {
            putback();
            assignment();
        } else { //Значит это команда
            switch (token_int) {
                case PRINT:
                    basic_print();
                    break;
                case END:
                    exit(0);
            }
        }
    } while (token_int != FINISHED);
}
