#include "tests.h" // подключает общий заголовок для тестов (Google Test и вспомогательные функции)
#include "../brick_game/brick_game_single.h" // подключает заголовок с определениями игры и константами
#include <cstring> // подключает C-функции работы с памятью/строками (например, memcmp)
#include <thread> // подключает возможности для работы с потоками (sleep_for)
#include <chrono> // подключает типы для выражений времени (milliseconds и т.д.)

using namespace std::chrono_literals; // позволяет использовать литералы времени, например 40ms, 1150ms


// Проверяет, что сразу после start() game_timer_check возвращает false
TEST(timer_tests, game_timer_check_immediate_false) { // объявление теста проверки немедленного состояния таймера
  s21::Timer t; // создаём объект таймера
  t.start(); // запускаем/сбрасываем таймер

  // Подбираем параметры так, чтобы задержка была достаточно большая (например 1000 ms)
  int speed = 1; // скорость 1 — минимальная
  int max_delay = 1000; // максимальная задержка в мс
  int min_delay = 1000; // минимальная задержка равна максимальной, чтобы задержка была фиксированной
  int max_speed = 10; // максимальное значение скорости для расчёта задержки

  bool res = t.game_timer_check(speed, max_delay, min_delay, max_speed); // проверяем, сработал ли таймер сразу после старта
  EXPECT_FALSE(res); // ожидаем, что таймер ещё не сработал
} // конец теста game_timer_check_immediate_false

// Проверяет, что после ожидания больше, чем рассчитанная задержка, game_timer_check возвращает true
TEST(timer_tests, game_timer_check_after_delay_true) { // тест ожидания и проверки срабатывания таймера
  s21::Timer t; // создаём объект таймера
  t.start(); // сбрасываем стартовое время таймера

  // Подберём небольшую задержку для быстрого теста
  int speed = 1; // минимальная скорость
  int max_delay = 20;   // 20 ms — верхняя граница задержки
  int min_delay = 10;   // 10 ms — нижняя граница задержки
  int max_speed = 10; // максимальная скорость

  // Немного подождём больше максимальной ожидаемой задержки
  std::this_thread::sleep_for(40ms); // приостанавливаем текущий поток на 40 миллисекунд

  bool res = t.game_timer_check(speed, max_delay, min_delay, max_speed); // проверяем, сработал ли таймер после ожидания
  EXPECT_TRUE(res); // ожидаем, что таймер вернёт true
} // конец теста game_timer_check_after_delay_true

// Тесты для get_miliseconds, get_seconds, get_minutes
TEST(timer_tests, time_components_reflect_elapsed_time) { // тест соответствия компонент времени прошедшему времени
  s21::Timer t; // создаём таймер
  t.start(); // сбрасываем старт таймера

  // Ждём 1 сек + 150 мс = 1150 ms
  std::this_thread::sleep_for(1150ms); // делаем паузу, чтобы накопить время

  double ms = t.get_miliseconds(); // получаем миллисекунды (в пределах секунды)
  int sec = t.get_seconds(); // получаем целые секунды (в пределах минуты)
  int min = t.get_minutes(); // получаем целые минуты (в пределах часа)

  // Проверяем секунды: ожидаем 1 (плюс-минус 1 на случай сдвига)
  EXPECT_TRUE(sec == 1 || sec == 0 || sec == 2); // допускаем небольшую погрешность из‑за планировщика ОС

  // Проверяем миллисекунды: ожидаем около 150 ms (допуск 200 ms для надёжности)
  EXPECT_GE(ms, 0.0); // миллисекунды не должны быть отрицательными
  EXPECT_LE(ms, 999.0); // миллисекунды находятся в диапазоне 0..999

  // Проверка конкретного приближения миллисекунд (около 150)
  // допускаем широкую границу из-за планировщика ОС
  EXPECT_TRUE(ms >= 50.0 && ms <= 500.0); // проверка, что миллисекунды в разумной окрестности 150ms

  // Минуты для 1150 ms — ожидаем 0
  EXPECT_EQ(min, 0); // минуты должны оставаться равными нулю
} // конец теста time_components_reflect_elapsed_time

