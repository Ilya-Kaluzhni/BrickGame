#ifndef GTK_FRONTEND_H // защита от повторного включения: если макрос не определён
#define GTK_FRONTEND_H // определяет макрос, чтобы предотвратить повторное включение

#include <gtkmm.h> // подключает заголовки библиотеки GTKmm для создания GUI на C++
#include "../../brick_game//brick_game_single.h" // подключает общий заголовок с определениями игры и константами

#define Tetris 1 // макроопределение кода игры Tetris (используется в API выбора игры)
#define Snake 2 // макроопределение кода игры Snake

class GameArea : public Gtk::DrawingArea { // класс виджета для отрисовки основного игрового поля, наследует Gtk::DrawingArea
 public:
  int **game_field; // указатель на матрицу игрового поля, которую виджет отрисовывает

  GameArea() : game_field(nullptr) { // конструктор, инициализирует game_field нулевым указателем
    set_draw_func(sigc::mem_fun(*this, &GameArea::on_draw)); // устанавливает callback-функцию отрисовки on_draw
  } // конец конструктора

  void on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height); // прототип метода отрисовки поля
}; // конец объявления класса GameArea

class NextArea : public Gtk::DrawingArea { // класс виджета для отрисовки области "NEXT", наследует Gtk::DrawingArea
 public:
  int **next_field; // указатель на матрицу следующей фигуры для отрисовки

  NextArea() : next_field(nullptr) { // конструктор, инициализирует next_field нулевым указателем
    set_draw_func(sigc::mem_fun(*this, &NextArea::on_draw)); // устанавливает callback-функцию отрисовки on_draw
  } // конец конструктора
  void on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height); // прототип метода отрисовки next области
}; // конец объявления класса NextArea

class MyGtkWindow : public Gtk::Window { // главный класс окна приложения, наследует Gtk::Window
 public:
  MyGtkWindow(); // конструктор окна, задаёт интерфейс и события

 private:
  Glib::RefPtr<Gtk::AlertDialog> dialog; // умный указатель на модальный диалог для оповещений об окончании игры

  Gtk::Box main_box; // главный вертикальный контейнер для всех виджетов
  Gtk::Box button_box; // контейнер для текстовых подсказок кнопок управления
  Gtk::Box start_box; // контейнер для стартового экрана с кнопками выбора игры
  Gtk::Box info_box; // контейнер для информационной панели (счёт, уровень, next)

  GameArea *game_area; // указатель на виджет отрисовки основного поля
  NextArea *next_area; // указатель на виджет отрисовки области NEXT

  sigc::connection timer_connection; // соединение для периодического таймера обновления (Glib timeout)

  Gtk::Button tetris_button; // кнопка выбора Tetris
  Gtk::Button snake_button; // кнопка выбора Snake
  Gtk::Button exit_button; // кнопка выхода из приложения

  GameInfo_t current_state; // структура с текущим состоянием игры, используемая интерфейсом

  Gtk::Label start_label; // метка подсказки START
  Gtk::Label quit_label; // метка подсказки QUIT
  Gtk::Label pause_label; // метка подсказки PAUSE
  Gtk::Label action_label; // метка подсказки ACTION

  Gtk::Label score_label; // текстовая метка "SCORE"
  Gtk::Label score_value_label; // метка для отображения текущего счёта
  Gtk::Label hi_score_label; // текстовая метка "HI-SCORE"
  Gtk::Label hi_score_value_label; // метка для отображения рекорда
  Gtk::Label next_label; // текстовая метка "NEXT"
  Gtk::Label speed_label; // текстовая метка "SPEED"
  Gtk::Label speed_value_label; // метка для отображения текущей скорости
  Gtk::Label level_label; // текстовая метка "LEVEL"
  Gtk::Label level_value_label; // метка для отображения текущего уровня

 protected:
  std::string format_score(const int score); // форматирует целочисленный счёт в строку с ведущими нулями
  void info_update_game(); // обновляет текстовые метки информационной панели по current_state
  bool update_game(); // один шаг обновления игры, вызывается таймером; возвращает true для продолжения таймера
  void show_game_over_dialog(const Glib::ustring &message); // показывает диалог завершения игры с детальным сообщением
  bool key_press(guint16 keyval, guint, Gdk::ModifierType state); // обработчик событий клавиатуры для окна
  void start_game(); // переключает интерфейс в режим игры и запускает обновления / события

  void setup_button_labels(); // настраивает и добавляет текстовые подписи кнопок управления
  void setup_info_box(Gtk::Frame *next_area_frame); // конфигурирует информационную панель и добавляет в неё NEXT рамку
  void setup_game_area_frame(Gtk::Frame *game_area_frame); // настраивает рамку игрового поля (размеры, отступы, стиль)

  void clicked_button_tetris(); // callback при клике кнопки Tetris — выбирает Tetris и стартует игру
  void clicked_button_snake(); // callback при клике кнопки Snake — выбирает Snake и стартует игру
  void clicked_button_exit(); // callback при клике Exit — закрывает окно
}; // конец объявления класса MyGtkWindow

#endif  // GTK_FRONTEND_H // закрывает защиту от повторного включения