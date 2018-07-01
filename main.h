#ifndef INTERPRETERBASIC_MAIN_H
#define INTERPRETERBASIC_MAIN_H

//Типы лексем
#define DELIMITER  1 //Разделитель
#define VARIABLE   2 //Переменная
#define NUMBER     3 //Число
#define COMMAND    4 //Команда
#define QUOTE      5 //Кавычки

//Внутренние представления лексем
#define PRINT 10
#define INPUT 11
#define IF 12
#define THEN 13
#define ELSE 14
#define GOTO 15
#define GOSUB 16
#define RETURN 17
#define EOL 18 //Конец строки файла
#define END 19 //Конец файла
#define FINISHED 20 //Конец программы
#define ENDFI 21

#define NUM_LABELS 100
#define GOSUB_NESTING 25

struct Lexem {
    char *name;
    int id;
    int type;
};
struct Variable {
    char *name;
    int value;
};
struct InfoVariables {
    struct Variable *vars;
    int countVars;
};
struct Label {
    char name[3];
    char *pointer;
};
struct Gosub {
    char *stack[GOSUB_NESTING];
    int index;
};
struct Program {
    struct Lexem token;
    char *currentChar;
    struct InfoVariables infoVariables;
    struct Label labels[NUM_LABELS];
    struct Gosub gosub;
};

int length(char *name);
void execute(struct Program *program);
void assignment(struct Program *program),
        calcExpression(struct Program *program, int *result);

void printError(char *error),
        putBack(struct Program *program);

struct Variable *findVariable(struct Program *program, char *name),
        *addVariable(struct Program *program, char *name);

void getToken(struct Program *program);
char *mallocAndCopy(char *source, int steps);

#endif //INTERPRETERBASIC_MAIN_H
