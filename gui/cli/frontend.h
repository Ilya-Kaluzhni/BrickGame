#ifndef FRONTEND_H // защита от повторного включения заголовка: если макрос не определён
#define FRONTEND_H // определяем макрос чтобы избежать повторного включения

#include <ncurses.h> // подключаем библиотеку ncurses для работы с терминальным UI
#include <time.h> // подключаем функции времени (time, srand и т.д.)
#include <unistd.h> // подключаем POSIX функции (sleep, usleep и т.д.)
#include <stdlib.h> // подключаем стандартные функции C (malloc, rand и т.д.)
#include <string.h> // подключаем функции работы со строками C (strlen, memcpy и т.д.)

#include "../../brick_game/brick_game_single.h" // подключаем общий заголовок с игровыми структурами и константами

#define Tetris 1 // макрос-код для выбора игры Tetris
#define Snake 2 // макрос-код для выбора игры Snake

void ncurses_init(); // прототип функции инициализации ncurses и базовых настроек терминала
void game_loop(WINDOW* my_win); // прототип главного игрового цикла, принимает окно ncurses
void set_user_action(); // прототип функции обработки ввода пользователя и преобразования в действия
bool is_end(GameInfo_t stats); // прототип функции проверки состояния завершения игры по gameinfo
void update_screen(GameInfo_t stats, WINDOW* local_win); // прототип функции обновления экрана на основе gameinfo
void score_to_string(char* str, int score); // прототип функции форматирования числа в строку фиксированной длины
int selection_game(WINDOW* local_win); // прототип функции меню выбора игры, возвращает код выбранной игры

WINDOW* create_new_window(); // прототип функции создания и возвращения нового окна ncurses
void destroy_win(WINDOW* local_win); // прототип функции уничтожения окна и очистки ресурсов

void print_interface(WINDOW* local_win); // прототип функции отрисовки статической части интерфейса (рамки, подписи)
void print_tetris(WINDOW* local_win); // прототип функции вывода текста TETRIS в окне
void print_snake(WINDOW* local_win); // прототип функции вывода текста SNAKE в окне
void print_pause(WINDOW* local_win); // прототип функции вывода текста PAUSE в окне
void print_end(WINDOW* local_win); // прототип функции вывода текста GAME OVER в окне
void print_win(WINDOW* local_win); // прототип функции вывода текста YOU WIN в окне
void print_stats_field(GameInfo_t stats, WINDOW* local_win); // прототип функции отрисовки основного игрового поля в окне
void print_stats_next(GameInfo_t stats, WINDOW* local_win); // прототип функции отрисовки области NEXT в окне

#endif  // FRONTEND_H // конец защиты от повторного включения заголовка