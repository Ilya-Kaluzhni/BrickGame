// tests/tetris_tests.cpp
#include <gtest/gtest.h> // подключает фреймворк Google Test для определения тестов и макросов EXPECT_*
#include <cstring> // подключает C-функции для работы с памятью, например memcpy
#include <cstdio> // подключает C-функции ввода/вывода, например remove, fopen
#include <cstdlib> // подключает C-функции stdlib, например remove
#include <thread> // подключает std::this_thread::sleep_for для пауз в тестах
#include <chrono> // подключает типы времени (milliseconds и пр.) для задержек

// Сделать private/protected публичными для тестов — обязательно до включения заголовка tetris.h
#define private public // временно переопределяем private на public чтобы тесты могли обращаться к внутренним методам
#define protected public // временно переопределяем protected на public для доступа к защищённым членам
#include "../brick_game/tetris/tetris.h" // подключаем реализацию Tetris, теперь с раскрытыми модификаторами доступа
#undef private // восстанавливаем оригинальное значение private
#undef protected // восстанавливаем оригинальное значение protected

using s21::Tetris; // импортируем имя класса Tetris в локальное пространство имён теста

// Вспомогательная функция для обнуления матрицы
static void zero_matrix(int** m, int h, int w) { // обнуляет матрицу размером h x w
  for (int i = 0; i < h; ++i) // проходим по строкам
    for (int j = 0; j < w; ++j) // проходим по столбцам
      m[i][j] = 0; // устанавливаем ячейку в 0
}


TEST(tetris_backend, is_Smashboy_and_is_Hero_and_coord_shift) { // тест проверки распознавания фигур и сдвига координат
  Tetris *t = Tetris::get_instance(); // получаем единственный экземпляр Tetris (синглтон)
  int brick[BRICK_SIZE]; // массив для хранения координат фигуры (Y,X пары)

  // Smashboy (square)
  brick[0] = 0; brick[1] = 0; // блок 1 (Y0, X0)
  brick[2] = 0; brick[3] = 1; // блок 2 (Y0, X1)
  brick[4] = 1; brick[5] = 0; // блок 3 (Y1, X0)
  brick[6] = 1; brick[7] = 1; // блок 4 (Y1, X1)
  EXPECT_EQ(t->is_Smashboy(brick), 1); // ожидаем, что это квадрат (Smashboy)

  // Not Smashboy
  brick[6] = 2; brick[7] = 1; // модифицируем один блок, делая фигуру не квадратом
  EXPECT_EQ(t->is_Smashboy(brick), 0); // ожидаем отрицательный результат

  // Hero horizontal
  brick[0] = 5; brick[1] = 1; // задаём горизонтальную линию (Y одинаковы)
  brick[2] = 5; brick[3] = 2;
  brick[4] = 5; brick[5] = 3;
  brick[6] = 5; brick[7] = 4;
  EXPECT_EQ(t->is_Hero(brick), 1); // ожидаем, что это Hero (линия)

  // Hero vertical
  brick[0] = 1; brick[1] = 7; // задаём вертикальную линию (X одинаковы)
  brick[2] = 2; brick[3] = 7;
  brick[4] = 3; brick[5] = 7;
  brick[6] = 4; brick[7] = 7;
  EXPECT_EQ(t->is_Hero(brick), 1); // ожидаем, что это Hero

  // coord_shift Y then X
  int cs[BRICK_SIZE] = {0,0, 1,1, 2,2, 3,3}; // начальные координаты (Y,X pairs)
  t->coord_shift(cs, 0, 2); // сдвигаем все Y на +2
  EXPECT_EQ(cs[0], 2); // проверяем первый Y
  EXPECT_EQ(cs[2], 3); // проверяем второй Y
  EXPECT_EQ(cs[4], 4); // проверяем третий Y
  EXPECT_EQ(cs[6], 5); // проверяем четвёртый Y
  t->coord_shift(cs, 1, -1); // сдвигаем все X на -1
  EXPECT_EQ(cs[1], -1); // проверяем первый X
  EXPECT_EQ(cs[3], 0); // проверяем второй X
  EXPECT_EQ(cs[5], 1); // проверяем третий X
  EXPECT_EQ(cs[7], 2); // проверяем четвёртый X
}

