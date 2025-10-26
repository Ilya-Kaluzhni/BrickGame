#ifndef TESTS_H // защита от повторного включения: если TESTS_H не определён
#define TESTS_H // определяет макрос TESTS_H чтобы предотвратить повторное включение

#include <gtest/gtest.h> // подключает Google Test для написания и выполнения модульных тестов

#include <iostream> // подключает потоки ввода/вывода (std::cout, std::cerr) для отладки тестов

#include "../brick_game/brick_game_single.h" // подключает заголовок с определениями игры, структур и констант (WINDOW_WIDTH/HEIGHT и т.д.)

void find_obj_coords(int** field, int& coord_y, int& coord_x,
                     const int& obj_type); // прототип утилиты: ищет в матрице field координаты последнего вхождения obj_type и записывает их в coord_y/coord_x

void find_field_half(const int& apple_y, const int& apple_x, int& y_half,
                     int& x_half); // прототип утилиты: определяет, в какой трёхзонной разбиении поля (лево/центр/право и вверх/центр/низ) находится точка (apple_y, apple_x)

void horizontal_shift(s21::Game* snake_game, const int& x_half,
                      const int& apple_x, int& head_x); // прототип вспомогательной функции для тестов: выполняет последовательность вызовов КА, чтобы сдвинуть голову змейки по X в сторону apple_x

void vertical_shift(s21::Game* snake_game, const int& y_half, const int& x_half,
                    const int& apple_y, int& head_y); // прототип вспомогательной функции для тестов: выполняет последовательность вызовов КА, чтобы сдвинуть голову змейки по Y в сторону apple_y с учётом положения по X

#endif  // TESTS_H // конец защиты от повторного включения заголовка
