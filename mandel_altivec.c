#include <altivec.h>
#include "mandel.h"

void
mandel_altivec(unsigned char *image, const struct spec *s)
{
    vector float xmin, ymin, xscale, yscale, iter_scale, depth_scale;
    vector float threshold = (vector float) { 4.0, 4.0, 4.0, 4.0 };
    vector float one = (vector float) { 1.0, 1.0, 1.0, 1.0 };
    vector float zero = (vector float) { 0.0, 0.0, 0.0, 0.0 };
    
    xmin = (vector float) { s->xlim[0], s->xlim[0], s->xlim[0], s->xlim[0] };
    ymin = (vector float) { s->ylim[0], s->ylim[0], s->ylim[0], s->ylim[0] };
    xscale = (vector float) { (s->xlim[1] - s->xlim[0]) / s->width,
        (s->xlim[1] - s->xlim[0]) / s->width,
        (s->xlim[1] - s->xlim[0]) / s->width,
        (s->xlim[1] - s->xlim[0]) / s->width };
    yscale = (vector float) { (s->ylim[1] - s->ylim[0]) / s->height,
        (s->ylim[1] - s->ylim[0]) / s->height,
        (s->ylim[1] - s->ylim[0]) / s->height,
        (s->ylim[1] - s->ylim[0]) / s->height };
    iter_scale = (vector float) { 1.0f / s->iterations,
        1.0f / s->iterations,
        1.0f / s->iterations,
        1.0f / s->iterations };
    depth_scale = (vector float) { s->depth - 1, s->depth - 1,
        s->depth - 1, s->depth - 1 };

    #pragma omp parallel for schedule(dynamic, 1)
    for (int y = 0; y < s->height; y++) {
        for (int x = 0; x < s->width; x += 4) {
            vector float mx = (vector float)
                { x, x + 1, x + 2, x + 3 };
            vector float my = (vector float) 
                { y, y, y, y };
            vector float cr = vec_madd(mx, xscale, xmin);
            vector float ci = vec_madd(my, yscale, ymin);
            vector float zr = cr;
            vector float zi = ci;

            int k = 1;
            vector float mk = (vector float) { 1, 1, 1, 1 };
            while (++k < s->iterations) {
                /* Compute z1 from z0 */
                vector float zr2 = vec_madd(zr, zr, zero);
                vector float zi2 = vec_madd(zi, zi, zero);
                vector float zrzi = vec_madd(zr, zi, zero);

                /* zr1 = zr0 * zr0 - zi0 * zi0 + cr */
                /* zi1 = zr0 * zi0 + zr0 * zi0 + ci */
                zr = vec_add(vec_sub(zr2, zi2), cr);
                zi = vec_add(vec_add(zrzi, zrzi), ci);

                /* Increment k */
                zr2 = vec_madd(zr, zr, zero);
                zi2 = vec_madd(zi, zi, zero);
                vector float mag2 = vec_add(zr2, zi2);
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