TEST(tetris_backend, rotate_check_walls_basic) { // тест базовых проверок столкновений с границами при повороте
  Tetris *t = Tetris::get_instance(); // получаем синглтон

  int brick_down[BRICK_SIZE] = { WINDOW_HEIGHT, 1, WINDOW_HEIGHT + 1, 2, 5, 3, 6, 4 }; // пример с Y за нижней границей
  EXPECT_EQ(t->rotate_check_down_wall(brick_down), 0); // ожидаем столкновение с нижней стенкой

  int brick_ok[BRICK_SIZE] = {0,0, 1,1, 2,2, 3,3}; // корректный набор координат
  EXPECT_EQ(t->rotate_check_down_wall(brick_ok), 1); // не должен быть выход за нижнюю границу

  int brick_left[BRICK_SIZE] = {0,-1, 1,0, 2,1, 3,2}; // пример с X < 0
  EXPECT_EQ(t->rotate_check_left_wall(brick_left), 0); // ожидаем столкновение с левой границей
  EXPECT_EQ(t->rotate_check_left_wall(brick_ok), 1); // корректный набор не должен выдавать столкновение

  int brick_right[BRICK_SIZE] = {0, WINDOW_WIDTH, 1, WINDOW_WIDTH + 1, 2,3, 3,4}; // пример с X >= WINDOW_WIDTH
  EXPECT_EQ(t->rotate_check_right_wall(brick_right), 0); // ожидаем столкновение с правой границей
  EXPECT_EQ(t->rotate_check_right_wall(brick_ok), 1); // корректный набор разрешён

  int brick_up[BRICK_SIZE] = {-1, 0, 1,1, 2,2, 3,3}; // пример с Y < 0
  EXPECT_EQ(t->rotate_check_up_wall(brick_up), 0); // сталкиваемся с верхней границей
  EXPECT_EQ(t->rotate_check_up_wall(brick_ok), 1); // корректный набор не выходит за верх
}

TEST(tetris_more, stats_init_init_score_and_game_over) { // тест инициализации статистики, счёта и логики завершения игры
  Tetris *t = Tetris::get_instance(); // получаем синглтон

#ifdef DATA_FILE_NAME
  remove(DATA_FILE_NAME); // удаляем файл рекорда если он существует, чтобы начать тест в чистой среде
#endif

  t->stats_init(t); // инициализируем статистику и выделяем память под фигуры
  EXPECT_NE(t->current_brick, nullptr); // проверяем, что current_brick выделен
  EXPECT_NE(t->next_brick, nullptr); // проверяем, что next_brick выделен
  EXPECT_EQ(t->gameinfo.level, 0); // ожидаем, что уровень 0 после stats_init
  EXPECT_EQ(t->gameinfo.pause, 0); // ожидаем, что пауза снята

  int res = t->init_score(t); // инициализируем счёт/файл рекорда
  EXPECT_EQ(res, 0); // ожидаем успех инициализации

#ifdef DATA_FILE_NAME
  FILE *f = fopen(DATA_FILE_NAME, "rb"); // открываем файл рекорда для проверки
  ASSERT_NE(f, nullptr); // убеждаемся, что файл действительно создан
  int val = -1; // переменная для чтения рекорда
  fread(&val, sizeof(int), 1, f); // читаем значение из файла
  fclose(f); // закрываем файл
  EXPECT_GE(val, 0); // проверяем, что записанное значение неотрицательное
  remove(DATA_FILE_NAME); // удаляем файл рекорда после проверки
#endif

  t->gameinfo.level = 2; // симулируем уровень > 0 чтобы game_over удалил память
  if (!t->current_brick) t->current_brick = (int*)malloc(sizeof(int) * BRICK_SIZE); // обеспечиваем наличие буфера
  if (!t->next_brick) t->next_brick = (int*)malloc(sizeof(int) * BRICK_SIZE); // обеспечиваем наличие буфера

  t->game_over(); // вызываем логику завершения игры
  EXPECT_EQ(t->gameinfo.level, -1); // ожидаем, что уровень указывает на проигрыш/завершение

  t->current_brick = nullptr; // очищаем указатели чтобы не мешать следующим тестам
  t->next_brick = nullptr;
}

