#define _POSIX_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <tmmintrin.h>

int
main(void)
{
    int width = 1440;
    int height = 1080;
    float xlim[] = {-2.5, 1.5};
    float ylim[] = {-1.5, 1.5};

    __m128 xmin = _mm_set_ps1(xlim[0]);
    __m128 xscale = _mm_set_ps1((xlim[1] - xlim[0]) / width);
    float yscale = (ylim[1] - ylim[0]) / height;
    __m128 thresh = _mm_set_ps1(4);
    __m128 one = _mm_set_ps1(1);
    __m128i zero = _mm_setzero_si128();
    __m128i pixel_pack =
        _mm_set_epi8(15, 15, 15, 15, 12, 12, 12, 8, 8, 8, 4, 4, 4, 0, 0, 0);

    printf("P6\n%d %d\n255\n", width, height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x += 4) {
            __m128 mx = _mm_set_ps(x + 3, x + 2, x + 1, x + 0);
            __m128 cr = _mm_add_ps(_mm_mul_ps(mx, xscale), xmin);
            __m128 ci = _mm_set_ps1(y * yscale + ylim[0]);
            __m128 zr = cr;
            __m128 zi = ci;
            int k = 1;
            __m128 mk = _mm_set_ps1(k);
            while (++k < 256) {
                /* Compute z1 from z0 */
                __m128 zr2 = _mm_mul_ps(zr, zr);
                __m128 zi2 = _mm_mul_ps(zi, zi);
                __m128 zrzi = _mm_mul_ps(zr, zi);
                /* zr1 = zr0 * zr0 - zi0 * zi0 + cr */
                /* zi1 = zr0 * zi0 + zr0 * zi0 + ci */
                zr = _mm_add_ps(_mm_sub_ps(zr2, zi2), cr);
                zi = _mm_add_ps(_mm_add_ps(zrzi, zrzi), ci);

                /* Increment k */
                zr2 = _mm_mul_ps(zr, zr);
                zi2 = _mm_mul_ps(zi, zi);
                __m128 mag2 = _mm_add_ps(zr2, zi2);
                __m128 mask = _mm_cmplt_ps(mag2, thresh);
                mk = _mm_add_ps(_mm_and_ps(mask, one), mk);

                /* Early bailout? */
                __m128i maski = _mm_castps_si128(mask);
                if (0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(maski, zero)))
                    break;
            }
            uint8_t ks[128];
            __m128i pixels = _mm_shuffle_epi8(_mm_cvtps_epi32(mk), pixel_pack);
            _mm_store_si128((void *)ks, pixels);
            fwrite(ks, 3, 4, stdout);
        }
    }

    return 0;
}
