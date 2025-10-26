#include "gtk_frontend.h" // подключает заголовок с объявлениями класса MyGtkWindow и областей отрисовки
#define COLOR_RED_BLOCK(cr) \ // макрос для установки красного цвета в контексте Cairo через метод set_source_rgb
    cr->set_source_rgb(1, 0, 0); // задаёт красный цвет (R=1,G=0,B=0) для переданного контекста cr

#include "../../brick_game/brick_game_single.h" // подключает общий заголовок с игровыми структурами и константами

#define GTK_WINDOW_Y 400 // задаёт высоту окна GTK в пикселях (константа)
#define GTK_WINDOW_X 550 // задаёт ширину окна GTK в пикселях (константа)

#define SCALE 20.0 // масштаб одного блока игрового поля в пикселях для отрисовки
#define FRAME_SHIFT 20 // отступы рамки вокруг игрового поля в пикселях

#define NEXT_SHIFT 3 // сдвиг/обрезание области NEXT при расчётах размера кадра

MyGtkWindow::MyGtkWindow() // конструктор окна приложения MyGtkWindow
    : main_box(Gtk::Orientation::VERTICAL), // инициализирует главный вертикальный контейнер
      button_box(Gtk::Orientation::VERTICAL), // инициализирует контейнер для кнопочных подсказок
      start_box(Gtk::Orientation::VERTICAL), // инициализирует стартовый вертикальный контейнер
      info_box(Gtk::Orientation::VERTICAL), // инициализирует контейнер для информационной панели
      tetris_button("Tetris"), // создаёт кнопку Tetris с подписью
      snake_button("Snake"), // создаёт кнопку Snake с подписью
      exit_button("Exit") { // создаёт кнопку Exit с подписью

    set_title("Brick game"); // устанавливает заголовок окна
    set_default_size(GTK_WINDOW_Y, GTK_WINDOW_X); // задаёт размер окна по умолчанию (высота, ширина)

    // --- Тёмный стиль через CSS ---
    auto provider = Gtk::CssProvider::create(); // создаёт провайдер CSS-стилей
    provider->load_from_data(R"(
window {
    background-color: #1e1e1e;
    color: #ffffff;
}
frame {
    border-radius: 0px;
    border: 1px solid #555555;
    background-color: #2c2c2c;
}
button {
    background-color: #f0f0f0; /* светлый фон кнопки */
    border: 1px solid #555555;
    border-radius: 0px;
}
button label {
    color: #000000; /* тёмный текст */
}
button:hover {
    background-color: #e0e0e0;
}
label {
    color: #ffffff;
}
)"); // загружает CSS-правила для темной темы интерфейса

    Gtk::StyleContext::add_provider_for_display(
        Gdk::Display::get_default(), provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION); // применяет CSS-провайдер ко всему приложению

    set_child(main_box); // устанавливает main_box как дочерний виджет окна

    start_box.set_margin_top(100); // задаёт верхний отступ для стартового бокса
    start_box.set_spacing(50); // задаёт расстояние между дочерними элементами в start_box

    tetris_button.get_style_context()->add_class("dark-button"); // добавляет CSS-класс к кнопке Tetris
    snake_button.get_style_context()->add_class("dark-button"); // добавляет CSS-класс к кнопке Snake
    exit_button.get_style_context()->add_class("dark-button"); // добавляет CSS-класс к кнопке Exit

    start_box.set_spacing(20); // устанавливает внутренний отступ между элементами start_box
    start_box.set_valign(Gtk::Align::CENTER); // вертикально выравнивает start_box по центру окна
    start_box.set_halign(Gtk::Align::CENTER); // горизонтально выравнивает start_box по центру окна

    Gtk::Box* game_buttons = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL); // создаёт управляемый горизонтальный контейнер для кнопок игр
    game_buttons->set_spacing(30); // задаёт расстояние между игровыми кнопками

    tetris_button.set_size_request(120, 60); // задаёт минимальный размер кнопки Tetris (ширина, высота)
    snake_button.set_size_request(120, 60); // задаёт минимальный размер кнопки Snake
    exit_button.set_size_request(120, 60); // задаёт минимальный размер кнопки Exit

    game_buttons->append(snake_button); // добавляет кнопку Snake в контейнер игровых кнопок слева
    game_buttons->append(tetris_button); // добавляет кнопку Tetris в контейнер игровых кнопок справа

    start_box.append(*game_buttons); // добавляет контейнер игровых кнопок в стартовый бокс
    start_box.append(exit_button); // добавляет кнопку выхода в стартовый бокс

    exit_button.set_halign(Gtk::Align::CENTER); // выравнивает кнопку выхода по центру по горизонтали


    main_box.append(start_box); // добавляет стартовый бокс в главный контейнер окна

    tetris_button.signal_clicked().connect( // подключает обработчик нажатия для кнопки Tetris
        sigc::mem_fun(*this, &MyGtkWindow::clicked_button_tetris));
    snake_button.signal_clicked().connect( // подключает обработчик нажатия для кнопки Snake
        sigc::mem_fun(*this, &MyGtkWindow::clicked_button_snake));
    exit_button.signal_clicked().connect( // подключает обработчик нажатия для кнопки Exit
        sigc::mem_fun(*this, &MyGtkWindow::clicked_button_exit));
} // конец конструктора MyGtkWindow


