#include "frontend.h" // подключает заголовок с прототипами функций фронтенда и ncurses

int main(void) { // точка входа в программу
  WINDOW* my_win; // указатель на окно ncurses

  ncurses_init(); // инициализируем ncurses и настройки терминала
  my_win = create_new_window(); // создаём новое окно для интерфейса
  print_interface(my_win); // рисуем статическую часть интерфейса
  game_loop(my_win); // запускаем главный игровой цикл
  destroy_win(my_win); // удаляем созданное окно
  endwin(); // завершаем работу ncurses и возвращаем терминал в нормальный режим

  return 0; // возвращаем код успешного завершения
} // конец main

void ncurses_init() { // инициализация ncurses и базовых параметров интерфейса
  srand(time(NULL)); // инициализируем генератор случайных чисел текущим временем
  initscr(); // инициализируем экран ncurses
  if (!has_colors()) { // если терминал не поддерживает цвета
    printf("not found color"); // печатаем сообщение в stdout (вне ncurses)
  } // конец проверки поддержки цвета
  start_color(); // включаем поддержку цветов в ncurses
  cbreak(); // отключаем буферизацию строки, ввод доступен сразу
  noecho(); // отключаем автоматический эхо-вывод вводимых символов
  curs_set(0); // скрываем курсор
  keypad(stdscr, TRUE); // включаем обработку функциональных клавиш (стрелки и т.д.)
  init_pair(0, COLOR_BLACK, COLOR_BLACK); // инициализируем пару цветов 0 — черный на черном
  init_pair(2, COLOR_RED, COLOR_RED); // инициализируем пару цветов 2 — красный на красном
  timeout(1); // устанавливаем неблокирующий режим getch с таймаутом 1 миллисекунда
} // конец ncurses_init

void score_to_string(char* str, int score) { // форматирование целочисленного счёта в 7-символьную строку
  int number = 0; // временная переменная для цифры
  if (score <= 9999999) { // если счёт помещается в 7 цифр
    for (int i = 6; i >= 0; i--) { // заполняем строку с конца до начала
      number = score % 10; // извлекаем младшую цифру
      str[i] = number + 48; // преобразуем цифру в символ ASCII и записываем в строку
      score = score / 10; // удаляем младшую цифру из числа
    } // конец цикла заполнения символов
  } // конец проверки верхнего предела счёта
} // конец score_to_string

void game_loop(WINDOW* my_win) { // главный цикл игры, обрабатывает ввод и обновляет экран
  GameInfo_t stats; // структура для получения состояния игры
  s21::Timer timer; // локальный таймер (может быть неиспользуемым, но инициализируется)
  int game = selection_game(my_win); // меню выбора игры возвращает выбранный идентификатор
  userInput((UserAction_t)game, false); // передаём выбор игры через API как вход пользователя

  while (!is_end(stats)) { // пока игра не завершена
    set_user_action(); // считываем пользовательский ввод и преобразуем в действие
    stats = updateCurrentState(); // обновляем состояние игры (один шаг КА) и получаем gameinfo
    if (!is_end(stats)) { // если после шага игра ещё не завершена
      update_screen(stats, my_win); // обновляем содержимое экрана на основе stats
    } // конец проверки состояния перед отрисовкой
    wrefresh(my_win); // перерисовываем окно ncurses
  } // конец основного игрового цикла

  if (stats.level == LOSE_LVL) { // если игра завершилась проигрышем
    print_end(my_win); // показываем сообщение GAME OVER
  } else if (stats.level == WIN_LVL) { // если игра завершилась победой
    print_win(my_win); // показываем сообщение YOU WIN!
  } // конец проверки результатов игры
  wrefresh(my_win); // перерисовываем окно чтобы отобразить финальное сообщение
  sleep(1); // даём пользователю секунду, чтобы увидеть сообщение перед выходом
} // конец game_loop

bool is_end(GameInfo_t stats) { // проверяет, достигнуто ли конечное состояние игры
  return (stats.level == LOSE_LVL || stats.level == WIN_LVL) ? true : false; // возвращает true если уровень соответствует коду конца
} // конец is_end

