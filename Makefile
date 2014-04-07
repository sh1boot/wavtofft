CFLAGS=-O3 -pedantic -std=c99 -Wall -ffast-math -lm -lsndfile -lfftw3
TARGET=wavtofft

$(TARGET): $(TARGET).c
	$(CC) $< $(CFLAGS) -o $@

clean:
	rm -f $(TARGET) *.o
