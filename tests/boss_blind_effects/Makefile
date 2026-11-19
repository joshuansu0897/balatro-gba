
CC := gcc
# WHY: the real blind.h needs GBA headers. The one of this folder is included first and has the same
# include guard, which makes the compiler skip the real one.
CFLAGS := -include blind.h -I../../include -I. \
          -g -O3 -std=gnu23 -Wall -Werror
SRC            := boss_blind_effects_test.c         \
                  ../../source/boss_blind_effects.c
OUT            := build/boss_blind_effects_test

$(OUT): $(SRC) | build
	$(CC) $(CFLAGS) -o $@ $^

build:
	mkdir -p build

clean:
	rm -f $(OUT)