void set_user_action() { // читает нажатия клавиш и отправляет соответствующие действия в движок
    int ch = getch(); // неблокирующее чтение символа из ncurses
    switch (ch) { // сопоставление кода клавиши с действием пользователя
        case KEY_LEFT: // стрелка влево
            userInput(Left, false); // передаём действие Left
            break;
        case KEY_RIGHT: // стрелка вправо
            userInput(Right, false); // передаём действие Right
            break;
        case KEY_UP: // стрелка вверх
            userInput(Up, false); // передаём действие Up
            break;
        case KEY_DOWN: // стрелка вниз
            userInput(Down, false); // передаём действие Down
            break;
        case ' ': // пробел
            userInput(Action, false); // передаём действие Action
            break;
        case 'p': // буква p
        case 'P': // или заглавная P
            userInput(Pause, false); // переключаем паузу
            break;
        case 27: // код ESC
            userInput(Terminate, false); // передаём действие Terminate
            break;
        case 's': // буква s
        case 'S': // или заглавная S
            userInput(Start, false); // передаём действие Start
            break;
        default: // прочие клавиши
            break; // игнорируем
    } // конец switch
} // конец set_user_action

WINDOW* create_new_window() { // создаёт и возвращает новое окно ncurses под игровой интерфейс
  WINDOW* local_win; // локальный указатель на окно
  int starty, startx, width, height; // параметры позиции и размеров окна
  height = 2 + WINDOW_HEIGHT + 5; // вычисляем высоту окна с отступами
  width = 3 + WINDOW_WIDTH * 2 + 11; // вычисляем ширину окна, учитывая по два символа на колонку поля и боковые панели
  starty = 2; // начальная строка окна
  startx = 4; // начальный столбец окна
  local_win = newwin(height, width, starty, startx); // создаём окно заданного размера и позиции
  return local_win; // возвращаем созданное окно
} // конец create_new_window


int selection_game(WINDOW* local_win) { // меню выбора игры (Tetris или Snake)
  int game = Tetris; // по умолчанию выбираем Tetris
  int switch_flag = 1; // флаг для управления отображением указателя
  mvwaddch(local_win, 10, 8, '>'); // рисуем символ '>' напротив первой опции меню
  mvwaddstr(local_win, 10, 9, "Tetris"); // выводим текст "Tetris" в окне
  mvwaddstr(local_win, 11, 9, "Snake"); // выводим текст "Snake" на следующей строке
  int input = getch(); // читаем ввод пользователя
  while (input != 'a' && input != 'q' && input != 's') { // цикл до нажатия завершающих клавиш ('a','q','s')
    if (input == KEY_UP || input == KEY_DOWN) { // если нажата стрелка вверх или вниз
      game = (game == Tetris ? Snake : Tetris); // переключаем выбранную игру
      switch_flag = (switch_flag == 0 ? 1 : 0); // переключаем флаг состояния указателя
      mvwaddch(local_win, 10 + switch_flag, 8, ' '); // очищаем символ указателя на старой позиции
      mvwaddch(local_win, 11 - switch_flag, 8, '>'); // рисуем символ указателя на новой позиции
    } // конец обработки стрелок
    wrefresh(local_win); // обновляем окно чтобы отобразить изменения
    input = getch(); // читаем следующий ввод
  } // конец цикла выбора игры
  mvwaddstr(local_win, 9, 8, "          "); // очищаем строку меню (заменяем пробелами)
  mvwaddstr(local_win, 10, 8, "          "); // очищаем следующую строку меню
  wrefresh(local_win); // обновляем окно после очистки текста
  if (game == Tetris) { // если выбрали Tetris
    print_tetris(local_win); // показываем заголовок Tetris
  } else if (game == Snake) { // если выбрали Snake
    print_snake(local_win); // показываем заголовок Snake
  } // конец выбора заголовка
  wrefresh(local_win); // обновляем окно чтобы отобразить заголовок
  sleep(1); // небольшая пауза перед стартом игры
  return game; // возвращаем код выбранной игры
} // конец selection_game

