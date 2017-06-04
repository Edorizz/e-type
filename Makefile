CC := gcc
C_FILES := $(wildcard src/*.c)
OBJ_FILES := $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))
CFLAGS := -c -std=c89 -Wall -pedantic
LDFLAGS := -lncurses
RM := rm -f
NAME := e-type

$(NAME): $(OBJ_FILES)
	$(CC) $(LDFLAGS) -O3 -fomit-frame-pointer -o $@ $^

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) obj/*.o $(NAME)