TEST(tetris_more, init_free_and_fill_matrix) { // тест выделения, заполнения и освобождения матрицы
  Tetris *t = Tetris::get_instance(); // получаем синглтон

  int **m = t->init_matrix(nullptr, 3, 4); // выделяем матрицу 3x4
  ASSERT_NE(m, nullptr); // проверяем, что матрица выделена
  t->fill_array_zero(m, 3, 4); // заполняем матрицу нулями
  for (int i = 0; i < 3; ++i) // проверяем, что все ячейки равны 0
    for (int j = 0; j < 4; ++j)
      EXPECT_EQ(m[i][j], 0);
  t->free_memory_matrix(m, 3); // освобождаем память матрицы
}

TEST(tetris_more, spawn_brick_despawn_and_move_checks) { // тест размещения, удаления фигуры и проверок перемещения
  Tetris *t = Tetris::get_instance(); // получаем синглтон

  int **field = t->init_matrix(nullptr, WINDOW_HEIGHT, WINDOW_WIDTH); // создаём поле игры
  zero_matrix(field, WINDOW_HEIGHT, WINDOW_WIDTH); // обнуляем поле

  int brick[BRICK_SIZE] = {0,0, 0,1, 1,0, 1,1}; // координаты квадрата
  int color = 7; // тестовый цвет

  t->spawn_brick(field, brick, color); // рисуем кирпич на поле
  EXPECT_EQ(field[0][0], color); // проверяем каждую ячейку квадрата
  EXPECT_EQ(field[0][1], color);
  EXPECT_EQ(field[1][0], color);
  EXPECT_EQ(field[1][1], color);

  t->despawn(field, brick); // убираем кирпич с поля
  EXPECT_EQ(field[0][0], 0); // проверяем обнуление
  EXPECT_EQ(field[0][1], 0);
  EXPECT_EQ(field[1][0], 0);
  EXPECT_EQ(field[1][1], 0);

  int b_right[BRICK_SIZE] = {0, 0, 0, 1, 0, 2, 0, 3}; // горизонтальная линия для проверки границ
  EXPECT_EQ(t->check_right(field, b_right, 1), 1); // вправо можно сдвинуть
  EXPECT_EQ(t->check_right(field, b_right, WINDOW_WIDTH), 0); // но не на такое большое смещение

  EXPECT_EQ(t->check_left(field, b_right, 1), 0); // проверка влево: выйдет за пределы
  EXPECT_EQ(t->check_left(field, b_right, 0), 1); // без сдвига влево допустимо

  int b_down[BRICK_SIZE] = {WINDOW_HEIGHT - 1, 0, WINDOW_HEIGHT - 2, 1, WINDOW_HEIGHT - 3, 2, WINDOW_HEIGHT - 4, 3}; // часть фигуры у дна
  EXPECT_EQ(t->check_down(field, b_down), 0); // ожидание, что опускание невозможно из-за дна

  zero_matrix(field, WINDOW_HEIGHT, WINDOW_WIDTH); // обнуляем поле
  field[2][2] = 5; // устанавливаем препятствие в поле
  int b2[BRICK_SIZE] = {1,2, 0,0, 0,0, 0,0}; // блок, который должен столкнуться с препятствием при опускании
  EXPECT_EQ(t->check_down(field, b2), 0); // проверяем столкновение с уже занятым полем

  t->free_memory_matrix(field, WINDOW_HEIGHT); // освобождаем выделенное поле
}

