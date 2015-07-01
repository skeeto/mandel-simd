#include <arm_neon.h>
#include "mandel.h"

void
mandel_neon(unsigned char *image, const struct spec *s)
{
    float32x4_t xmin = vdupq_n_f32(s->xlim[0]);
    float32x4_t ymin = vdupq_n_f32(s->ylim[0]);
    float32x4_t xscale = vdupq_n_f32((s->xlim[1] - s->xlim[0]) / s->width);
    float32x4_t yscale = vdupq_n_f32((s->ylim[1] - s->ylim[0]) / s->height);
    float32x4_t threshold = vdupq_n_f32(4);
    float32x4_t one = vdupq_n_f32(1);
    float32x4_t iter_scale = vdupq_n_f32(1.0f / s->iterations);
    float32x4_t depth_scale = vdupq_n_f32(s->depth - 1);
    float32x4_t c0123; // {0.0f, 1.0f, 2.0f, 3.0f}
    for (int i = 0; i < 4; i++)
        c0123 = vsetq_lane_f32(i, c0123, i);

    #pragma omp parallel for schedule(dynamic, 1)
    for (int y = 0; y < s->height; y++) {
        for (int x = 0; x < s->width; x += 4) {
            float32x4_t mx = vaddq_f32(vdupq_n_f32(x), c0123);
            float32x4_t my = vdupq_n_f32(y);
            float32x4_t cr = vaddq_f32(vmulq_f32(mx, xscale), xmin);
            float32x4_t ci = vaddq_f32(vmulq_f32(my, yscale), ymin);
            float32x4_t zr = cr;
            float32x4_t zi = ci;
            int k = 1;
            float32x4_t mk = vdupq_n_f32(k);
            while (++k < s->iterations) {
                /* Compute z1 from z0 */
                float32x4_t zr2 = vmulq_f32(zr, zr);
                float32x4_t zi2 = vmulq_f32(zi, zi);
                float32x4_t zrzi = vmulq_f32(zr, zi);
                /* zr1 = zr0 * zr0 - zi0 * zi0 + cr */
                /* zi1 = zr0 * zi0 + zr0 * zi0 + ci */
                zr = vaddq_f32(vsubq_f32(zr2, zi2), cr);
                zi = vaddq_f32(vaddq_f32(zrzi, zrzi), ci);

                /* Prepare to increment k */
                zr2 = vmulq_f32(zr, zr);
                zi2 = vmulq_f32(zi, zi);
                float32x4_t mag2 = vaddq_f32(zr2, zi2);
                uint32x4_t mask = vcltq_f32(mag2, threshold);

                /* Early bailout? */
                if (vgetq_lane_u32(mask, 0) == 0 &&
                    vgetq_lane_u32(mask, 1) == 0 &&
                    vgetq_lane_u32(mask, 2) == 0 &&
                    vgetq_lane_u32(mask, 3) == 0)
                    break;

                /* Increment k */
                uint32x4_t uone = vreinterpretq_u32_f32(one);
                float32x4_t inc = vreinterpretq_f32_u32(vandq_u32(mask, uone));
                mk = vaddq_f32(inc, mk);
            }
            mk = vmulq_f32(mk, iter_scale);
            mk = vrecpeq_f32(vrsqrteq_f32(mk)); // sqrt estimate
            mk = vmulq_f32(mk, depth_scale);
            uint32x4_t pixels = vcvtq_u32_f32(mk);
            unsigned char *dst = image + y * s->width * 3 + x * 3;
            unsigned char *src = (unsigned char *)&pixels;
            for (int i = 0; i < 4; i++) {
                dst[i * 3 + 0] = src[i * 4];
                dst[i * 3 + 1] = src[i * 4];
                dst[i * 3 + 2] = src[i * 4];
            }
        }
    }
}
