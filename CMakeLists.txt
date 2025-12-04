cmake_minimum_required(VERSION 4.0)
project(lab3 C)

set(CMAKE_C_STANDARD 11)

add_executable(lab3 main.c
        src/polynomial.c
        include/polynomial.h
        src/test.c
        include/test.h
        src/string_utils.c
        include/string_utils.h)
