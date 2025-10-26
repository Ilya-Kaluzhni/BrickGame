#include "gtk_frontend.h" // подключает заголовок с объявлением MyGtkWindow и виджетов GTK фронтенда

int main(int argc, char **argv) { // точка входа для GTK-приложения с передачей аргументов командной строки
  auto app = Gtk::Application::create("org.gtkmm.examples.base"); // создаёт экземпляр приложения GTKmm с уникальным ID
  return app->make_window_and_run<MyGtkWindow>(argc, argv); // создаёт окно типа MyGtkWindow, запускает главный цикл приложения и возвращает код выхода
} // конец main
