CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -Wextra -lm -g 

all: render_tone render_song render_echo

render_tone: render_tone.o io.o wave.o
	$(CC) -o render_tone render_tone.o io.o wave.o -lm

render_song: render_song.o io.o wave.o 
	$(CC) -o render_song render_song.o io.o wave.o -lm

render_echo: render_echo.o io.o wave.o
	$(CC) -o render_echo render_echo.o io.o wave.o -lm

render_tone.o: render_tone.c wave.h io.h
	$(CC) $(CFLAGS) -c render_tone.c

wave.o: wave.c wave.h io.h
	$(CC) $(CFLAGS) -c wave.c 

io.o: io.c io.h 
	$(CC) $(CFLAGS) -c io.c

render_song.o: render_song.c wave.h io.h 
	$(CC) -c render_song.c $(CFLAGS)

render_echo.o: render_echo.c wave.h io.h
	$(CC) -c render_echo.c $(CFLAGS)

clean:
	rm -f *.o *.wav render_tone render_echo render_song