#include <altivec.h>
#include "mandel.h"

/*
 * There's no one-instruction splat for float, although
 * you can splat into a float from an element in another
 * vector. Given that these are done outside of the innermost
 * loop it might not be worth the effort.
 */
#define VF_ALL(x) ((vector float) { x, x, x, x })

void
mandel_altivec(unsigned char *image, const struct spec *s)
{
    vector float xmin, ymin, xscale, yscale, iter_scale, depth_scale;
    vector float threshold = VF_ALL(4.0);
    vector float one = VF_ALL(1.0);
    vector float zero = VF_ALL(0.0);

    xmin = VF_ALL(s->xlim[0]);
    ymin = VF_ALL(s->ylim[0]);
    xscale = VF_ALL((s->xlim[1] - s->xlim[0]) / s->width);
    yscale = VF_ALL((s->ylim[1] - s->ylim[0]) / s->height);
    iter_scale = VF_ALL(1.0f / s->iterations);
    depth_scale = VF_ALL(s->depth - 1);

    #pragma omp parallel for schedule(dynamic, 1)
    for (int y = 0; y < s->height; y++) {
        for (int x = 0; x < s->width; x += 4) {
            vector float mx = (vector float) { x, x + 1, x + 2, x + 3 };
            vector float my = VF_ALL(y);
            vector float cr = vec_madd(mx, xscale, xmin);
            vector float ci = vec_madd(my, yscale, ymin);
            vector float zr = cr;
            vector float zi = ci;

            int k = 1;
            vector float mk = VF_ALL(1);
            while (++k < s->iterations) {
                /* Compute z1 from z0 */
                vector float zr2cr = vec_madd(zr, zr, cr);
                vector float zi2 = vec_madd(zi, zi, zero);
                vector float zrzi = vec_madd(zr, zi, zero);

                /* zr1 = zr0 * zr0 - zi0 * zi0 + cr */
                /* zi1 = zr0 * zi0 + zr0 * zi0 + ci */
                zr = vec_sub(zr2cr, zi2);
                zi = vec_add(vec_add(zrzi, zrzi), ci);

                /* Increment k */
                vector float zr2 = vec_madd(zr, zr, zero);
                vector float mag2 = vec_madd(zi, zi, zr2);
		vector bool int mask = vec_cmplt(mag2, threshold);
                mk = vec_add(mk, vec_and(one, mask));

                if(vec_all_ge(mag2, threshold))
                    break;
            }

            mk = vec_madd(mk, iter_scale, zero);
            mk = vec_madd(vec_rsqrte(mk), mk, zero);
            mk = vec_madd(mk, depth_scale, zero);

            vector int pixels = vec_cts(mk, 0);

            unsigned char *dst = image + y * s->width * 3 + x * 3;
            unsigned char *src = (unsigned char *)&pixels;

            for (int i = 0; i < 4; i++) {
                dst[i * 3 + 0] = src[(i * 4) + 3];
                dst[i * 3 + 1] = src[(i * 4) + 3];
                dst[i * 3 + 2] = src[(i * 4) + 3];
            }
        }
    }
}