// Проверяет, что updateCurrentState возвращает пустую GameInfo_t если игры нет
TEST(api_tests, updateCurrentState_no_game_returns_default) { // тест API для случая, когда игра не установлена
  // Постараемся убедиться, что игры нет. Если у вас есть метод reset, используйте его.
  // s21::GameFabric::reset(); // <-- при наличии метода сброса раскомментируйте

  // Получаем состояние без установленной игры
  GameInfo_t gi = updateCurrentState(); // вызываем API для получения текущего состояния

  // Сравним с дефолтно-конструированным GameInfo_t
  GameInfo_t expected{}; // создаём ожидание как нулевую/пустую структуру
  // Если у GameInfo_t определён оператор==, лучше использовать EXPECT_EQ(gi, expected);
  // Но чтобы не полагаться на наличие operator==, используем побитовую проверку.
  EXPECT_TRUE(std::memcmp(&gi, &expected, sizeof(GameInfo_t)) == 0); // сравниваем память структур побитово
} // конец теста updateCurrentState_no_game_returns_default


TEST(game_fabric_tests, get_non_existent_test) { // тест: при старте фабрика не должна возвращать игру по умолчанию
  s21::Game* game = s21::GameFabric::get_game(); // получаем текущую игру из фабрики
  EXPECT_EQ(game, nullptr); // ожидаем, что никакая игра ещё не установлена (nullptr)
} // конец теста get_non_existent_test

TEST(game_fabric_tests, set_snake) { // тест установки игры Snake
  s21::GameFabric::set_game(s21::GameFabric::GameName::Snake); // устанавливаем Snake через фабрику
  s21::Game* game = s21::GameFabric::get_game(); // получаем указатель на текущую игру
  EXPECT_NE(game, nullptr); // ожидаем, что указатель не нулевой (игра установлена)
} // конец теста set_snake

TEST(game_fabric_tests, set_tetris) { // тест установки игры Tetris
  s21::GameFabric::set_game(s21::GameFabric::GameName::Tetris); // устанавливаем Tetris через фабрику
  s21::Game* game = s21::GameFabric::get_game(); // получаем указатель на текущую игру
  EXPECT_NE(game, nullptr); // ожидаем, что указатель не нулевой
} // конец теста set_tetris

TEST(game_fabric_tests, set_unknown_game) { // тест обработки неизвестного имени игры
  EXPECT_THROW(s21::GameFabric::set_game(s21::GameFabric::GameName::EmptyGame),
               std::runtime_error); // ожидаем выброс std::runtime_error для пустого имени
} // конец теста set_unknown_game

TEST(game_fabric_tests, snake_not_tetris) { // тест, что после смены игры указатели различаются
  s21::GameFabric::set_game(s21::GameFabric::GameName::Snake); // устанавливаем Snake
  s21::Game* snake_game = s21::GameFabric::get_game(); // сохраняем указатель на Snake
  s21::GameFabric::set_game(s21::GameFabric::GameName::Tetris); // устанавливаем Tetris
  s21::Game* tetris_game = s21::GameFabric::get_game(); // получаем указатель на Tetris
  EXPECT_NE(snake_game, tetris_game); // ожидаем, что указатели на разные игры различны
} // конец теста snake_not_tetris

TEST(snake_test, start) { // тест начального состояния игры Snake (Start)
  s21::GameFabric::set_game(s21::GameFabric::GameName::Snake); // выбираем Snake
  s21::Game* snake_game = s21::GameFabric::get_game(); // получаем указатель на игру
  snake_game->fsm(); // выполняем один шаг КА (GameStart -> Spawn/инициализация)
  GameInfo_t actual_info = snake_game->get_gameinfo(); // получаем текущую структуру gameinfo
  EXPECT_NE(actual_info.field, nullptr); // поле field должно быть инициализировано
  EXPECT_NE(actual_info.next, nullptr); // поле next должно быть инициализировано
  EXPECT_EQ(actual_info.level, 1); // ожидание, что уровень установлен в 1 после старта
} // конец теста snake_test.start

