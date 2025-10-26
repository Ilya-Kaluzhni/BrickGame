CC := g++
CFLAGS := -std=c++17 -Wall -Werror -Wextra

OS := $(shell uname)

ifeq ($(OS), Linux)
	TESTFLAGS += -lgtest -lgtest_main -lpthread
	COVFLAGS := --coverage
endif

ifeq ($(OS), Darwin)
	GTEST_DIR := /opt/homebrew/Cellar/googletest/$(shell ls /opt/homebrew/Cellar/googletest | tail -1)
	TESTFLAGS += -I$(GTEST_DIR)/include -L$(GTEST_DIR)/lib -lgtest -lgtest_main -lpthread
	COVFLAGS := -fprofile-arcs -ftest-coverage
endif

GAME_DIR := brick_game
GAME_OBJS := $(GAME_DIR)/tetris/tetris.o \
             $(GAME_DIR)/snake/snake.o \
             $(GAME_DIR)/brick_game_single.o

GAME_LIB := libs21_game.a

TEST_DIR := tests
TEST_EXEC := test_runner
TEST_SRC := $(wildcard $(TEST_DIR)/*.cpp)

FRONTED_CPP := gui/cli/frontend.cpp
GTK_CPP := gui/desktop/gtk_frontend.cpp gui/desktop/gtk_main.cpp

BUILD_DIR := build
CLI_EXEC := BrickGameCli
DESKTOP_EXEC := BrickGameDesktop

GTKMM_FLAGS := $(shell pkg-config gtkmm-4.0 --cflags --libs)

REPORT_DIR := report

all: install

$(GAME_LIB): $(GAME_OBJS)
	ar rcs $@ $^

$(CLI_EXEC): $(FRONTED_CPP) $(GAME_LIB)
	$(CC) $(CFLAGS) -o $@ $^ -lncurses -lm

$(DESKTOP_EXEC): $(GTK_CPP) $(GAME_LIB)
	$(CC) $(CFLAGS) -o $@ $^ $(GTKMM_FLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

install: $(CLI_EXEC) $(DESKTOP_EXEC)
	mkdir -p $(BUILD_DIR)
	mv $(CLI_EXEC) $(DESKTOP_EXEC) $(BUILD_DIR)/
	@echo "Игры установлены в $(BUILD_DIR)/"

uninstall:
	@echo "Удаление..."
	-rm -rf $(BUILD_DIR)
	@echo "Удалено."

test: $(GAME_LIB)
	$(CC) $(CFLAGS) -DUNIT_TEST -o $(TEST_EXEC) $(TEST_SRC) $(GAME_LIB) $(TESTFLAGS)
	./$(TEST_EXEC)

ifeq ($(OS), Darwin)
leaks_test: test
	@echo "Запуск тестов под leaks..."
	@leaks --atExit -- ./$(TEST_EXEC)
endif

gcov_report: clean
	$(CC) $(CFLAGS) $(COVFLAGS) -o $(TEST_EXEC) $(TEST_SRC) \
	$(GAME_DIR)/tetris/tetris.cpp \
	$(GAME_DIR)/snake/snake.cpp \
	$(GAME_DIR)/brick_game_single.cpp \
	$(TESTFLAGS)
	./$(TEST_EXEC)
	@echo "Генерация отчёта покрытия..."
	@lcov -t "coverage" -o coverage.info -c -d . --no-external --ignore-errors mismatch,format
	@lcov --remove coverage.info 'tests/*' -o coverage.info
	@genhtml -o $(REPORT_DIR) coverage.info
	@echo "Отчёт создан: $(REPORT_DIR)/index.html"

dvi:
	@echo "Генерация документации Doxygen..."
	@mkdir -p doc
	@if [ ! -f doc/Doxyfile ]; then doxygen -g doc/Doxyfile; fi
	@sed -i '' 's|^OUTPUT_DIRECTORY.*|OUTPUT_DIRECTORY = doc|' doc/Doxyfile
	@sed -i '' 's|^INPUT .*|INPUT = ./|' doc/Doxyfile
	@sed -i '' 's|^RECURSIVE.*|RECURSIVE = YES|' doc/Doxyfile
	@sed -i '' 's|^GENERATE_LATEX.*|GENERATE_LATEX = NO|' doc/Doxyfile
	@sed -i '' 's|^PROJECT_NAME.*|PROJECT_NAME = "S21 Brick Game"|' doc/Doxyfile
	@doxygen doc/Doxyfile > /dev/null
	@echo "Документация создана: doc/html/index.html"

dist:
	@echo "Создание архива проекта..."
	@mkdir -p dist
	@tar -czf dist/s21_brick_game.tar.gz \
		Makefile $(GAME_DIR) gui $(TEST_DIR)
	@echo "Архив создан: dist/s21_brick_game.tar.gz"

clean:
	find . -name "*.o" -type f -delete
	-rm -f *.a $(CLI_EXEC) $(DESKTOP_EXEC) $(TEST_EXEC)
	-rm -rf $(BUILD_DIR) $(REPORT_DIR) *.gcda *.gcno *.info doc dist
	@echo "Очистка завершена."
