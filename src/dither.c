#include "dither.h"
#include <stdlib.h>
#include <string.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

/* Kernel definitions */

// Stucki (default)
static const unsigned char stucki_kernel[3][5] = {
    {0, 0, 0, 8, 4}, {2, 4, 8, 4, 2}, {1, 2, 4, 2, 1}};

// Floyd‑Steinberg
// static const unsigned char fs_kernel[3][4] = {{0, 0, 7}, {3, 5, 1}, {0, 0, 0}};
// For simplicity, implemented Stucki in default
// TODO: add more kernel

int dither_row_cyclic(int i, int j, int rows) {
  (void)rows;
  return (i % 3 + 2 - j % 3) % 3;
}

void dither_default_params(DitherParams *params, const Palette *pal) {
  params->coef = 42;
  params->kernelX = 2;
  params->kernelHeight = 3;
  params->kernelWidth = 5;
  // flatten the 2D kernel into a 1D array
  static unsigned char flat_kernel[3 * 5];
  memcpy(flat_kernel, stucki_kernel, sizeof(stucki_kernel));
  params->kernel = flat_kernel;
  params->palette = *pal;
  params->paletteRowFunc = dither_row_cyclic;
}

// Helper to get palette colour at (row, col)
static inline unsigned int get_palette_color(const Palette *pal, int row,
                                             int col) {
  return pal->colors[row * pal->cols + col];
}

int dither_image(const unsigned char *input, unsigned int width,
                 unsigned int height, const DitherParams *params,
                 unsigned char *output, unsigned char *indexArray) {

  const int K_H = params->kernelHeight;
  const int K_W = params->kernelWidth;
  const int K_X = params->kernelX;
  const int coef = params->coef;
  const unsigned char *kernel = params->kernel;
  const Palette *pal = &params->palette;
  int (*rowFunc)(int, int, int) = params->paletteRowFunc;

  // allocate error buffers - using flat arrays for simplicity
  int *rBuf = (int *)calloc(K_H * width, sizeof(int));
  int *gBuf = (int *)calloc(K_H * width, sizeof(int));
  int *bBuf = (int *)calloc(K_H * width, sizeof(int));
  if (!rBuf || !gBuf || !bBuf) {
    free(rBuf);
    free(gBuf);
    free(bBuf);
    return 1;
  }

  const unsigned char *ptr = input;
  unsigned char *ptrOut = output;
  unsigned char *idxOut = indexArray;

  for (unsigned int i = 0; i < height; ++i) {
    int row_i = i % K_H;
    for (unsigned int j = 0; j < width; ++j) {
      int idx = row_i * width + j;

      int r = *ptr++ + rBuf[idx];
      int g = *ptr++ + gBuf[idx];
      int b = *ptr++ + bBuf[idx];
      ptr++; /* skip alpha */

      rBuf[idx] = gBuf[idx] = bBuf[idx] = 0;

      r = MAX(0, MIN(255, r));
      g = MAX(0, MIN(255, g));
      b = MAX(0, MIN(255, b));

      // choose palette row
      int pal_row = rowFunc(i, j, pal->rows);
      int best_col = 0;
      int best_dist = 1 << 30;

      for (unsigned int k = 0; k < pal->cols; ++k) {
        unsigned int col = get_palette_color(pal, pal_row, k);
        int rp = (col >> 16) & 0xFF;
        int gp = (col >> 8) & 0xFF;
        int bp = (col >> 0) & 0xFF;

        int dr = rp - r;
        int dg = gp - g;
        int db = bp - b;
        int d = dr * dr + dg * dg + db * db;
        if (d < best_dist) {
          best_dist = d;
          best_col = k;
        }
      }

      unsigned int chosen = get_palette_color(pal, pal_row, best_col);
      *ptrOut++ = (chosen >> 16) & 0xFF;
      *ptrOut++ = (chosen >> 8) & 0xFF;
      *ptrOut++ = (chosen >> 0) & 0xFF;
      *ptrOut++ = 0xFF;

      int rErr = r - (int)((chosen >> 16) & 0xFF);
      int gErr = g - (int)((chosen >> 8) & 0xFF);
      int bErr = b - (int)((chosen >> 0) & 0xFF);

      // store index if requested
      if (indexArray)
        *idxOut++ = (unsigned char)(7 - best_col);

      // diffuse error
      for (int k = 0; k < K_H; ++k) {
        int buf_row = (i + k) % K_H;
        for (int l = -K_X; l < K_W - K_X; ++l) {
          int col = (int)j + l;
          if (col < 0 || col >= (int)width)
            continue;
          int weight = kernel[k * K_W + (l + K_X)];
          if (weight == 0)
            continue;
          int idx2 = buf_row * width + col;
          rBuf[idx2] += (weight * rErr) / coef;
          gBuf[idx2] += (weight * gErr) / coef;
          bBuf[idx2] += (weight * bErr) / coef;
        }
      }
    }
  }

  free(rBuf);
  free(gBuf);
  free(bBuf);
  return 0;
}
