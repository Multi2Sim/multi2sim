TEST_NAME=vector_copy
LFLAGS= -g  -Wl,--unresolved-symbols=ignore-in-shared-libs
INCS = -I ../../../runtime/include
C_FILES := $(wildcard *.c)
OBJ_FILES := $(addprefix obj/, $(notdir $(C_FILES:.c=.o)))

all: $(TEST_NAME) $(TEST_NAME).brig

$(TEST_NAME): $(OBJ_FILES) $(COMMON_OBJ_FILES)
	$(CC) -m32 $(LFLAGS) $(OBJ_FILES) \
		-L../../../lib/.libs -lm2s-hsa \
		-o $(TEST_NAME) -static

$(TEST_NAME).brig :
	cloc.sh $(TEST_NAME).cl

obj/%.o: %.c
	$(CC) -m32 -c $(CFLAGS) $(INCS) -o $@ $< -std=c99

clean:
	rm -rf obj/*o *.brig $(TEST_NAME)



