BIN=ficlib2
CC=gcc -O2 -g
#CC=gcc -O0 -g

all:
	make $(BIN)

$(BIN):
	$(CC) ficlib2.c -o $(BIN)

run: $(BIN)
	rm -rf /tmp/gpio.lock && ./$(BIN)

debug:
	gdb ./$(BIN)

rawrpigpio.so: rawrpigpio_lib.c
	gcc -O2 $< -shared -I/usr/include/python3.5 -o $@

clean:
	rm -rf $(BIN)