void MyGtkWindow::start_game() { // метод подготовки интерфейса и запуска игрового экрана
    main_box.remove(start_box); // удаляет стартовый бокс из главного контейнера

    game_area = Gtk::manage(new GameArea); // создаёт и управляет виджетом игрового поля GameArea
    Gtk::Box field_box; // локальный контейнер для поля
    Gtk::Frame *game_area_frame = Gtk::manage(new Gtk::Frame); // создаёт рамку для игрового поля и управляет её памятью
    game_area_frame->set_child(*game_area); // помещает GameArea внутрь рамки
    setup_game_area_frame(game_area_frame); // настраивает внешний вид и размеры рамки игрового поля
    field_box.append(*game_area_frame); // добавляет рамку с игровым полем в контейнер field_box

    next_area = Gtk::manage(new NextArea); // создаёт и управляет виджетом области NEXT
    Gtk::Frame *next_area_frame = Gtk::manage(new Gtk::Frame); // создаёт рамку для области NEXT
    next_area_frame->set_size_request((NEXT_SIZE - NEXT_SHIFT) * SCALE,
                                      (NEXT_SIZE - NEXT_SHIFT) * SCALE); // задаёт размер рамки NEXT на основе констант и масштаба
    next_area_frame->set_child(*next_area); // помещает NextArea внутрь рамки

    Gtk::Box arrow_box(Gtk::Orientation::VERTICAL); // создаёт вертикальный контейнер для подсказок управления стрелками
    arrow_box.set_spacing(5); // устанавливает промежуток между метками стрелок
    auto up_label    = Gtk::make_managed<Gtk::Label>("↑  UP"); // создаёт метку "UP" управляемо
    auto down_label  = Gtk::make_managed<Gtk::Label>("↓  DOWN"); // создаёт метку "DOWN"
    auto left_label  = Gtk::make_managed<Gtk::Label>("←  LEFT"); // создаёт метку "LEFT"
    auto right_label = Gtk::make_managed<Gtk::Label>("→  RIGHT"); // создаёт метку "RIGHT"
    up_label->set_halign(Gtk::Align::START); // выравнивание метки UP по началу (левая сторона)
    down_label->set_halign(Gtk::Align::START); // выравнивание метки DOWN по началу
    left_label->set_halign(Gtk::Align::START); // выравнивание метки LEFT по началу
    right_label->set_halign(Gtk::Align::START); // выравнивание метки RIGHT по началу
    arrow_box.append(*up_label); // добавляет метку UP в контейнер подсказок
    arrow_box.append(*down_label); // добавляет метку DOWN в контейнер подсказок
    arrow_box.append(*left_label); // добавляет метку LEFT в контейнер подсказок
    arrow_box.append(*right_label); // добавляет метку RIGHT в контейнер подсказок

    setup_button_labels(); // настраивает и добавляет подписи управления в button_box
    setup_info_box(next_area_frame); // настраивает информационную панель и добавляет рамку NEXT как дочерний элемент

    Gtk::Box top_box(Gtk::Orientation::HORIZONTAL); // создаёт верхний горизонтальный контейнер для поля и инфо
    top_box.append(field_box); // добавляет поле в top_box
    top_box.append(info_box); // добавляет информационную панель в top_box

    Gtk::Box control_box(Gtk::Orientation::HORIZONTAL); // создаёт контейнер для кнопок и подсказок управления
    control_box.set_margin_start(20); // задаёт левый отступ для control_box
    control_box.append(button_box); // добавляет box с текстовыми подсказками кнопок
    control_box.append(arrow_box); // добавляет box с подсказками стрелок

    main_box.append(top_box); // добавляет верхнюю панель (поле + инфо) в главный контейнер
    main_box.append(control_box); // добавляет панель с контролами под верхней панелью

    auto button_controller = Gtk::EventControllerKey::create(); // создаёт контроллер для обработки нажатий клавиш
    button_controller->signal_key_pressed().connect( // подключает обработчик событий клавиатуры
        sigc::mem_fun(*this, &MyGtkWindow::key_press), false);
    add_controller(button_controller); // регистрирует контроллер на уровне окна

    timer_connection = Glib::signal_timeout().connect( // создаёт таймерный сигнал с периодом 5 мс для обновления игры
        sigc::mem_fun(*this, &MyGtkWindow::update_game), 5);
} // конец метода start_game

