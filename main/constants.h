//
// Created by Алеша on 21.05.2018.
//

#ifndef INTERPRETERBASIC_CONSTANTS_H
#define INTERPRETERBASIC_CONSTANTS_H

//Типы лексем
#define DELIMITER  1 //Разделитель
#define VARIABLE   2 //Переменная
#define NUMBER     3 //Число
#define COMMAND    4 //Команда
#define QUOTE      5 //Кавычки

//Внутренние представления лексем
#define PRINT 10
#define INPUT 11 //TODO
#define IF 12
#define THEN 13
#define GOTO 15
#define GOSUB 16 //TODO
#define RETURN 17 //TODO
#define EOL 18 //Конец строки файла
#define END 19 //Конец файла
#define FINISHED 20 //Конец программы

#endif //INTERPRETERBASIC_CONSTANTS_H
