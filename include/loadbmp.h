#ifndef LOADBMP_H
#define LOADBMP_H

#ifdef __cplusplus
extern "C" {
#endif

#define LOADBMP_NO_ERROR 0
#define LOADBMP_OUT_OF_MEMORY 1
#define LOADBMP_FILE_NOT_FOUND 2
#define LOADBMP_FILE_OPERATION 3
#define LOADBMP_INVALID_FILE_FORMAT 4
#define LOADBMP_INVALID_SIGNATURE 5
#define LOADBMP_INVALID_BITS_PER_PIXEL 6

#define LOADBMP_RGB 3
#define LOADBMP_RGBA 4

#ifdef LOADBMP_IMPLEMENTATION
#define LOADBMP_API
#else
#define LOADBMP_API extern
#endif

LOADBMP_API unsigned int loadbmp_decode_file(const char *filename,
                                             unsigned char **imageData,
                                             unsigned int *width,
                                             unsigned int *height,
                                             unsigned int components);

LOADBMP_API unsigned int loadbmp_encode_file(const char *filename,
                                             const unsigned char *imageData,
                                             unsigned int width,
                                             unsigned int height,
                                             unsigned int components);

#ifdef LOADBMP_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LOADBMP_API unsigned int loadbmp_decode_file(const char *filename,
                                             unsigned char **imageData,
                                             unsigned int *width,
                                             unsigned int *height,
                                             unsigned int components) {
  FILE *f = fopen(filename, "rb");
  if (!f)
    return LOADBMP_FILE_NOT_FOUND;

  unsigned char bmp_file_header[14];
  unsigned char bmp_info_header[40];
  unsigned char bmp_pad[3];

  unsigned int w, h;
  unsigned char *data = NULL;

  if (fread(bmp_file_header, 14, 1, f) != 1) {
    fclose(f);
    return LOADBMP_INVALID_FILE_FORMAT;
  }
  if (fread(bmp_info_header, 40, 1, f) != 1) {
    fclose(f);
    return LOADBMP_INVALID_FILE_FORMAT;
  }

  if (bmp_file_header[0] != 'B' || bmp_file_header[1] != 'M') {
    fclose(f);
    return LOADBMP_INVALID_SIGNATURE;
  }

  if (bmp_info_header[14] != 24 && bmp_info_header[14] != 32) {
    fclose(f);
    return LOADBMP_INVALID_BITS_PER_PIXEL;
  }

  w = (unsigned int)(bmp_info_header[4] | (bmp_info_header[5] << 8) |
                     (bmp_info_header[6] << 16) | (bmp_info_header[7] << 24));
  h = (unsigned int)(bmp_info_header[8] | (bmp_info_header[9] << 8) |
                     (bmp_info_header[10] << 16) | (bmp_info_header[11] << 24));

  if (w == 0 || h == 0) {
    fclose(f);
    return LOADBMP_INVALID_FILE_FORMAT;
  }

  data = (unsigned char *)malloc(w * h * components);
  if (!data) {
    fclose(f);
    return LOADBMP_OUT_OF_MEMORY;
  }

  unsigned int padding = ((4 - (w * 3) % 4) % 4);
  for (int y = (int)h - 1; y >= 0; y--) {
    for (unsigned int x = 0; x < w; x++) {
      unsigned int i = (x + (unsigned int)y * w) * components;
      if (fread(data + i, 3, 1, f) != 1) {
        free(data);
        fclose(f);
        return LOADBMP_INVALID_FILE_FORMAT;
      }
      // BGR -> RGB swap using a temporary variable
      unsigned char tmp = data[i];
      data[i] = data[i + 2];
      data[i + 2] = tmp;
      if (components == LOADBMP_RGBA)
        data[i + 3] = 255;
    }
    if (fread(bmp_pad, 1, padding, f) != padding) {
      free(data);
      fclose(f);
      return LOADBMP_INVALID_FILE_FORMAT;
    }
  }

  *width = w;
  *height = h;
  *imageData = data;
  fclose(f);
  return LOADBMP_NO_ERROR;
}

LOADBMP_API unsigned int loadbmp_encode_file(const char *filename,
                                             const unsigned char *imageData,
                                             unsigned int width,
                                             unsigned int height,
                                             unsigned int components) {
  FILE *f = fopen(filename, "wb");
  if (!f)
    return LOADBMP_FILE_OPERATION;

  unsigned char bmp_file_header[14] = {'B', 'M', 0, 0,  0, 0, 0,
                                       0,   0,   0, 54, 0, 0, 0};
  unsigned char bmp_info_header[40] = {40, 0, 0, 0, 0, 0, 0,  0,
                                       0,  0, 0, 0, 1, 0, 24, 0};
  unsigned char bmp_pad[3] = {0, 0, 0};

  unsigned int size = 54 + width * height * 3;
  bmp_file_header[2] = (unsigned char)(size);
  bmp_file_header[3] = (unsigned char)(size >> 8);
  bmp_file_header[4] = (unsigned char)(size >> 16);
  bmp_file_header[5] = (unsigned char)(size >> 24);

  bmp_info_header[4] = (unsigned char)(width);
  bmp_info_header[5] = (unsigned char)(width >> 8);
  bmp_info_header[6] = (unsigned char)(width >> 16);
  bmp_info_header[7] = (unsigned char)(width >> 24);
  bmp_info_header[8] = (unsigned char)(height);
  bmp_info_header[9] = (unsigned char)(height >> 8);
  bmp_info_header[10] = (unsigned char)(height >> 16);
  bmp_info_header[11] = (unsigned char)(height >> 24);

  if (fwrite(bmp_file_header, 14, 1, f) != 1) {
    fclose(f);
    return LOADBMP_FILE_OPERATION;
  }
  if (fwrite(bmp_info_header, 40, 1, f) != 1) {
    fclose(f);
    return LOADBMP_FILE_OPERATION;
  }

  unsigned int padding = ((4 - (width * 3) % 4) % 4);
  for (int y = (int)height - 1; y >= 0; y--) {
    for (unsigned int x = 0; x < width; x++) {
      unsigned int i = (x + (unsigned int)y * width) * components;
      unsigned char pixel[3];
      pixel[0] = imageData[i + 2]; // B
      pixel[1] = imageData[i + 1]; // G
      pixel[2] = imageData[i];     // R
      if (fwrite(pixel, 3, 1, f) != 1) {
        fclose(f);
        return LOADBMP_FILE_OPERATION;
      }
    }
    if (fwrite(bmp_pad, 1, padding, f) != padding) {
      fclose(f);
      return LOADBMP_FILE_OPERATION;
    }
  }

  fclose(f);
  return LOADBMP_NO_ERROR;
}

#endif /* LOADBMP_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* LOADBMP_H */