void MyGtkWindow::setup_game_area_frame(Gtk::Frame *game_area_frame) { // настройка внешнего вида и размеров рамки игрового поля
    game_area_frame->set_label_align(Gtk::Align::CENTER); // выравнивание заголовка рамки по центру
    game_area_frame->set_margin_start(FRAME_SHIFT); // левый отступ рамки
    game_area_frame->set_margin_end(FRAME_SHIFT); // правый отступ рамки
    game_area_frame->set_margin_top(FRAME_SHIFT); // верхний отступ рамки
    game_area_frame->set_margin_bottom(FRAME_SHIFT); // нижний отступ рамки
    game_area_frame->set_size_request(WINDOW_WIDTH * SCALE, WINDOW_HEIGHT * SCALE); // задаёт размер рамки на основе размеров поля и масштаба
    game_area_frame->get_style_context()->add_class("game-frame"); // добавляет CSS-класс для стилизации рамки игрового поля
} // конец setup_game_area_frame

void MyGtkWindow::setup_button_labels() { // сформировать текстовые метки подсказок управления и добавить их в button_box
    quit_label.set_markup("<span font_desc='15'>ESC - QUIT</span>"); // задаёт форматированный текст для метки выхода
    pause_label.set_markup("<span font_desc='15'>P - PAUSE</span>"); // задаёт форматированный текст для метки паузы
    action_label.set_markup("<span font_desc='15'>SPACE - ACTION</span>"); // задаёт форматированный текст для метки действия
    start_label.set_valign(Gtk::Align::START); // вертикальное выравнивание метки START
    quit_label.set_valign(Gtk::Align::START); // вертикальное выравнивание метки QUIT
    pause_label.set_valign(Gtk::Align::START); // вертикальное выравнивание метки PAUSE
    action_label.set_valign(Gtk::Align::START); // вертикальное выравнивание метки ACTION

    button_box.set_margin_end(50); // правый отступ для блока с метками
    button_box.append(start_label); // добавляет метку START в button_box
    button_box.append(quit_label); // добавляет метку QUIT в button_box
    button_box.append(pause_label); // добавляет метку PAUSE в button_box
    button_box.append(action_label); // добавляет метку ACTION в button_box
} // конец setup_button_labels