// Дополнительные тесты: brick_move, rotate_check_field, fix_brick_coord, check_level, brick_copy, check_gameover, score_write


TEST(tetris_more2, brick_move_left_right_and_action_rotate) { // тест перемещений и поворотов фигур через brick_move
  Tetris *t = Tetris::get_instance(); // получаем синглтон

  int **field = t->init_matrix(nullptr, WINDOW_HEIGHT, WINDOW_WIDTH); // создаём поле
  zero_matrix(field, WINDOW_HEIGHT, WINDOW_WIDTH); // обнуляем поле

  int brick[BRICK_SIZE] = {5, 5, 5, 6, 6, 5, 6, 6}; // координаты квадрата
  int orig[BRICK_SIZE]; // буфер для сохранения оригинальных координат
  memcpy(orig, brick, sizeof(orig)); // копируем исходный массив

  t->brick_move(brick, Left, field); // выполняем движение влево
  EXPECT_EQ(brick[1], orig[1] - 1); // проверяем эффект сдвига X для первых блоков
  EXPECT_EQ(brick[3], orig[3] - 1);

  memcpy(brick, orig, sizeof(orig)); // восстанавливаем исходные координаты
  t->brick_move(brick, Right, field); // выполняем движение вправо
  EXPECT_EQ(brick[1], orig[1] + 1); // проверяем сдвиг вправо
  EXPECT_EQ(brick[3], orig[3] + 1);

  int hero[BRICK_SIZE] = {2,2, 2,3, 2,4, 2,5}; // линейная фигура Hero
  memcpy(brick, hero, sizeof(hero)); // копируем Hero в рабочий массив
  t->brick_move(brick, Action, field); // вызываем поворот через Action
  for (int i = 0; i < BRICK_SIZE; ++i) EXPECT_TRUE(brick[i] < 1000); // проверяем, что координаты остаются в разумных пределах

  t->free_memory_matrix(field, WINDOW_HEIGHT); // освобождаем поле
}

TEST(tetris_more2, rotate_check_field_and_fix_brick_coord) { // тест проверки пересечения с полем и приведения координат внутрь поля
  Tetris *t = Tetris::get_instance(); // получаем синглтон

  int **field = t->init_matrix(nullptr, WINDOW_HEIGHT, WINDOW_WIDTH); // выделяем поле
  zero_matrix(field, WINDOW_HEIGHT, WINDOW_WIDTH); // обнуляем поле

  int brick[BRICK_SIZE] = {0,0, 0,1, 1,0, 1,1}; // квадрат
  field[0][0] = 9; // ставим препятствие в поле поверх одной из клеток квадрата
  EXPECT_EQ(t->rotate_check_field(field, brick), 0); // проверяем, что пересечение обнаружено
  field[0][0] = 0; // убираем препятствие
  EXPECT_EQ(t->rotate_check_field(field, brick), 1); // проверяем, что пересечения нет

  int b_right[BRICK_SIZE] = {2, WINDOW_WIDTH, 3, WINDOW_WIDTH+1, 4, WINDOW_WIDTH+2, 5, WINDOW_WIDTH+3}; // координаты, выходящие за правую границу
  t->fix_brick_coord(b_right); // корректируем координаты внутрь поля
  for (int i = 1; i < BRICK_SIZE; i += 2) EXPECT_LT(b_right[i], WINDOW_WIDTH); // проверяем, что X < WINDOW_WIDTH

  int b_left[BRICK_SIZE] = {2, -3, 3, -2, 4, -1, 5, 0}; // координаты, выходящие за левую границу
  t->fix_brick_coord(b_left); // корректируем координаты внутрь поля
  for (int i = 1; i < BRICK_SIZE; i += 2) EXPECT_GE(b_left[i], 0); // проверяем, что X >= 0

  int b_down[BRICK_SIZE] = { WINDOW_HEIGHT+2, 1, WINDOW_HEIGHT+3, 2, WINDOW_HEIGHT+1, 3, WINDOW_HEIGHT+4, 4 }; // координаты за нижней границей
  t->fix_brick_coord(b_down); // корректируем по Y
  for (int i = 0; i < BRICK_SIZE; i += 2) EXPECT_LT(b_down[i], WINDOW_HEIGHT); // проверяем, что Y < WINDOW_HEIGHT

  int b_up[BRICK_SIZE] = { -5, 1, -4, 2, -3, 3, -2, 4 }; // координаты за верхней границей
  t->fix_brick_coord(b_up); // корректируем по Y
  for (int i = 0; i < BRICK_SIZE; i += 2) EXPECT_GE(b_up[i], 0); // проверяем, что Y >= 0

  t->free_memory_matrix(field, WINDOW_HEIGHT); // освобождаем поле
}

