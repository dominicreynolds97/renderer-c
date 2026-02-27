CC = clang
CFLAGS = -Wall -Wextra -std=c11 $(shell pkg-config --cflags glfw3) -I/opt/homebrew/include
LIBS = $(shell pkg-config --libs glfw3) -lGLEW -framework OpenGL -L/opt/homebrew/lib

TARGET = renderer
SRC = main.c $(wildcard src/**/*.c)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)
