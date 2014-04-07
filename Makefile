CFLAGS=-march=armv7-a -mfpu=neon -O3 -pedantic -std=c99 -Wall -ffast-math
LDFLAGS=-lm -lsndfile -lfftw3
TARGET=wavtofft

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -f $(TARGET) *.o
