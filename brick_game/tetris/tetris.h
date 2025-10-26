#ifndef TETRIS_H // защита от повторного включения заголовка: если TETRIS_H не определён
#define TETRIS_H // определяет макрос TETRIS_H чтобы предотвратить повторное включение

#include <chrono> // подключает заголовок для работы со временем и таймерами
#include <iostream> // подключает заголовок для ввода/вывода в потоках
#include <stdbool.h> // подключает стандартный заголовок для типа bool в C-стиле
#include <stdio.h> // подключает стандартный C-заголовок для ввода/вывода файлов и консоли
#include <string.h> // подключает заголовок для работы со строками C (memcpy, memset и т.д.)
#include <unistd.h> // подключает POSIX-заголовок для системных вызовов и функций (sleep, usleep и т.д.)

#include "../brick_game_single.h" // подключает общий заголовок с базовыми типами и абстрактным классом Game

#define BRICK_SIZE 8 // задаёт размер описания фигуры в массиве (количество строк/строчек в шаблоне)

namespace s21 { // начало пространства имён s21

/**
 * @brief Класс тетриса.
 *
 * Наследуется от абстрактного класса Game.
 * Интерфейс представлен функцией get_instance(), которая дает доступ к единственному экземпляру класса.
 */
class Tetris : public Game { // объявление класса Tetris, наследника Game
 private: // начало секции приватных членов класса
  Tetris() = default; // приватный конструктор по умолчанию, предотвращает прямое создание извне
  ~Tetris() {} // приватный деструктор по умолчанию
  Tetris(const Tetris&) = delete; // удалённый копирующий конструктор, запрет копирования
  Tetris& operator=(const Tetris&) = delete; // удалённый оператор присваивания, запрет копирования

  void starting_game() override; // переопределённый метод начальной установки игры
  void spawn() override; // переопределённый метод появления новой кирпичной фигуры
  void moving() override; // переопределённый метод обработки движения фигуры
  void shifting() override; // переопределённый метод сдвига / обновления состояния
  void attaching() override; // переопределённый метод прикрепления фигуры к полю
  void game_over() override; // переопределённый метод обработки завершения игры

 public: // начало секции публичных членов класса
  static Tetris* get_instance() { // статический метод доступа к единственному экземпляру (синглтон)
    static Tetris instance; // локальный статический экземпляр класса, обеспечивающий единственность
    return &instance; // возвращает указатель на единственный экземпляр
  } // конец метода get_instance

 private: // приватная секция для внутренних структур и данных
  typedef struct { // определение структуры содержащей шаблоны всех фигур тетриса
    int Teewee[BRICK_SIZE]; // шаблон фигуры Teewee в массиве размером BRICK_SIZE
    int Hero[BRICK_SIZE]; // шаблон фигуры Hero
    int Smashboy[BRICK_SIZE]; // шаблон фигуры Smashboy
    int Orange_Ricky[BRICK_SIZE]; // шаблон фигуры Orange Ricky
    int Blue_Ricky[BRICK_SIZE]; // шаблон фигуры Blue Ricky
    int Cleveland_Z[BRICK_SIZE]; // шаблон фигуры Cleveland Z
    int Rhode_Island_Z[BRICK_SIZE]; // шаблон фигуры Rhode Island Z
  } Bricks; // имя типа структуры — Bricks

  int* current_brick; // указатель на массив/шаблон текущей фигуры
  int* next_brick; // указатель на массив/шаблон следующей фигуры

  int current_color; // цвет/идентификатор цвета текущей фигуры
  int next_color; // цвет следующей фигуры

  Timer time; // объект таймера для отсчёта времени игры/скорости падения

 private: // приватная секция для вспомогательных методов
  int** init_matrix(int** matrix, int height, int width); // инициализация матрицы игрового поля
  void free_memory_matrix(int** matrix, int size); // освобождение памяти матрицы размером size
  void fill_array_zero(int** matrix, int height, int width); // заполнение матрицы нулями по заданным размерам
  void stats_init(Tetris* tetris); // инициализация статистики для переданного экземпляра Tetris
  int init_score(Tetris* tetris); // инициализация счёта и возвращение стартового значения
  int rotate_check_down_wall(int* brick); // проверка поворота относительно нижней границы
  int rotate_check_up_wall(int* brick); // проверка поворота относительно верхней границы
  int rotate_check_right_wall(int* brick); // проверка поворота относительно правой границы
  int rotate_check_left_wall(int* brick); // проверка поворота относительно левой границы
  int rotate_check_field(int** matrix, int* brick); // проверка поворота относительно занятых ячеек поля
  void new_brick(int* brick, int random); // наполнение массива brick шаблоном на основе случайного индекса
  void brick_copy(int* src, int* other); // копирование данных одной фигуры в другую
  void spawn_brick(int** matrix, int* array, int color); // размещение фигуры array на поле matrix с цветом color
  void despawn(int** matrix, int* array); // удаление отображения фигуры array с поля matrix
  void brick_move(int* brick, UserAction_t state, int** matrix); // обработка перемещения/действия над фигурой в зависимости от состояния игрока
  int check_right(int** matrix, int* brick, int shift); // проверка возможности сдвига фигуры вправо с учётом сдвига shift
  int check_left(int** matrix, int* brick, int shift); // проверка возможности сдвига фигуры влево с учётом сдвига shift
  int check_down(int** matrix, int* brick); // проверка возможности опускания фигуры вниз
  int is_Smashboy(int* brick); // проверка, соответствует ли фигура шаблону Smashboy
  int check_gameover(int* brick); // проверка условия окончания игры для текущей фигуры
  int check_attaching(int* brick, int** matrix); // проверка необходимости прикрепления фигуры к полю
  int is_Hero(int* brick); // проверка, соответствует ли фигура шаблону Hero
  void coord_shift(int* brick, int cords, int shift); // сдвиг координат фигуры в массиве brick на значение shift по индексу cords
  void rotate(int** matrix, int* brick, int size); // выполнение поворота фигуры размером size с учётом матрицы поля
  void fix_brick_coord(int* brick); // корректировка координат фигуры после операций (поворот/сдвиг)
  int check_full_row(int** field); // проверка поля на заполненные строки и возвращение их количества
  void score_write(Tetris* tetris, int full_rows_counter); // обновление счёта в зависимости от количества удалённых строк
  void check_level(Tetris* tetris); // проверка и обновление уровня игры для указанного экземпляра

}; // конец объявления класса Tetris

}  // namespace s21 // конец пространства имён s21

#endif  // TETRIS_H // конец защиты от повторного включения заголовка
