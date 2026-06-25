#ifndef DITHER_H
#define DITHER_H

#include <stddef.h>
#include <stdint.h>

// Kernel types
typedef enum {
  KERNEL_FLOYD_STEINBERG,
  KERNEL_JARVIS,
  KERNEL_ATKINSON,
  KERNEL_BURKES,
  KERNEL_STUCKI,
  KERNEL_SIERRA_LITE
} KernelType;

// Palette: flat array of RGB colours (rows x cols)
typedef struct {
  unsigned int rows;
  unsigned int cols;
  unsigned int *colors;
} Palette;

// Dithering parameters
typedef struct {
  int coef;
  int kernelX;
  int kernelHeight;
  int kernelWidth;
  const unsigned char *kernel; // flattened row‑major
  Palette palette;
  int (*paletteRowFunc)(int i, int j, int rows);
} DitherParams;

// Initialise parameters with given kernel type and palette
void dither_init_params(DitherParams *params, const Palette *pal,
                        KernelType kt);

// Main dithering function
int dither_image(const unsigned char *input, unsigned int width,
                 unsigned int height, const DitherParams *params,
                 unsigned char *output, unsigned char *indexArray);

// Default row selector (cyclic 3x3)
int dither_row_cyclic(int i, int j, int rows);

#endif
