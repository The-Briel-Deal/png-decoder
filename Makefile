CFLAGS  := -std=gnu99 -Iinclude -Wall -Werror -lm 
HEADERS := $(wildcard include/*.h)
SOURCES := $(wildcard src/*.c)
OBJECTS := $(addprefix build/, $(notdir $(SOURCES:.c=.o)))

TEST_HEADERS := $(HEADERS) $(wildcard test/include/*.h)
TEST_SOURCES := $(wildcard test/src/*.c)
TEST_OBJECTS := $(filter-out %/main.o, $(OBJECTS)) $(addprefix build/test/, $(notdir $(TEST_SOURCES:.c=.o)))

ifdef LOG_LEVEL
	CFLAGS += -DLOG_LEVEL=$(LOG_LEVEL)
endif
ifeq (${DEBUG_SYM}, 1)
	CFLAGS += -g
endif

TEST_CFLAGS := $(CFLAGS) -Itest/include

build/:
	mkdir -p build

# Compile Object Files
build/%.o: src/%.c $(HEADERS) | build/
	gcc $< -c $(CFLAGS) -o $@

png_decoder: $(OBJECTS)
	gcc $^ $(CFLAGS) -o ./build/$@

run: png_decoder
	./build/png_decoder

# Test
test: build/test/test
build/test/test: $(TEST_OBJECTS)
	gcc $^ $(TEST_CFLAGS) -o build/test/test

# Compile Object Files
build/test/%.o: test/src/%.c $(TEST_HEADERS) | build/ build/test/
	gcc $< -c $(TEST_CFLAGS) -o $@

build/test/:
	mkdir -p build/test

run-test: build/test/test
	exec $<
