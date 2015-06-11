#include <stdio.h>

int
main(void)
{
    int width = 1440;
    int height = 1080;
    float xlim[] = {-2.5, 1.5};
    float ylim[] = {-1.5, 1.5};

    float xdiff = xlim[1] - xlim[0];
    float ydiff = ylim[1] - ylim[0];

    printf("P6\n%d %d\n255\n", width, height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float cr = x * xdiff / width  + xlim[0];
            float ci = y * ydiff / height + ylim[0];
            float zr = cr;
            float zi = ci;
            int k = 0;
            while (++k < 255) {
                float zr1 = zr * zr - zi * zi + cr;
                float zi1 = zr * zi + zr * zi + ci;
                zr = zr1;
                zi = zi1;
                if (zr * zr + zi * zi >= 4.0f)
                    break;
            }
            putchar(k);
            putchar(k);
            putchar(k);
        }
    }

    return 0;
}
