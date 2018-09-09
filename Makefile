BIN=ficlib2
CC=gcc -O2 -g
#CC=gcc -O0 -g

all:
	make $(BIN)
	make pyficlib2.so

$(BIN):
	$(CC) ficlib2.c -o $(BIN)

run: $(BIN)
	rm -rf /tmp/gpio.lock && ./$(BIN)

debug:
	gdb ./$(BIN)

pyficlib2.so: pyficlib2.c ficlib2.o
	$(CC) $< ficlib2.o -shared -I/usr/include/python3.5 -o $@

clean:
	rm -rf $(BIN)
	rm -rf pyficlib2.so
	rm -rf /tmp/gpio.lock