TEST(snake_test, spawn) { // тест корректной генерации яблока на поле при Spawn
  s21::GameFabric::set_game(s21::GameFabric::GameName::Snake); // выбираем Snake
  s21::Game* snake_game = s21::GameFabric::get_game(); // получаем указатель на игру
  snake_game->fsm(); // выполняем шаг КА для старта и спауна
  GameInfo_t actual_info = snake_game->get_gameinfo(); // получаем текущее состояние поля
  int apple = 0; // счётчик яблок на поле
  for (auto i = 0; i < WINDOW_HEIGHT; i++) { // проход по строкам поля
    for (auto j = 0; j < WINDOW_WIDTH; j++) { // проход по столбцам поля
      if (actual_info.field[i][j] == 2) { // если ячейка содержит маркер яблока (значение 2)
        apple++; // увеличиваем счётчик найденных яблок
      } // конец проверки ячейки
    } // конец внутреннего цикла
  } // конец внешнего цикла
  EXPECT_EQ(apple, 1); // ожидаем, что на поле ровно одно яблоко
} // конец теста snake_test.spawn

TEST(snake_test, pause) { // тест установки паузы
  s21::GameFabric::set_game(s21::GameFabric::GameName::Snake); // выбираем Snake
  s21::Game* snake_game = s21::GameFabric::get_game(); // получаем указатель на игру
  snake_game->set_user_action(UserAction_t::Pause); // передаём действие Pause
  snake_game->fsm(); // выполняем шаг КА, который должен обработать паузу
  GameInfo_t actual_info = snake_game->get_gameinfo(); // получаем текущее состояние
  EXPECT_EQ(actual_info.pause, 1); // ожидаем, что pause установлен в 1
} // конец теста snake_test.pause

TEST(snake_test, unpause) { // тест снятия паузы (повторное нажатие Pause)
  s21::GameFabric::set_game(s21::GameFabric::GameName::Snake); // выбираем Snake
  s21::Game* snake_game = s21::GameFabric::get_game(); // получаем игру
  snake_game->set_user_action(UserAction_t::Pause); // посылаем Pause (в коде переключается состояние)
  snake_game->fsm(); // выполняем шаг КА
  GameInfo_t actual_info = snake_game->get_gameinfo(); // получаем состояние
  EXPECT_EQ(actual_info.pause, 0); // ожидаем, что пауза снята (вернулось в исходное состояние)
} // конец теста snake_test.unpause

TEST(snake_test, action) { // тест, проверяющий, что действие приводит к изменению положения головы змейки
  s21::GameFabric::set_game(s21::GameFabric::GameName::Snake); // выбираем Snake
  s21::Game* snake_game = s21::GameFabric::get_game(); // получаем указатель
  GameInfo_t start_point = snake_game->get_gameinfo(); // сохраняем начальную структуру поля
  int start_y = 0; // локальная переменная для координаты Y головы в начале
  int start_x = 0; // локальная переменная для координаты X головы в начале
  int end_y = 0; // локальная переменная для координаты Y головы в конце
  int end_x = 0; // локальная переменная для координаты X головы в конце

  find_obj_coords(start_point.field, start_y, start_x, 4); // находим координаты головы (значение 4) в начальном поле
  snake_game->set_user_action(UserAction_t::Action); // задаём действие Action (ход)
  snake_game->fsm();  // Moving — выполняем перемещение
  snake_game->fsm();  // Shifting — выполняем сдвиг вниз/вперёд по таймеру
  GameInfo_t end_point = snake_game->get_gameinfo(); // получаем обновлённое состояние поля
  find_obj_coords(end_point.field, end_y, end_x, 4); // находим координаты головы в конце

  EXPECT_NE(start_y, end_y); // проверяем, что Y координата головы изменилась (голова двинулась)
} // конец теста snake_test.action