void print_interface(WINDOW* local_win) { // рисует статическую интерфейсную рамку и подписи
  box(local_win, 0, 0); // рисует рамку вокруг окна
  mvwvline(local_win, 1, 21, ACS_VLINE, 20); // рисует вертикальную линию раздела
  mvwhline(local_win, 21, 1, ACS_HLINE, 32); // рисует горизонтальную линию внизу секции
  mvwaddstr(local_win, 2, 25, "SCORE"); // подпись SCORE
  mvwaddstr(local_win, 5, 23, "HI- SCORE"); // подпись HI- SCORE
  mvwaddstr(local_win, 8, 25, "NEXT"); // подпись NEXT (окно следующей фигуры)
  mvwaddstr(local_win, 12, 25, "SPEED"); // подпись SPEED
  mvwaddstr(local_win, 15, 25, "LEVEL"); // подпись LEVEL
  mvwaddstr(local_win, 22, 2, "ENTER  START"); // подсказка управления Enter (Start)
  mvwaddstr(local_win, 22, 2, "SPACE  ACTION"); // эта строка перекрывает предыдущую, но оставлена в коде
  mvwaddstr(local_win, 23, 2, "P      PAUSE"); // подсказка P для паузы
  mvwaddstr(local_win, 24, 2, "ESC    QUIT"); // подсказка ESC для выхода
  mvwaddch(local_win, 22, 26, ACS_UARROW); // размещение символа стрелки вверх (декоративно)
  mvwaddch(local_win, 22, 24, ACS_LARROW); // символ стрелки влево
  mvwaddch(local_win, 22, 28, ACS_RARROW); // символ стрелки вправо
  mvwaddch(local_win, 22, 26, ACS_DARROW); // символ стрелки вниз (перезаписывает предыдущий UARROW)
} // конец print_interface

void print_tetris(WINDOW* local_win) { // выводит надпись TETRIS по центру игрового поля в окне
    const char* text = "TETRIS"; // текст для вывода
    int row = (WINDOW_HEIGHT + 2) / 2; // вычисляем строку для центрирования по вертикали
    int col = (WINDOW_WIDTH * 2 + 3) / 2 - strlen(text)/2; // вычисляем столбец для центрирования по горизонтали
    mvwaddstr(local_win, row, col, text); // выводим текст в окно
} // конец print_tetris

void print_snake(WINDOW* local_win) { // выводит надпись SNAKE по центру игрового поля в окне
    const char* text = "SNAKE"; // текст для вывода
    int row = (WINDOW_HEIGHT + 2) / 2; // вычисляем строку для центрирования по вертикали
    int col = (WINDOW_WIDTH * 2 + 3) / 2 - strlen(text)/2; // вычисляем столбец для центрирования по горизонтали
    mvwaddstr(local_win, row, col, text); // выводим текст в окно
} // конец print_snake

void print_pause(WINDOW* local_win) { // выводит надпись PAUSE по центру игрового поля в окне
    const char* text = "PAUSE"; // текст для вывода
    int row = (WINDOW_HEIGHT + 2) / 2; // вычисляем строку для центрирования по вертикали
    int col = (WINDOW_WIDTH * 2 + 3) / 2 - strlen(text)/2; // вычисляем столбец для центрирования по горизонтали
    mvwaddstr(local_win, row, col, text); // выводим текст в окно
} // конец print_pause

void update_screen(GameInfo_t stats, WINDOW* local_win) { // обновляет экран на основе текущего состояния игры
    char score_str[8] = {0}; // буфер для форматированной строки счёта (7 символов + терминатор)
    char high_score_str[8] = {0}; // буфер для рекорда
    score_str[7] = '\0'; // явно устанавливаем терминатор в конце буфера
    high_score_str[7] = '\0'; // явно устанавливаем терминатор в конце буфера
    score_to_string(score_str, stats.score); // форматируем текущий счёт в строку
    score_to_string(high_score_str, stats.high_score); // форматируем рекорд в строку

    print_stats_field(stats, local_win); // отрисовываем основное игровое поле
    print_stats_next(stats, local_win); // отрисовываем окно следующей фигуры

    if (stats.pause == 1) { // если игра на паузе
        print_pause(local_win); // показываем надпись PAUSE
    } // конец проверки паузы
    mvwaddstr(local_win, 3, 31 - strlen(score_str), score_str); // выводим строку счёта справа от метки
    mvwaddstr(local_win, 6, 31 - strlen(high_score_str), high_score_str); // выводим рекорд
    mvwprintw(local_win, 13, 27, "%d", stats.speed); // выводим текущее значение скорости
    mvwprintw(local_win, 16, 27, "%d", stats.level); // выводим текущий уровень
} // конец update_screen