void MyGtkWindow::setup_info_box(Gtk::Frame *next_area_frame) { // настройка информационной панели справа от поля
    info_box.set_margin_start(10); // левый отступ для info_box
    info_box.set_margin_end(10); // правый отступ для info_box
    info_box.set_margin_top(10); // верхний отступ для info_box
    info_box.set_margin_bottom(10); // нижний отступ для info_box
    info_box.append(score_label); // добавляет подпись SCORE
    info_box.append(score_value_label); // добавляет значение счёта
    info_box.append(hi_score_label); // добавляет подпись HI-SCORE
    info_box.append(hi_score_value_label); // добавляет значение рекорда
    info_box.append(next_label); // добавляет подпись NEXT
    info_box.append(*next_area_frame); // добавляет рамку с областью NEXT в info_box
    info_box.append(speed_label); // добавляет подпись SPEED
    info_box.append(speed_value_label); // добавляет значение скорости
    info_box.append(level_label); // добавляет подпись LEVEL
    info_box.append(level_value_label); // добавляет значение уровня
    score_label.set_markup("<span font_desc='15'>SCORE</span>"); // форматирует текст подписи SCORE
    score_value_label.set_markup("<span font_desc='15'>000000</span>"); // инициализирует отображение значения счёта
    hi_score_label.set_markup("<span font_desc='15'>HI-SCORE</span>"); // форматирует текст подписи HI-SCORE
    hi_score_value_label.set_markup("<span font_desc='15'>000000</span>"); // инициализирует отображение рекорда
    next_label.set_markup("<span font_desc='15'>NEXT</span>"); // форматирует подпись NEXT
    speed_label.set_markup("<span font_desc='15'>SPEED</span>"); // форматирует подпись SPEED
    speed_value_label.set_markup("<span font_desc='15'>0</span>"); // задаёт начальное значение скорости в интерфейсе
    level_label.set_markup("<span font_desc='15'>LEVEL</span>"); // форматирует подпись LEVEL
    level_value_label.set_markup("<span font_desc='15'>1</span>"); // задаёт начальное значение уровня в интерфейсе

    hi_score_label.set_margin_top(20); // задаёт верхний отступ для подписи рекорда
    next_label.set_margin_top(20); // задаёт верхний отступ для подписи NEXT
    speed_label.set_margin_top(20); // задаёт верхний отступ для подписи SPEED
    level_label.set_margin_top(20); // задаёт верхний отступ для подписи LEVEL
} // конец setup_info_box
bool MyGtkWindow::key_press(guint16 keyval, guint, Gdk::ModifierType state) { // обработчик нажатий клавиш в окне, возвращает флаг продолжения работы
    bool res = true; // результат обработки, true — продолжать таймер/обновления
    (void)state; // явно игнорируем параметр модификаторов, чтобы избежать предупреждений компилятора
    if (keyval == GDK_KEY_Right) { // если нажата правая стрелка
        userInput(UserAction_t::Right, false); // отправляем действие Right в движок
    } else if (keyval == GDK_KEY_Left) { // если нажата левая стрелка
        userInput(UserAction_t::Left, false); // отправляем действие Left в движок
    } else if (keyval == GDK_KEY_Up) { // если нажата стрелка вверх
        userInput(UserAction_t::Up, false); // отправляем действие Up в движок
    } else if (keyval == GDK_KEY_Down) { // если нажата стрелка вниз
        userInput(UserAction_t::Down, false); // отправляем действие Down в движок
    } else if (keyval == GDK_KEY_s || keyval == GDK_KEY_S) { // если нажата клавиша s или S
        userInput(UserAction_t::Start, false); // отправляем действие Start в движок
    } else if (keyval == GDK_KEY_space) { // если нажата пробел
        userInput(UserAction_t::Action, false); // отправляем действие Action в движок
    } else if (keyval == GDK_KEY_p || keyval == GDK_KEY_P) { // если нажата p или P
        userInput(UserAction_t::Pause, false); // отправляем действие Pause в движок
    } else if (keyval == GDK_KEY_Escape) { // если нажата клавиша Escape
        userInput(UserAction_t::Terminate, false); // отправляем действие Terminate в движок
        close(); // закрываем окно приложения
        res = false; // устанавливаем результат в false чтобы остановить таймер обновлений
    }
    return res; // возвращаем флаг продолжения/остановки
}

void MyGtkWindow::clicked_button_tetris() { // обработчик клика по кнопке Tetris
    userInput(UserAction_t(Tetris), false); // отправляем в движок выбор игры Tetris
    start_game(); // переключаем интерфейс на игровой режим и запускаем игру
}

void MyGtkWindow::clicked_button_snake() { // обработчик клика по кнопке Snake
    userInput(UserAction_t(Snake), false); // отправляем в движок выбор игры Snake
    start_game(); // переключаем интерфейс на игровой режим и запускаем игру
}

void MyGtkWindow::clicked_button_exit() { close(); } // обработчик клика Exit — закрывает окно