TEST(tetris_more2, check_level_and_brick_copy_and_check_gameover) { // тест проверки повышения уровня, копирования кирпича и условия Game Over
  Tetris *t = Tetris::get_instance(); // получаем синглтон Tetris

  t->gameinfo.level = 0; // устанавливаем начальный уровень 0
  t->gameinfo.speed = 0; // начальная скорость 0
  t->gameinfo.score = 600; // задаём счёт ровно 600 чтобы пройти проверку для уровня 1
  t->check_level(t); // вызываем проверку уровня
  EXPECT_EQ(t->gameinfo.level, 1); // ожидаем, что уровень увеличился до 1
  EXPECT_EQ(t->gameinfo.speed, 1); // ожидаем, что скорость также увеличилась

  t->gameinfo.level = 10; // ставим уровень в максимальный порог (10)
  t->gameinfo.speed = 5; // задаём некоторую скорость
  t->gameinfo.score = 100000; // большой счёт, не должен увеличить уровень выше 10
  t->check_level(t); // проверяем снова
  EXPECT_EQ(t->gameinfo.level, 10); // уровень остаётся 10 (ограничение)

  int src[BRICK_SIZE] = {1,2,3,4,5,6,7,8}; // исходный массив координат кирпича
  int dst[BRICK_SIZE] = {0}; // целевой массив, который будет заполнен копированием
  t->brick_copy(dst, src); // копируем данные src в dst
  for (int i = 0; i < BRICK_SIZE; ++i) EXPECT_EQ(dst[i], src[i]); // проверяем поэлементное совпадение

  int g1[BRICK_SIZE] = {0,1, 2,3, 4,5, 6,7}; // массив, где присутствует Y==0 (верхняя граница) — означает Game Over
  EXPECT_EQ(t->check_gameover(g1), 1); // ожидаем, что check_gameover вернёт 1 (игра окончена)
  int g2[BRICK_SIZE] = {1,1, 2,3, 4,5, 6,7}; // массив без Y==0
  EXPECT_EQ(t->check_gameover(g2), 0); // ожидаем, что check_gameover вернёт 0 (игра не окончена)
}

