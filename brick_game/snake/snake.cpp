#include "snake.h"  // Подключение заголовочного файла с описанием класса Snake и зависимостями

// Определение координат центра игрового поля по вертикали и горизонтали
#define MID_FIELD_Y ((WINDOW_HEIGHT / 2) - 1)
#define MID_FIELD_X ((WINDOW_WIDTH / 2) - 1)

// Определение начальных координат для элементов поля
#define START_Y 0
#define START_X 0

// Определение флагов состояния паузы
#define PAUSE 1
#define UNPAUSE 0

// Максимальный и стартовый размер змейки, а также индекс головы
#define SNAKE_MAX_SIZE 200
#define SNAKE_START_SIZE 4
#define SNAKE_HEAD 0

// Определение значений для отображения элементов на поле
#define DESPAWN 0                 // Очистка ячейки
#define SPAWN 1                   // Отрисовка тела змейки
#define SPAWN_COLOR_APPLE 2       // Цвет яблока
#define SPAWN_COLOR_SNAKE 3       // Цвет тела змейки
#define SPAWN_COLOR_SNAKE_HEAD 4  // Цвет головы змейки

// Настройки таймера (задержка и скорость)
#define TIMER_MAX_DELAY 1000
#define TIMER_MIN_DELAY 200
#define TIMER_MAX_SPEED 10

// Настройки уровней
#define MAX_LEVEL 10
#define POINTS_FOR_NEXT_LEVEL 5

// Имя бинарного файла, где хранится рекорд игрока
#define DATA_FILE_NAME "snake_data.bin"

