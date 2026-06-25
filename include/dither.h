#ifndef DITHER_H
#define DITHER_H

#include <stddef.h>
#include <stdint.h>

// Palette: flat array of RGB colours (rows x cols)
typedef struct {
  unsigned int rows;
  unsigned int cols;
  unsigned int *colors; // flat, row‑major: colors[r*cols + c]
} Palette;

// Dithering parameters
typedef struct {
  int coef;    // error diffusion denominator
  int kernelX; // horizontal offset to centre of kernel
  int kernelHeight;
  int kernelWidth;
  const unsigned char *kernel; // flattened kernel (row‑major)
  Palette palette;
  int (*paletteRowFunc)(int i, int j, int rows); // row selector
} DitherParams;

/* Main dithering function.
   - input: RGBA image data (4 bytes per pixel)
   - output: will be filled with RGBA data
   - indexArray: if not NULL, filled with the chosen palette index (0..cols-1)
                 for each pixel (width*height bytes)
*/
int dither_image(const unsigned char *input, unsigned int width,
                 unsigned int height, const DitherParams *params,
                 unsigned char *output, unsigned char *indexArray);

// Initialises DitherParams with Stucki kernel and cyclic row selector
void dither_default_params(DitherParams *params, const Palette *pal);

// Example palette row selector: cyclic 3x3 pattern
int dither_row_cyclic(int i, int j, int rows);

#endif
