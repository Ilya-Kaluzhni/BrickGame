#pragma once // защита от многократного включения заголовка — альтернативa include guards

#include <stdexcept> // подключает исключения стандартной библиотеки (std::runtime_error и др.)
#include <chrono> // подключает возможности работы со временем и таймерами
#include <iostream> // подключает потоки ввода/вывода (std::cout, std::cerr и т.д.)

// --- defines.h ---
#define WINDOW_HEIGHT 20 // высота игрового окна (число строк игрового поля)
#define WINDOW_WIDTH 10 // ширина игрового окна (число столбцов игрового поля)
#define NEXT_SIZE 7 // размер области отображения следующей фигуры (NEXT_SIZE x NEXT_SIZE)

#define WIN_LVL 200 // код уровня/статуса для победы
#define LOSE_LVL -1 // код уровня/статуса для поражения / выхода из игры

// --- specification.h ---
typedef enum { // перечисление возможных действий пользователя
  Start = 0, // действие "Start" (начало / сброс)
  Pause, // действие "Pause" (пауза)
  Terminate, // действие "Terminate" (завершение игры)
  Left, // действие "Left" (сдвиг влево)
  Right, // действие "Right" (сдвиг вправо)
  Up, // действие "Up" (сдвиг вверх / поворот в некоторых играх)
  Down, // действие "Down" (сдвиг вниз / ускорение падения)
  Action // действие "Action" (действие, например, поворот)
} UserAction_t; // тип UserAction_t используется для передачи действий игрока

typedef struct { // структура для хранения информации об игре
  int **field; // указатель на матрицу игрового поля
  int **next; // указатель на матрицу для показа следующей фигуры
  int score; // текущее количество очков
  int high_score; // рекорд игрока
  int level; // текущий уровень игры или код состояния завершения
  int speed; // текущая скорость игры (влияет на таймеры)
  int pause; // флаг паузы (0 или 1)
} GameInfo_t; // имя типа — GameInfo_t

// Forward declarations
namespace s21 { // начало пространства имён s21
class Game; // предварительное объявление класса Game
class GameFabric; // предварительное объявление класса GameFabric
} // конец пространства имён s21

void userInput(UserAction_t action, bool hold); // прототип глобальной функции API для передачи ввода пользователя
GameInfo_t updateCurrentState(); // прототип глобальной функции API для обновления и получения текущего состояния игры

// --- game.h ---
namespace s21 { // начало пространства имён s21

class Game { // объявление абстрактного базового класса Game
 public:
  void set_user_action(UserAction_t user_input); // метод установки действия пользователя
  const GameInfo_t& get_gameinfo(); // метод получения константной ссылки на структуру gameinfo
  void fsm(); // метод выполнения одного шага конечного автомата игры

 protected:
  enum State_of_machine { GameStart = 0, Spawn, Moving, Shifting, Attaching, GameOver }; // перечисление состояний КА

  GameInfo_t gameinfo; // поле для хранения информации об игре
  UserAction_t action; // текущее действие пользователя, ожидаемое/обрабатываемое игрой
  State_of_machine statemachine; // текущее состояние конечного автомата

  Game(); // защищённый конструктор базового класса
  virtual ~Game(); // виртуальный защищённый деструктор базового класса

 private:
  virtual void starting_game() = 0; // чисто виртуальная функция для обработки состояния GameStart
  virtual void spawn() = 0; // чисто виртуальная функция для обработки состояния Spawn
  virtual void moving() = 0; // чисто виртуальная функция для обработки состояния Moving
  virtual void shifting() = 0; // чисто виртуальная функция для обработки состояния Shifting
  virtual void attaching() = 0; // чисто виртуальная функция для обработки состояния Attaching
  virtual void game_over() = 0; // чисто виртуальная функция для обработки состояния GameOver

  int** matrix_init(const int rows, const int cols); // выделение и инициализация матрицы rows x cols
  void matrix_free(int** matrix, const int rows); // освобождение памяти матрицы с указанным числом строк
}; // конец объявления класса Game

class GameFabric { // фабрика для выбора и хранения текущей игры
 private:
  inline static Game* current_game = nullptr; // статический указатель на текущую выбранную игру, инициализированный nullptr

 public:
  enum class GameName { EmptyGame = 0, Tetris, Snake }; // перечисление доступных имён/типов игр в фабрике

  static void set_game(GameName name); // статический метод установки текущей игры по имени
  static Game* get_game(); // статический метод получения указателя на текущую игру
}; // конец объявления класса GameFabric

class Timer { // класс-обёртка для замеров времени и расчёта задержек игрового шага
 private:
  using ClockType = std::chrono::high_resolution_clock; // тип часов высокой точности
  using TimePoint = std::chrono::time_point<ClockType>; // тип точки времени на основе выбранных часов
  using DurationMs = std::chrono::milliseconds; // тип длительности в миллисекундах

  TimePoint start_time_; // поле для хранения времени старта/перезапуска таймера

 public:
  Timer(); // конструктор инициализирует start_time_

  void start(); // метод перезапуска таймера (установка текущего времени в start_time_)
  bool game_timer_check(int speed, int max_delay, int min_delay, int max_speed); // проверяет, истёк ли интервал для шага при данных параметрах
  double get_miliseconds() const; // возвращает миллисекунды из прошедшего времени в пределах секунды
  int get_seconds() const; // возвращает секунды из прошедшего времени в пределах минуты
  int get_minutes() const; // возвращает минуты из прошедшего времени в пределах часа

 private:
  DurationMs get_elapsed_time() const; // возвращает прошедшее время как DurationMs
  DurationMs calculate_delay(int speed, int max_delay, int min_delay, int max_speed) const; // вычисляет задержку на основе скорости и лимитов
}; // конец объявления класса Timer

}  // namespace s21 // конец пространства имён s21