void print_stats_field(GameInfo_t stats, WINDOW* local_win) { // отрисовка основного поля игры в окне
    int k = 1; // смещение по колонкам для отрисовки с учётом двойной ширины ячейки
    for (int i = 0; i < WINDOW_HEIGHT; i++) { // цикл по строкам игрового поля
        for (int j = 0; j < WINDOW_WIDTH; j++) { // цикл по столбцам игрового поля
            int color = (stats.field[i][j] != 0) ? 2 : 0; // выбираем цвет: 2 если ячейка занята, 0 если пуста
            mvwaddch(local_win, i + 1, j + k, ' ' | COLOR_PAIR(color)); // рисуем левую половину ячейки как пробел с фоновым цветом
            k++; // сдвигаем позицию для правой половины
            mvwaddch(local_win, i + 1, j + k, ' ' | COLOR_PAIR(color)); // рисуем правую половину ячейки как пробел с тем же цветом
        } // конец внутреннего цикла по столбцам
        k = 1; // сбрасываем смещение в начало для следующей строки
    } // конец внешнего цикла по строкам
} // конец print_stats_field

void print_stats_next(GameInfo_t stats, WINDOW* local_win) { // отрисовка окна "NEXT" (слева от метки NEXT)
    int k = 1; // вспомогательное смещение по колонкам внутри области NEXT
    for (int i = 0; i < 2; i++) { // фиксированно отрисовываем только две строки области next (верхняя часть)
        for (int j = 3; j < 7; j++) { // проходим по столбцам внутри области next, смещая диапазон для центрирования
            int color = (stats.next[i][j] != 0) ? 2 : 0; // выбираем цвет ячейки next (занята/пусто)
            mvwaddch(local_win, i + 9, j + k + 21, ' ' | COLOR_PAIR(color)); // рисуем левую половину ячейки next
            k++; // смещаем позицию для правой половины
            mvwaddch(local_win, i + 9, j + k + 21, ' ' | COLOR_PAIR(color)); // рисуем правую половину ячейки next
        } // конец внутреннего цикла по столбцам области next
        k = 1; // сбрасываем смещение для следующей строки области next
    } // конец внешнего цикла по строкам области next
} // конец print_stats_next

void destroy_win(WINDOW* local_win) { // удаляет окно и очищает его границы
  wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '); // очищаем рамку окна пробелами
  wrefresh(local_win); // обновляем окно чтобы отобразить очищение
  delwin(local_win); // удаляем объект окна
} // конец destroy_win

void print_end(WINDOW* local_win) { // отображает сообщение GAME OVER в центре игрового поля
    const char* text = "GAME OVER"; // текст для отображения
    int row = (WINDOW_HEIGHT + 2) / 2; // вычисляем строку для центрирования по вертикали
    int col = (WINDOW_WIDTH * 2 + 3) / 2 - strlen(text)/2; // вычисляем столбец для центрирования по горизонтали
    mvwaddstr(local_win, row, col, text); // выводим текст в окно
} // конец print_end

void print_win(WINDOW* local_win) { // отображает сообщение YOU WIN! в центре игрового поля
    const char* text = "YOU WIN!"; // текст для отображения
    int row = (WINDOW_HEIGHT + 2) / 2; // вычисляем строку для центрирования по вертикали
    int col = (WINDOW_WIDTH * 2 + 3) / 2 - strlen(text)/2; // вычисляем столбец для центрирования по горизонтали
    mvwaddstr(local_win, row, col, text); // выводим текст в окно
} // конец print_win
