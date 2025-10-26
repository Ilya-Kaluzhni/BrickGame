#include "brick_game_single.h" // подключает общий заголовок с определением Game, GameInfo_t и константами окна
#include "tetris/tetris.h" // подключает заголовок класса Tetris
#include "snake/snake.h" // подключает заголовок класса Snake

namespace s21 { // начало пространства имён s21

// ================= GameFabric ==================
void GameFabric::set_game(GameName name) { // метод установки текущей игры по перечислению GameName
  if (name == GameName::Tetris) { // если выбран Tetris
    current_game = Tetris::get_instance(); // берём единственный экземпляр Tetris (синглтон)
  } else if (name == GameName::Snake) { // иначе если выбран Snake
    current_game = Snake::get_instance(); // берём единственный экземпляр Snake (синглтон)
  } else { // если передано неизвестное имя игры
    throw std::runtime_error("Error: There is no game with this name"); // выбрасываем исключение о некорректном имени
  } // конец условной логики выбора игры
} // конец метода set_game

Game* GameFabric::get_game() { return current_game; } // возвращает указатель на текущую выбранную игру

// ================= Game ==================
Game::Game() { // конструктор базового класса Game
  gameinfo.field = matrix_init(WINDOW_HEIGHT, WINDOW_WIDTH); // инициализирует игровое поле матрицей высоты и ширины окна
  gameinfo.next = matrix_init(NEXT_SIZE, NEXT_SIZE); // инициализирует матрицу для отображения следующей фигуры
} // конец конструктора Game

Game::~Game() { // деструктор базового класса Game
  matrix_free(gameinfo.field, WINDOW_HEIGHT); // освобождает память основной матрицы поля
  matrix_free(gameinfo.next, NEXT_SIZE); // освобождает память матрицы для следующей фигуры
} // конец деструктора Game

void Game::set_user_action(UserAction_t user_input) { action = user_input; } // устанавливает действие пользователя в поле action

const GameInfo_t& Game::get_gameinfo() { return gameinfo; } // возвращает константную ссылку на структуру gameinfo

int** Game::matrix_init(const int rows, const int cols) { // выделяет динамическую матрицу rows x cols
  if (rows <= 0 || cols <= 0) // проверка валидности размеров
    throw std::invalid_argument("Error: Number of rows and columns must be greater than zero"); // выбрасывает исключение при некорректных размерах
  int** matrix = new int*[rows]; // выделяет массив указателей на строки
  for (int i = 0; i < rows; i++) { // цикл по каждой строке
    matrix[i] = new int[cols](); // выделяет и инициализирует строку нулями
  } // конец цикла выделения строк
  return matrix; // возвращает указатель на выделенную матрицу
} // конец метода matrix_init

void Game::matrix_free(int** matrix, const int rows) { // освобождает память матрицы с заданным количеством строк
  if (matrix) { // если матрица не равна NULL
    for (int i = 0; i < rows; i++) delete[] matrix[i]; // освобождаем каждую строку
    delete[] matrix; // освобождаем массив указателей на строки
  } // конец проверки перед освобождением
} // конец метода matrix_free

void Game::fsm() { // метод обработки конечного автомата состояний игры
  switch (this->statemachine) { // переключатель по текущему состоянию statemachine
    case GameStart: this->starting_game(); break; // если GameStart — вызываем starting_game
    case Spawn: this->spawn(); break; // если Spawn — вызываем spawn
    case Moving: this->moving(); break; // если Moving — вызываем moving
    case Shifting: this->shifting(); break; // если Shifting — вызываем shifting
    case Attaching: this->attaching(); break; // если Attaching — вызываем attaching
    case GameOver: this->game_over(); break; // если GameOver — вызываем game_over
    default: break; // для прочих значений ничего не делаем
  } // конец switch
} // конец метода fsm

// ================= Timer ==================
Timer::Timer() : start_time_(ClockType::now()) {} // конструктор Timer инициализирует время старта текущим моментом

void Timer::start() { start_time_ = ClockType::now(); } // сбрасывает время старта на текущий момент

bool Timer::game_timer_check(int speed, int max_delay, int min_delay, int max_speed) { // проверяет, истёк ли интервал времени для шага
  bool res = false; // по умолчанию результат false
  DurationMs delay = calculate_delay(speed, max_delay, min_delay, max_speed); // вычисляем задержку на основе скорости и лимитов
  DurationMs elapsed_time = get_elapsed_time(); // получаем прошедшее время с момента старта

  if (elapsed_time >= delay) { // если прошедшее время больше или равно требуемой задержке
    res = true; // помечаем, что таймер сработал
  } // конец проверки времени
  return res; // возвращаем результат проверки
} // конец метода game_timer_check

double Timer::get_miliseconds() const { // возвращает миллисекунды от прошедшего времени в пределах секунды
  return get_elapsed_time().count() % 1000; // берёт остаток миллисекунд от общего прошедшего времени
} // конец метода get_miliseconds

int Timer::get_seconds() const { // возвращает количество секунд прошедшего времени в пределах минуты
  return get_elapsed_time().count() / 1000 % 60; // преобразует миллисекунды в секунды и берёт остаток по минуте
} // конец метода get_seconds

int Timer::get_minutes() const { // возвращает количество минут прошедшего времени в пределах часа
  return get_elapsed_time().count() / 60000 % 60; // преобразует миллисекунды в минуты и берёт остаток по часу
} // конец метода get_minutes

Timer::DurationMs Timer::get_elapsed_time() const { // получает DurationMs, представляющее прошедшее время
  return std::chrono::duration_cast<DurationMs>(ClockType::now() - start_time_); // разница между текущим моментом и временем старта
} // конец метода get_elapsed_time

Timer::DurationMs Timer::calculate_delay(int speed, int max_delay, int min_delay, int max_speed) const { // вычисляет задержку в зависимости от скорости
  return DurationMs(max_delay -
                    (speed - 1) * (max_delay - min_delay) / (max_speed - 1)); // линейная интерполяция задержки по скорости
} // конец метода calculate_delay

} // namespace s21 // конец пространства имён s21

// ================= API ==================
void userInput(UserAction_t input, bool hold) { // глобальная функция API для передачи ввода пользователя в движок
  (void)hold; // явно игнорируем параметр hold если он не используется

  static bool first_call = true; // статический флаг первого вызова функции
  if (first_call) { // если функция вызывается в первый раз
    // Устанавливаем игру на основе первого ввода
    s21::GameFabric::set_game(s21::GameFabric::GameName(input)); // приводим input к GameName и устанавливаем игру
    input = UserAction_t::Start; // заменяем ввод на Start чтобы инициировать начало игры
    first_call = false; // отмечаем, что первый вызов уже произошёл
  } // конец обработки первого вызова

  s21::Game* current_game = s21::GameFabric::get_game(); // получаем указатель на текущую игру через фабрику
  if (current_game) { // если игра установлена
    current_game->set_user_action(input); // передаём действие пользователя в текущую игру
  } // конец проверки current_game
} // конец функции userInput

GameInfo_t updateCurrentState() { // глобальная функция API для обновления и получения текущего состояния игры
  GameInfo_t gameinfo{}; // создаём локальную структуру gameinfo для возврата
  s21::Game* current_game = s21::GameFabric::get_game(); // получаем указатель на текущую игру
  if (current_game) { // если текущая игра установлена
    current_game->fsm(); // выполняем один шаг конечного автомата игры
    gameinfo = current_game->get_gameinfo(); // получаем актуальную информацию об игре
  } // конец проверки наличия текущей игры
  return gameinfo; // возвращаем полученную структуру GameInfo_t
} // конец функции updateCurrentState
