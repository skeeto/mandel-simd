CFLAGS = -std=c99 -Wall -Wextra -Ofast -fopenmp

mandel.ppc : mandel.c mandel_altivec.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

mandel.x86 : mandel.c mandel_avx.o mandel_sse2.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

mandel.arm : mandel.c mandel_neon.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

mandel_altivec.o : mandel_altivec.c
	$(CC) -c $(CFLAGS) -maltivec -mabi=altivec -o $@ $^

mandel_avx.o : mandel_avx.c
	$(CC) -c $(CFLAGS) -mavx -o $@ $^

mandel_sse2.o : mandel_sse2.c
	$(CC) -c $(CFLAGS) -msse2 -o $@ $^

mandel_neon.o : mandel_neon.c
	$(CC) -c $(CFLAGS) -mfpu=neon -o $@ $^

clean :
	$(RM) mandel.x86 mandel.arm mandel.ppc mandel_altivec.o mandel_avx.o mandel_sse2.o mandel_neon.o

.PHONY : clean
