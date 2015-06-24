CFLAGS = -std=c99 -Wall -Wextra -Ofast -fopenmp

mandel : mandel.c mandel_avx.o mandel_sse2.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

mandel_sse2.o : mandel_sse2.c
	$(CC) -c $(CFLAGS) -msse2 -o $@ $^

mandel_avx.o : mandel_avx.c
	$(CC) -c $(CFLAGS) -mavx -o $@ $^

clean :
	$(RM) mandel mandel_sse2.o mandel_avx.o

.PHONY : clean