TEST(tetris_new_brick, generates_known_bricks_for_1_to_7_and_default_noop) { // тест генерации новых фигур по коду
  Tetris *t = Tetris::get_instance(); // получаем синглтон

  // Проверяем случаи 1..7: после вызова temp не должен остаться sentinel для хотя бы одного элемента
  for (int r = 1; r <= 7; ++r) { // пробег по всем допустимым значениям типа кирпича
    int temp[BRICK_SIZE];
    for (int i = 0; i < BRICK_SIZE; ++i) temp[i] = -99; // заполняем sentinel-значением -99
    t->new_brick(temp, r); // генерируем кирпич для конкретного r

    bool changed = false;
    for (int i = 0; i < BRICK_SIZE; ++i) {
      if (temp[i] != -99) { changed = true; break; } // если хотя бы один элемент изменился — генерация прошла
    }
    EXPECT_TRUE(changed) << "new_brick did not change temp for random=" << r; // проверяем, что генерация произошла
  }

  // Проверяем default: передаём значение вне диапазона, temp должен остаться нетронутым
  int temp_def[BRICK_SIZE];
  for (int i = 0; i < BRICK_SIZE; ++i) temp_def[i] = -42; // sentinel -42 для default теста
  t->new_brick(temp_def, 0); // вызываем new_brick с недопустимым кодом (0)
  for (int i = 0; i < BRICK_SIZE; ++i) {
    EXPECT_EQ(temp_def[i], -42) << "default branch modified temp_def at index " << i; // ожидаем, что ничего не изменилось
  }

  // Ещё один out-of-range тест
  int temp_def2[BRICK_SIZE];
  for (int i = 0; i < BRICK_SIZE; ++i) temp_def2[i] = -7; // sentinel -7
  t->new_brick(temp_def2, 999); // вызываем new_brick с явно недопустимым кодом (999)
  for (int i = 0; i < BRICK_SIZE; ++i) {
    EXPECT_EQ(temp_def2[i], -7) << "default branch modified temp_def2 at index " << i; // ожидаем, что массив остался неизменён
  }
}

TEST(tetris_check_full_row, single_full_row_removes_and_shifts_down) { // тест удаления одной полностью заполненной строки
  Tetris *t = Tetris::get_instance(); // получаем синглтон

  // Подготовка поля
  int **field = t->init_matrix(nullptr, WINDOW_HEIGHT, WINDOW_WIDTH); // выделяем матрицу поля
  t->fill_array_zero(field, WINDOW_HEIGHT, WINDOW_WIDTH); // обнуляем поле

  // Заполним предпоследнюю строку (индекс WINDOW_HEIGHT - 1) полностью единицами
  int target = WINDOW_HEIGHT - 1; // индекс последней строки
  for (int j = 0; j < WINDOW_WIDTH; ++j) field[target][j] = 5; // заполняем строку значением 5

  // Поставим уникальные значения в несколько строк выше, чтобы отследить сдвиг
  field[target - 1][0] = 7; // метка в строке выше
  field[target - 2][0] = 8; // метка ещё выше

  int removed = t->check_full_row(field); // вызываем проверку и удаление полных строк
  EXPECT_EQ(removed, 1); // ожидаем удаление ровно одной строки

  // После удаления верхние строки должны сдвинуться вниз:
  // то, что было в target-1 теперь должно оказаться в target
  EXPECT_EQ(field[target][0], 7); // проверяем, что метка сдвинулась на 1 вниз
  // то, что было в target-2 теперь на target-1
  EXPECT_EQ(field[target - 1][0], 8); // проверяем следующую метку

  t->free_memory_matrix(field, WINDOW_HEIGHT); // освобождаем поле
}

TEST(tetris_check_full_row, multiple_adjacent_full_rows_removed) { // тест удаления нескольких смежных заполненных строк
  Tetris *t = Tetris::get_instance();

  int **field = t->init_matrix(nullptr, WINDOW_HEIGHT, WINDOW_WIDTH); // создаём поле
  t->fill_array_zero(field, WINDOW_HEIGHT, WINDOW_WIDTH); // обнуляем

  // Сделаем две нижние строки полностью заполненными
  int last = WINDOW_HEIGHT - 1;
  for (int j = 0; j < WINDOW_WIDTH; ++j) {
    field[last][j] = 1; // заполняем последнюю строку
    field[last - 1][j] = 2; // и предпоследнюю строку
  }

  // Заполним строку выше уникальным маркером
  field[last - 2][0] = 9; // метка для отслеживания сдвига

  int removed = t->check_full_row(field); // удаляем заполненные строки
  EXPECT_EQ(removed, 2); // ожидаем, что удалилось две строки

  // После удаления маркер должен сдвинуться вниз на 2 позиции
  EXPECT_EQ(field[last][0], 9); // проверяем, что маркер оказался в нижней строке

  t->free_memory_matrix(field, WINDOW_HEIGHT); // освобождаем поле
}

