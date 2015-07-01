CFLAGS = -std=c99 -Wall -Wextra -Ofast -fopenmp

mandel.arm : mandel.c mandel_neon.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

mandel : mandel.c mandel_avx.o mandel_sse2.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

mandel_sse2.o : mandel_sse2.c
	$(CC) -c $(CFLAGS) -msse2 -o $@ $^

mandel_avx.o : mandel_avx.c
	$(CC) -c $(CFLAGS) -mavx -o $@ $^

mandel_neon.o : mandel_neon.c
	$(CC) -c $(CFLAGS) -mfpu=neon -o $@ $^

clean :
	$(RM) mandel mandel_sse2.o mandel_avx.o mandel_neon.o mandel.arm

.PHONY : clean

run : mandel.arm
	./$^ > out.ppm
