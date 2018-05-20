//
// Created by Алеша on 20.05.2018.
//

#ifndef INTERPRETERBASIC_CONSTANTS_H
#define INTERPRETERBASIC_CONSTANTS_H

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

#endif //INTERPRETERBASIC_CONSTANTS_H
