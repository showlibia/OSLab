CC=gcc
CFLAGS=-Wall -Werror -O2
TARGET=pstree
SOURCES=pstree.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	@$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	@rm -f $(TARGET)