// Определение пространства имён s21
namespace s21 {

/**
 * @brief Конструктор.
 *
 * Выделяет память для хранения координат змейки.
 */
Snake::Snake() {                               // Определение конструктора класса Snake
  snake_coords = new std::pair<int, int>[SNAKE_MAX_SIZE];  // Динамическое выделение массива координат
}

/**
 * @brief Деструктор.
 *
 * Очищает память массива с координатами змейки.
 */
Snake::~Snake() {              // Определение деструктора класса Snake
  delete[] snake_coords;       // Освобождение динамически выделенной памяти
}


/**
 * @brief Возвращает случайный индекс из диапазона доступных клеток.
 *
 * Использует стандартную библиотеку random для равномерного распределения.
 *
 * @param free_cells_size Количество доступных клеток.
 * @return Случайный индекс в диапазоне [0, free_cells_size - 1].
 */
int Snake::get_random_index(const size_t free_cells_size) const {
  std::random_device rd;              // Источник энтропии для инициализации генератора
  std::mt19937 gen(rd());             // Генератор случайных чисел Mersenne Twister
  std::uniform_int_distribution<> distrib(0, free_cells_size - 1);  // Равномерное распределение
  return distrib(gen);                // Возврат случайного индекса
}


/**
 * @brief Инициализация поля high_score.
 *
 * Проверяет наличие бинарного файла с рекордом. Если файл отсутствует — создаёт его.
 * В противном случае — считывает значение рекорда.
 * Все ошибки перехватываются в блоке try-catch.
 */
void Snake::init_record() {
  try {
    if (!read_record_file()) write_record_file(0);  // Если файла нет — создаём с 0
  } catch (const std::exception& e) {
    std::cerr << "File error: " << e.what() << DATA_FILE_NAME << '\n';
  }
}


/**
 * @brief GameStart — начальное состояние конечного автомата.
 *
 * Запускается один раз в начале игры. Инициализирует статистику, рекорд и уровень.
 * После запуска переводит FSM в состояние Spawn или завершает игру.
 */
void Snake::starting_game() {
  if (action == Start) {              // Игрок нажал "старт"
    init_statistic();                 // Сброс параметров игры
    init_record();                    // Проверка / создание файла рекорда
    gameinfo.level = 1;               // Устанавливаем уровень 1
    statemachine = Spawn;             // Переход в следующее состояние
  } else if (action == Terminate) {   // Если игрок завершает игру
    statemachine = GameOver;
  }
}


/**
 * @brief Проверяет, входят ли координаты в игровое поле.
 *
 * @param coords Проверяемая пара координат (y, x).
 * @return true, если координаты находятся в пределах поля.
 */
bool Snake::coord_valid_check(const std::pair<int, int> coords) const {
  bool res = true;
  if (coords.first >= WINDOW_HEIGHT || coords.second >= WINDOW_WIDTH) {
    res = false;                      // Выход за нижнюю или правую границу
  } else if (coords.first < START_Y || coords.second < START_X) {
    res = false;                      // Выход за верхнюю или левую границу
  }
  return res;
}


/**
 * @brief Spawn — состояние появления яблока.
 *
 * Генерирует новое яблоко на свободной клетке.
 * После спавна переводит FSM в состояние Moving.
 */
void Snake::spawn() {
  spawn_apple();                      // Появление яблока
  snake_position_update(SPAWN);       // Отрисовка змейки
  statemachine = Moving;              // Переход к движению
}


/**
 * @brief Moving (состояние конечного автомата).
 *
 * Всегда перезаписывает действие игрока (action) на Start (используется только в GameStart).
 * Пауза не является состоянием конечного автомата (КА), а служит лишь условием для перехода к следующему состоянию.
 * Может перевести КА в состояние Shifting, формируя основной игровой цикл Moving <-> Shifting.
 * Может переключить КА в состояние GameOver, если выполнено соответствующее действие Terminate.
 * Переход в состояние Shifting инициируется по трём «флагам»: поворот, истечение таймера или действие action. В любом случае при этом происходит обнуление таймера и установка нового направления (или сохранение текущего).
 */
void Snake::moving() {
  if (action == Pause) {              // Игрок нажал паузу
    pause_game();
  } else if (action == Terminate) {   // Игрок завершает игру
    statemachine = GameOver;
  }

  if (gameinfo.pause != PAUSE) {      // Если не на паузе
    if (check_rotate_head() ||        // Проверка поворота
        timer.game_timer_check(gameinfo.speed, TIMER_MAX_DELAY, TIMER_MIN_DELAY,
                               TIMER_MAX_SPEED) ||
        action == Action) {           // Или таймер / действие игрока
      timer.start();                  // Обновляем таймер
      set_direction();                // Устанавливаем новое направление
      statemachine = Shifting;        // Переход к движению
    }
  }

  action = Start;                     // Сброс действия
}

/**
 * @brief Shifting (состояние конечного автомата).
 *
 * Двигает змейку и проверяет коллизию.
 * Может переключить КА на состояние Moving, создавая основной игровой цикл
 * Moving <-> Shifting. В случае когда какой-то игровой элемент сталкивается с другим -
 * перключает конечный автомат в состояние Attaching.
 */
void Snake::shifting() {
  snake_position_update(DESPAWN);     // Очистка старого положения
  shift_to_head();                    // Сдвиг тела змейки
  rotate_head();                      // Движение головы

  if (coord_valid_check(snake_coords[SNAKE_HEAD]) && 
      !check_collide_apple_of_snake() &&
      !check_collide_body_head()) {
    snake_position_update(SPAWN);     // Перерисовка змейки
    statemachine = Moving;            // Возврат в цикл
  } else {
    statemachine = Attaching;         // Коллизия → обработка
  }
}

/**
 * @brief Attaching (состояние конечного автомата).
 *
 * Обрабатывает все коллизии.
 * Разделяет коллизию на отдельные случаи
 * Если была задета стенка или сама змейка - перекалючает конечный автомат на GameOver.
 * Если было задето яблоко и змейка максимально размера - перекалючает конечный автомат на GameOver. 
 * Если было задето яблоко и змейка НЕ максимально размера - перекалючает конечный автомат на Spawn.
 */
void Snake::attaching() {
  if (!coord_valid_check(snake_coords[SNAKE_HEAD]) || check_collide_body_head()) {
    statemachine = GameOver;          // Столкновение со стеной / собой
  } else if (check_collide_apple_of_snake()) {
    snake_size++;                     // Увеличиваем размер змейки
    if (snake_size >= SNAKE_MAX_SIZE) {
      statemachine = GameOver;        // Победа при максимальном размере
    } else {
      snake_coords[snake_size - 1] = snake_coords[snake_size - 2];
      snake_position_update(SPAWN);   // Отрисовка новой клетки
      update_score_game();            // Обновление очков
      update_level_speed();           // Повышение уровня и скорости
      statemachine = Spawn;           // Появление нового яблока
    }
  }
}

/**
 * @brief Инициализация полей класса Snake.
 *
 * Записывает стандартные значения в поля класса для корректного начала игры.
 */
void Snake::init_statistic() { // начало определения метода инициализации статистики
  apple_coords = {START_Y, START_X}; // устанавливает начальные координаты яблока в константные START_Y и START_X
  snake_size = SNAKE_START_SIZE; // задаёт начальный размер змейки константой SNAKE_START_SIZE
  curr_direction = Direction::Dir_Up; // устанавливает текущее направление змейки вверх
  for (int i = SNAKE_HEAD; i < SNAKE_START_SIZE; i++) { // цикл для инициализации координат каждой клетки змейки от головы до начального размера
    snake_coords[i].first = MID_FIELD_Y + i; // задаёт координату Y для сегмента змейки со смещением от середины поля
    snake_coords[i].second = MID_FIELD_X; // задаёт координату X для сегмента змейки по середине поля
  } // конец цикла инициализации координат змейки
} // конец метода init_statistic

/**
 * @brief Запись вектора свободных координат.
 *
 * Помогает определить координаты для появления яблока.
 * Берёт все доступные координаты и исключает из них координаты занятые змейкой.
 *
 * @param free_cells Вектор свободных координат.
 */
void Snake::get_free_cells(std::vector<std::pair<int, int>>& free_cells) const { // начало метода заполнения вектора свободных клеток
  for (int y = START_Y; y < WINDOW_HEIGHT; y++) { // итерирует по всем строкам игрового окна от START_Y до высоты окна
    for (int x = START_X; x < WINDOW_WIDTH; x++) { // итерирует по всем столбцам игрового окна от START_X до ширины окна
      free_cells.push_back({y, x}); // добавляет текущую координату (y,x) в список свободных клеток
    } // конец цикла по столбцам
  } // конец цикла по строкам
  for (int i = SNAKE_HEAD; i < snake_size; i++) { // проходит по всем сегментам змейки от головы до её текущего размера
    bool cont = true; // флаг продолжения поиска для текущего сегмента змейки
    for (size_t j = 0; (j < free_cells.size()) && cont; j++) { // ищет координату сегмента в списке свободных клеток, пока не найдёт или не закончится список
      if (free_cells[j] == snake_coords[i]) { // если свободная клетка совпадает с координатой сегмента змейки
        free_cells.erase(free_cells.begin() + j); // удаляет эту клетку из списка свободных клеток
        cont = false; // ставит флаг в false чтобы прекратить внутренний цикл для этого сегмента
      } // конец проверки совпадения координат
    } // конец внутреннего цикла поиска и удаления
  } // конец внешнего цикла по сегментам змейки
} // конец метода get_free_cells

/**
 * @brief Отвечает за обновление скорости и уровеня.
 *
 * Увеличивает уровень и скорость за каждые POINTS_FOR_NEXT_LEVEL очков.
 */
void Snake::update_level_speed() { // начало метода обновления уровня и скорости
  if (gameinfo.level < MAX_LEVEL) { // проверяет, что текущий уровень меньше максимального
    if (gameinfo.score % POINTS_FOR_NEXT_LEVEL == 0) { // если счёт кратен порогу для следующего уровня
      gameinfo.speed++; // увеличивает скорость игры
      gameinfo.level++; // повышает уровень на единицу
    } // конец проверки кратности очков порогу
  } // конец проверки максимального уровня
} // конец метода update_level_speed

/**
 * @brief Считывает рекорд из файла и записывает в структуру.
 *
 * \throw std::runtime_error - если ошибка.
 * @return Статус открытия файла.
 */
bool Snake::read_record_file() { // начало метода чтения рекорда из файла
  bool res = false; // результат успешности чтения файла по умолчанию false
  int number = 0; // временная переменная для хранения прочитанного числа (рекорда)
  std::ifstream input_file(DATA_FILE_NAME, std::ios::binary); // открывает входной бинарный файл с именем DATA_FILE_NAME
  if (input_file.is_open()) { // проверяет, открыт ли файл успешно
    input_file.read(reinterpret_cast<char*>(&number), sizeof(number)); // читает целое число из файла в переменную number
    gameinfo.high_score = number; // записывает прочитанное число в поле high_score структуры gameinfo
    input_file.close(); // закрывает файл после чтения
    if (input_file.fail()) { // проверяет флаг ошибки потока после закрытия
      throw std::runtime_error("Error reading from file"); // выбрасывает исключение в случае ошибки чтения
    } // конец проверки ошибки потока
    res = true; // отмечает успешное чтение
  } // конец проверки открытия файла
  return res; // возвращает статус операции
} // конец метода read_record_file

/**
 * @brief Создает и записывает в файл число.
 *
 * \throw std::runtime_error Если ошибка создания файла.
 */
void Snake::write_record_file(const int number) { // начало метода записи рекорда в файл с передачей числа number
  std::ofstream output_file(DATA_FILE_NAME, std::ios::binary); // открывает выходной бинарный файл с именем DATA_FILE_NAME
  if (output_file.is_open()) { // проверяет, открыт ли файл для записи
    output_file.write(reinterpret_cast<const char*>(&number), sizeof(number)); // записывает число number в файл в бинарном виде
    output_file.close(); // закрывает файл после записи
  } else { // если файл не открылся
    throw std::runtime_error("Error creating file"); // выбрасывает исключение о невозможности создания файла
  } // конец проверки открытия файла
} // конец метода write_record_file

/**
 * @brief Проверяет соприкосновения яблока и головы змейки.
 *
 * @return Результат проверки.
 */
bool Snake::check_collide_apple_of_snake() const { // начало метода проверки столкновения головы змейки с яблоком
  bool res = false; // по умолчанию результат равен false
  if (snake_coords[SNAKE_HEAD] == apple_coords) { // проверяет равенство координат головы и яблока
    res = true; // если совпадают, установка результата в true
  } // конец проверки совпадения координат
  return res; // возвращает результат проверки
} // конец метода check_collide_apple_of_snake

/**
 * @brief Проверяет координаты змейки на соприкосновение с головой.
 *
 * @return Результат проверки.
 */
bool Snake::check_collide_body_head() const { // начало метода проверки столкновения головы с телом змейки
  bool res = false; // по умолчанию результат равен false
  for (int i = SNAKE_HEAD + 1; i < snake_size; i++) { // проходит по всем сегментам тела, исключая голову
    if (snake_coords[i] == snake_coords[SNAKE_HEAD]) { // если координата сегмента совпадает с координатой головы
      res = true; // устанавливает результат в true при столкновении
    } // конец проверки совпадения для текущего сегмента
  } // конец цикла по сегментам тела
  return res; // возвращает результат проверки столкновения
} // конец метода check_collide_body_head

/**
 * @brief Отвечает за поворот головы.
 *
 * Поворачивает голову змейки в нужную сторону, производя вычисление с координатой y(first) или x(second).
 */
void Snake::rotate_head() { // начало метода изменения позиции головы в зависимости от направления
  if (curr_direction == Direction::Dir_Up) { // если направление вверх
    snake_coords[SNAKE_HEAD].first -= 1; // уменьшаем координату Y головы на 1
  } else if (curr_direction == Direction::Dir_Left) { // иначе если направление влево
    snake_coords[SNAKE_HEAD].second -= 1; // уменьшаем координату X головы на 1
  } else if (curr_direction == Direction::Dir_Down) { // иначе если направление вниз
    snake_coords[SNAKE_HEAD].first += 1; // увеличиваем координату Y головы на 1
  } else if (curr_direction == Direction::Dir_Right) { // иначе если направление вправо
    snake_coords[SNAKE_HEAD].second += 1; // увеличиваем координату X головы на 1
  } // конец условного блока по направлению
} // конец метода rotate_head

/**
 * @brief Переключаетль паузы.
 *
 * Ставит или снимает игру с паузы.
 */
void Snake::pause_game() { // начало метода переключения состояния паузы
  if (gameinfo.pause == UNPAUSE) { // если игра сейчас не на паузе
    gameinfo.pause = PAUSE; // ставим игру на паузу
  } else if (gameinfo.pause == PAUSE) { // иначе если игра на паузе
    gameinfo.pause = UNPAUSE; // снимаем паузу
  } // конец условий переключения паузы
} // конец метода pause_game

/**
 * @brief Отвечает за смещение координат змейки к голове.
 *
 * Производит сдвиг всех координат с конца к началу.
 * Голова не участвует в сдвиге т. к. ее движение обрабатывается в rotate_head().
 */
void Snake::shift_to_head() { // начало метода сдвига сегментов змейки к голове
  for (int i = snake_size - 1; i > SNAKE_HEAD; i--) { // цикл от хвоста к элементу после головы
    snake_coords[i] = snake_coords[i - 1]; // копирует координаты предыдущего сегмента в текущий (сдвигает сегменты)
  } // конец цикла сдвига
} // конец метода shift_to_head

/**
 * @brief Проверяет получилось ли повернуть змейку.
 *
 * Поворот обрабатывается не только по нажатой клавише, но и с учётом допустимых направлений: двигаясь вверх/вниз можно поворачивать только влево или вправо, а двигаясь влево/вправо — только вверх или вниз.
 * Переменная curr_direction использует собственное перечисление Direction, чтобы отделить логику направления от действия игрока UserAction_t action.
 * @return Результат проверки.
 */
bool Snake::check_rotate_head() const { // начало метода проверки допустимости поворота головы в соответствии с текущим направлением
  int res = false; // флаг результата, инициализированный как false (целочисленный тип здесь используется в оригинале)
  if (curr_direction == Direction::Dir_Up || // если текущее направление вверх
      curr_direction == Direction::Dir_Down) { // или вниз
    res = (action == Left || action == Right); // разрешён поворот влево или вправо в зависимости от действия игрока
  } else if (curr_direction == Direction::Dir_Left || // иначе если направление влево
             curr_direction == Direction::Dir_Right) { // или вправо
    res = (action == Up || action == Down); // разрешён поворот вверх или вниз в зависимости от действия игрока
  } // конец условий проверки направлений
  return res; // возвращает результат проверки
} // конец метода check_rotate_head

/**
 * @brief Устанавливает текущее направление змейки.
 *
 * В set_direction() сохраняются условия проверки корректности поворота, поскольку check_rotate_head() — лишь один из трёх «флагов» для перехода КА в состояние Shifting.
 * Даже если переход произошёл по другому «флагу», проверки всё равно выполняются, чтобы предотвратить возможные ошибки.
 */
void Snake::set_direction() { // начало метода установки текущего направления на основе действия игрока
  if (curr_direction == Direction::Dir_Up || // если текущее направление вверх
      curr_direction == Direction::Dir_Down) { // или вниз
    if (action == Left) { // если действие игрока — влево
      curr_direction = Direction::Dir_Left; // меняем направление на влево
    } else if (action == Right) { // если действие игрока — вправо
      curr_direction = Direction::Dir_Right; // меняем направление на вправо
    } // конец вложенных условий для движения по вертикали
  } else if (curr_direction == Direction::Dir_Left || // иначе если текущее направление влево
             curr_direction == Direction::Dir_Right) { // или вправо
    if (action == Up) { // если действие игрока — вверх
      curr_direction = Direction::Dir_Up; // меняем направление на вверх
    } else if (action == Down) { // если действие игрока — вниз
      curr_direction = Direction::Dir_Down; // меняем направление на вниз
    } // конец вложенных условий для движения по горизонтали
  } // конец внешнего условия установки направления
} // конец метода set_direction

/**
 * @brief Обновляет информацию о змейке на поле field.
 *
 * Проходит по массиву координат змейки и либо удляет их с поля, либо наносит.
 *
 * @param value Значение которое отвечает за SPAWN или DESPAWN координат на
 * поле.
 */
void Snake::snake_position_update(const int value) { // начало метода обновления отображения змейки на поле
  for (int i = SNAKE_HEAD; i < snake_size; i++) { // проходит по всем сегментам змейки от головы до хвоста
    std::pair<int, int> coord = snake_coords[i]; // получает текущую координату сегмента в локальную переменную coord
    if (value == DESPAWN) { // если передано значение удаления (DESPAWN)
      gameinfo.field[coord.first][coord.second] = DESPAWN; // помечает ячейку поля как очищенную (DESPAWN)
    } else if (value == SPAWN) { // иначе если передано значение появления (SPAWN)
      if (i == SNAKE_HEAD) { // если это голова змейки
        gameinfo.field[coord.first][coord.second] = SPAWN_COLOR_SNAKE_HEAD; // рисует голову с соответствующим цветом/значением
      } else { // если это не голова (обычный сегмент тела)
        gameinfo.field[coord.first][coord.second] = SPAWN_COLOR_SNAKE; // рисует сегмент тела с соответствующим цветом/значением
      } // конец проверки на голову или тело
    } // конец проверки значения SPAWN/DESPAWN
  } // конец цикла по сегментам змейки
} // конец метода snake_position_update


/**
 * @brief Обновление очков игрока.
 *
 * При колиизии и обработке яблока увеличивает счёт.
 * Если был поставлен новый рекорд, то перезаписывает его в файле.
 */
void Snake::update_score_game() { // начало метода увеличения счёта и обновления рекорда
  gameinfo.score++; // увеличивает поле score на единицу
  if (gameinfo.score > gameinfo.high_score) { // если текущий счёт превысил предыдущий рекорд
    gameinfo.high_score = gameinfo.score; // обновляет рекорд в структуре gameinfo
    write_record_file(gameinfo.high_score); // записывает новый рекорд в файл
  } // конец проверки и возможной записи рекорда
} // конец метода update_score_game

/**
 * @brief Отвечает за отображения яблока и его координаты.
 *
 * Выбирает одну случайную клетку из свободных и отрисовывает там яблоко
 * \throw std::runtime_error Если ошибка с координатами.
 */
void Snake::spawn_apple() { // начало метода спавна яблока на поле
  std::vector<std::pair<int, int>> free_cells; // создаёт вектор для хранения свободных координат
  int random_index; // индекс случайной выбранной свободной клетки
  get_free_cells(free_cells); // заполняет free_cells всеми доступными для яблока клетками
  random_index = get_random_index(free_cells.size()); // получает случайный индекс в диапазоне размера free_cells
  apple_coords = free_cells[random_index]; // устанавливает координаты яблока по выбранной случайной клетке
  if (coord_valid_check(apple_coords)) { // проверяет корректность полученных координат яблока
    std::pair<int, int> random_cell = free_cells[random_index]; // сохраняет выбранную клетку в локальную переменную random_cell
    gameinfo.field[random_cell.first][random_cell.second] = SPAWN_COLOR_APPLE; // ставит на поле символ/цвет яблока в выбранной клетке
  } else { // если координаты некорректны
    throw std::runtime_error("Error coordinate! Cant spawn apple"); // выбрасывает исключение о невозможности поставить яблоко
  } // конец проверки валидности координат
} // конец метода spawn_apple

/**
 * @brief GameOver (состояние конечного автомата).
 *
 * Завершает игру записывая в поле level структуры GameInfo_t соответсвующий код.
 * Код -1 сообщает представлению о стандратном выходе из игры,
 * Код 200 - о выходе из игры с сообщением о победе.
 * Все коды выхода из игры определены в общем заголовочном файле для удобства настройки.
 */
void Snake::game_over() { // начало метода обработки завершения игры
  if (snake_size >= SNAKE_MAX_SIZE) { // если змейка достигла максимального размера
    gameinfo.level = WIN_LVL; // устанавливает уровень/код завершения как победа (WIN_LVL)
  } else { // иначе
    gameinfo.level = LOSE_LVL; // устанавливает уровень/код завершения как поражение (LOSE_LVL)
  } // конец условия определения кода завершения
} // конец метода game_over

}  // namespace s21 // закрывает пространство имён s21