TEST(tetris_check_full_row, no_full_rows_returns_zero_and_unchanged) { // тест случая, когда полных строк нет
  Tetris *t = Tetris::get_instance();

  int **field = t->init_matrix(nullptr, WINDOW_HEIGHT, WINDOW_WIDTH); // выделяем поле
  t->fill_array_zero(field, WINDOW_HEIGHT, WINDOW_WIDTH); // обнуляем

  // Частично заполненная нижняя строка (одна ячейка ноль не заполнит ряд)
  int last = WINDOW_HEIGHT - 1;
  for (int j = 0; j < WINDOW_WIDTH - 1; ++j) field[last][j] = 1; // заполняем почти всю строку
  field[last][WINDOW_WIDTH - 1] = 0; // оставляем один ноль, поэтому ряд не полный

  int before = field[last][0]; // сохраняем значение для проверки неизменности
  int removed = t->check_full_row(field); // вызываем проверку
  EXPECT_EQ(removed, 0); // ожидаем, что удалений не произошло

  // Поле не должно было сдвинуться: значение осталось на той же позиции
  EXPECT_EQ(field[last][0], before); // проверяем, что значение не сместилось

  t->free_memory_matrix(field, WINDOW_HEIGHT); // освобождаем поле
}

// tests for Tetris::check_attaching
TEST(tetris_check_attaching, moves_down_when_space_and_returns_zero) { // тест проверки возможности опускания фигуры
  Tetris *t = Tetris::get_instance();

  int **field = t->init_matrix(nullptr, WINDOW_HEIGHT, WINDOW_WIDTH); // создаём пустое поле
  t->fill_array_zero(field, WINDOW_HEIGHT, WINDOW_WIDTH);

  // brick well above bottom and empty below -> check_down should be true
  int brick[BRICK_SIZE] = {2,2, 2,3, 3,2, 3,3}; // квадратик, расположенный вверху поля
  int before_y0 = brick[0]; // сохраняем исходную Y координату первого блока

  int res = t->check_attaching(brick, field); // пытаемся опустить фигуру вниз (проверка + возможный сдвиг)
  // when can move down, function returns 0 and Y coords shifted by +1 (DOWN)
  EXPECT_EQ(res, 0); // ожидаем, что функция вернёт 0 (не нужно прикреплять)
  EXPECT_EQ(brick[0], before_y0 + 1); // проверяем, что Y координата первого блока увеличилась на 1

  t->free_memory_matrix(field, WINDOW_HEIGHT); // освобождаем поле
}

TEST(tetris_check_attaching, returns_one_when_blocked_and_does_not_shift) { // тест поведения при блокировке снизу
  Tetris *t = Tetris::get_instance();

  int **field = t->init_matrix(nullptr, WINDOW_HEIGHT, WINDOW_WIDTH); // создаём поле
  t->fill_array_zero(field, WINDOW_HEIGHT, WINDOW_WIDTH);

  // place occupancy directly below one of the brick's cells to block downward movement
  int brick[BRICK_SIZE] = {WINDOW_HEIGHT - 2, 4, WINDOW_HEIGHT - 3, 5, WINDOW_HEIGHT - 4, 6, WINDOW_HEIGHT - 5, 7};
  // occupy the cell immediately below the first brick cell
  field[brick[0] + 1][brick[1]] = 9; // ставим препятствие прямо под первым блоком кирпича

  int before_y0 = brick[0]; // сохраняем исходную Y координату первого блока
  int res = t->check_attaching(brick, field); // вызываем проверку, которая не должна сдвинуть фигуру
  // when blocked, function returns 1 and brick Y coords remain unchanged
  EXPECT_EQ(res, 1); // ожидаем, что вернётся 1 (фигура должна прикрепиться)
  EXPECT_EQ(brick[0], before_y0); // Y координата не изменилась

  t->free_memory_matrix(field, WINDOW_HEIGHT); // освобождаем поле
}