TEST(snake_test, incorrect_turn) { // тест, что неподходящее действие не меняет положение головы
  s21::GameFabric::set_game(s21::GameFabric::GameName::Snake); // выбираем Snake
  s21::Game* snake_game = s21::GameFabric::get_game(); // получаем игру
  GameInfo_t start_point = snake_game->get_gameinfo(); // начинаем с текущего состояния
  int start_y = 0; // переменные для хранения координат головы до и после
  int start_x = 0;
  int end_y = 0;
  int end_x = 0;

  find_obj_coords(start_point.field, start_y, start_x, 4); // находим координаты головы в начальном состоянии
  snake_game->set_user_action(UserAction_t::Down); // пытаемся выполнить некорректный поворот/действие Down
  snake_game->fsm();  // Moving
  snake_game->fsm();  // Shifting
  GameInfo_t end_point = snake_game->get_gameinfo(); // получаем состояние после попытки действия
  find_obj_coords(end_point.field, end_y, end_x, 4); // находим координаты головы после

  EXPECT_EQ(start_y, end_y); // ожидаем, что координаты не изменились
  EXPECT_EQ(start_x, end_x);
  EXPECT_EQ(start_x - end_x, 0); // дополнительная проверка равенства X
} // конец теста snake_test.incorrect_turn

TEST(snake_test, attaching_apple) { // интеграционный тест: движение змейки к яблоку и сбор
  s21::GameFabric::set_game(s21::GameFabric::GameName::Snake); // выбираем Snake
  s21::Game* snake_game = s21::GameFabric::get_game(); // получаем указатель на игру
  GameInfo_t start_point = snake_game->get_gameinfo(); // получаем начальное состояние поля
  int apple_y = 0; // координаты яблока
  int apple_x = 0;
  int head_y = 0; // координаты головы
  int head_x = 0;
  int x_half = 0; // номера половин поля для планирования маршрута
  int y_half = 0;
  find_obj_coords(start_point.field, apple_y, apple_x, 2); // находим яблоко (значение 2)
  find_obj_coords(start_point.field, head_y, head_x, 4); // находим голову (значение 4)
  find_field_half(apple_y, apple_x, y_half, x_half); // определяем половины поля где находится яблоко
  horizontal_shift(snake_game, x_half, apple_x, head_x); // выполняем горизонтальное выравнивание головы к яблоку
  vertical_shift(snake_game, y_half, x_half, apple_y, head_y); // выполняем вертикальное выравнивание головы к яблоку

  snake_game->fsm();  // Attaching — прикрепляем фигуру и обновляем счёт
  GameInfo_t end_point = snake_game->get_gameinfo(); // получаем состояние после прикрепления
  EXPECT_EQ(end_point.score, 1); // ожидаем, что счёт увеличился на 1 после поедания яблока
} // конец теста snake_test.attaching_apple

TEST(snake_test, game_over) { // тест обработки завершения игры для Snake (Terminate -> GameOver)
  s21::GameFabric::set_game(s21::GameFabric::GameName::Snake); // выбираем Snake
  s21::Game* snake_game = s21::GameFabric::get_game(); // получаем игру
  snake_game->set_user_action(UserAction_t::Terminate); // устанавливаем действие Terminate (выход)
  snake_game->fsm();  // Spawn — шаг КА
  snake_game->fsm();  // Moving — шаг КА
  snake_game->fsm();  // GameOver — шаг КА для завершения
  GameInfo_t game_info = snake_game->get_gameinfo(); // получаем итоговое состояние
  EXPECT_EQ(game_info.level, -1); // ожидаем, что уровень/код состояния равен -1 (LOSE_LVL)
} // конец теста snake_test.game_over