bool MyGtkWindow::update_game() { // вызывается таймером; обновляет состояние игры и интерфейс; возвращает true для продолжения таймера
    bool res = false; // по умолчанию прерываем таймер, если игра завершена или возникла ошибка
    current_state = updateCurrentState(); // запрашиваем у движка следующий шаг и состояние игры
    if (current_state.level == LOSE_LVL) { // если состояние сообщает о проигрыше
        show_game_over_dialog("you lose"); // показываем диалог окончания игры с сообщением о проигрыше
        res = false; // прекращаем таймер обновлений
    } else if (current_state.level == WIN_LVL) { // если состояние сообщает о победе
        show_game_over_dialog("you win"); // показываем диалог окончания игры с сообщением о победе
        res = false; // прекращаем таймер обновлений
    } else { // если игра продолжается
        game_area->game_field = current_state.field; // передаём указатель на основное поле в виджет игрового поля
        game_area->queue_draw(); // ставим задачу перерисовки игрового поля

        next_area->next_field = current_state.next; // передаём указатель на матрицу next во виджет NEXT
        next_area->queue_draw(); // ставим задачу перерисовки области NEXT
        info_update_game(); // обновляем текстовые метки с информацией (счёт, рекорд, скорость, уровень)
        res = true; // продолжаем таймер обновлений
    }
    return res; // возвращаем флаг продолжения или остановки таймера
}

void MyGtkWindow::show_game_over_dialog(const Glib::ustring &message) { // отображает модальный диалог с итоговым сообщением
    dialog = Gtk::AlertDialog::create(); // создаём экземпляр простого диалога
    dialog->set_message("Game over"); // заголовок диалога
    dialog->set_detail(message); // подробность с пользовательским сообщением (you win / you lose)
    dialog->set_default_button(-1); // убираем кнопку по умолчанию (нет)
    dialog->set_cancel_button(-1); // убираем кнопку отмены (нет)
    dialog->show(*this); // показываем диалог, привязанный к текущему окну
}

std::string MyGtkWindow::format_score(const int score) { // форматирует целочисленный счёт в строку длины 7, дополняя нулями слева
    std::string score_str = std::to_string(score); // преобразуем число в строку
    while (score_str.length() < 7) score_str = "0" + score_str; // дополняем ведущими нулями пока длина < 7
    return score_str; // возвращаем форматированную строку
}

void MyGtkWindow::info_update_game() { // обновляет метки информационной панели на основе current_state
    score_value_label.set_markup("<span font_desc='15'>" + format_score(current_state.score) + "</span>"); // обновляем отображение счёта
    hi_score_value_label.set_markup("<span font_desc='15'>" + format_score(current_state.high_score) + "</span>"); // обновляем отображение рекорда
    speed_value_label.set_markup("<span font_desc='15'>" + std::to_string(current_state.speed) + "</span>"); // обновляем отображение скорости
    level_value_label.set_markup("<span font_desc='15'>" + std::to_string(current_state.level) + "</span>"); // обновляем отображение уровня
}

void GameArea::on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height) { // рисование игрового поля через Cairo
    (void)width; (void)height; // явно игнорируем параметры width и height
    cr->scale(SCALE, SCALE); // масштабируем контекст, чтобы единица соответствовала одному блоку поля
    if (game_field != nullptr) { // если указатель на поле валиден
        for (int y = 0; y < WINDOW_HEIGHT; y++) { // проходим по всем строкам поля
            for (int x = 0; x < WINDOW_WIDTH; x++) { // проходим по всем столбцам поля
                if (game_field[y][x] != 0) { // если ячейка занята (ненулевое значение)
                    COLOR_RED_BLOCK(cr); // устанавливаем красный цвет в контексте рисования
                    cr->rectangle(x, y, 1, 1); // создаём прямоугольник один блок в масштабированных единицах
                    cr->fill(); // заливаем прямоугольник текущим цветом
                }
            }
        }
    }
}

void NextArea::on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height) { // рисование области NEXT через Cairo
    (void)width; (void)height; // явно игнорируем параметры width и height
    cr->scale(SCALE, SCALE); // масштабируем контекст для удобства рисования блоков
    if (next_field != nullptr) { // если указатель на матрицу next валиден
        for (int y = 0; y < NEXT_SIZE; y++) { // проходим по всем строкам матрицы next
            for (int x = NEXT_SHIFT; x < NEXT_SIZE; x++) { // проходим по столбцам, начиная с NEXT_SHIFT для центрирования
                if (next_field[y][x] != 0) { // если ячейка next занята
                    COLOR_RED_BLOCK(cr); // устанавливаем красный цвет для рисования блока
                    cr->rectangle(x - NEXT_SHIFT, y, 1, 1); // рисуем прямоугольник, сдвинутый на NEXT_SHIFT для выравнивания
                    cr->fill(); // заливаем прямоугольник текущим цветом
                }
            }
        }
    }
}
