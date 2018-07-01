cmake_minimum_required(VERSION 3.10)
project(InterpreterBasic C)

set(CMAKE_C_STANDARD 99)

add_executable(InterpreterBasic main.c expression.c variables.c executor.c utilities.c main.h)