#include "tests.h" // подключает заголовок с прототипами тестовых утилит и константами

void find_obj_coords(int** field, int& coord_y, int& coord_x,
                     const int& obj_type) { // ищет координаты объекта obj_type в поле и записывает их в coord_y и coord_x
  for (auto i = 0; i < WINDOW_HEIGHT; i++) { // итерация по всем строкам игрового поля
    for (auto j = 0; j < WINDOW_WIDTH; j++) { // итерация по всем столбцам игровой строки
      if (field[i][j] == obj_type) { // если найден объект заданного типа в ячейке
        coord_y = i; // сохраняем Y координату найденного объекта
        coord_x = j; // сохраняем X координату найденного объекта
      } // конец проверки типа ячейки
    } // конец цикла по столбцам
  } // конец цикла по строкам
} // конец функции find_obj_coords

void find_field_half(const int& apple_y, const int& apple_x, int& y_half,
                     int& x_half) { // определяет в каких "половинах" поля находится яблоко по осям и записывает в x_half и y_half
  if (apple_x == (WINDOW_WIDTH / 2 - 1)) { // если X совпадает с центральной колонкой
    x_half = 0; // помечаем как центральная колонка
  } else if (apple_x < (WINDOW_WIDTH / 2 - 1)) { // если X находится левее центра
    x_half = 1; // помечаем как левая половина
  } else if (apple_x > (WINDOW_WIDTH / 2 - 1)) { // если X находится правее центра
    x_half = 2; // помечаем как правая половина
  } // конец определения x_half
  if (apple_y == (WINDOW_HEIGHT / 2 - 2)) { // если Y совпадает с центральной строкой (смещённой)
    y_half = 0; // помечаем как центральная строка
  } else if (apple_y < (WINDOW_HEIGHT / 2 - 2)) { // если Y находится выше центра
    y_half = 1; // помечаем как верхняя половина
  } else if (apple_y > (WINDOW_HEIGHT / 2 - 2)) { // если Y находится ниже центра
    y_half = 2; // помечаем как нижняя половина
  } // конец определения y_half
} // конец функции find_field_half

void horizontal_shift(s21::Game* snake_game, const int& x_half,
                      const int& apple_x, int& head_x) { // сдвигает голову змейки горизонтально к яблоку, используя шаги КА
  if (x_half == 1) { // если яблоко слева от центра
    snake_game->set_user_action(UserAction_t::Left); // отправляем действие Left в игру
    snake_game->fsm();  // выполняем шаг КА — Moving
    snake_game->fsm();  // выполняем шаг КА — Shifting
    head_x--; // обновляем локальное положение головы влево
    while (head_x != apple_x) { // пока голова не достигнет X яблока
      snake_game->set_user_action(UserAction_t::Action); // продолжаем "двигаться" (Action как ход)
      snake_game->fsm();  // Moving
      snake_game->fsm();  // Shifting
      head_x--; // сдвигаем локальную координату головы влево на единицу
    } // конец цикла движения влево
  } else if (x_half == 2) { // если яблоко справа от центра
    snake_game->set_user_action(UserAction_t::Right); // отправляем действие Right в игру
    snake_game->fsm();  // Moving
    snake_game->fsm();  // Shifting
    head_x++; // обновляем локальную позицию головы вправо
    while (head_x != apple_x) { // пока голова не достигнет X яблока
      snake_game->set_user_action(UserAction_t::Action); // продолжаем шаги движения вправо
      snake_game->fsm();  // Moving
      snake_game->fsm();  // Shifting
      head_x++; // сдвигаем локальную координату головы вправо на единицу
    } // конец цикла движения вправо
  } // конец условной логики horizontal_shift
} // конец функции horizontal_shift

void vertical_shift(s21::Game* snake_game, const int& y_half, const int& x_half,
                    const int& apple_y, int& head_y) { // сдвигает голову змейки вертикально к яблоку, учитывая положение по X для обходов
  if (y_half == 1) { // если яблоко выше центра
    if (x_half != 0) { // если яблоко не строго по центральной колонке
      snake_game->set_user_action(UserAction_t::Up); // отправляем действие Up в игру
      snake_game->fsm();  // Moving
      snake_game->fsm();  // Shifting
      head_y--; // обновляем локальную координату головы вверх
    } // конец проверки центрального столбца
    while (head_y != apple_y) { // пока голова не достигнет Y яблока
      snake_game->set_user_action(UserAction_t::Action); // продолжаем шаги движения вверх
      snake_game->fsm();  // Moving
      snake_game->fsm();  // Shifting
      head_y--; // уменьшаем локальную координату головы на единицу
    } // конец цикла движения вверх
  } else if (y_half == 2) { // если яблоко ниже центра
    if (x_half == 0) { // если яблоко строго по центральной колонке, выполняем обход для корректного поворота
      snake_game->set_user_action(UserAction_t::Right); // сначала сдвигаем вправо
      snake_game->fsm();  // Moving
      snake_game->fsm();  // Shifting
      snake_game->set_user_action(UserAction_t::Down); // затем опускаем вниз
      snake_game->fsm();  // Moving
      snake_game->fsm();  // Shifting
      head_y++; // обновляем локальную координату головы вниз на 1
      snake_game->set_user_action(UserAction_t::Left); // затем возвращаемся влево
      snake_game->fsm();  // Moving
      snake_game->fsm();  // Shifting
    } // конец обхода при центральной колонке
    snake_game->set_user_action(UserAction_t::Down); // основной шаг вниз
    snake_game->fsm();  // Moving
    snake_game->fsm();  // Shifting
    head_y++; // увеличиваем локальную координату головы вниз
    while (head_y != apple_y) { // пока голова не достигнет Y яблока
      snake_game->set_user_action(UserAction_t::Action); // повторяем шаги опускания
      snake_game->fsm();  // Moving
      snake_game->fsm();  // Shifting
      head_y++; // увеличиваем локальную координату головы на единицу
    } // конец цикла движения вниз
  } // конец условной логики vertical_shift
} // конец функции vertical_shift
