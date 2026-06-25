#include "dither.h"
#include "loadbmp.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// Flat palette - 3 rows x 8 cols
static unsigned int my_palette_flat[3 * 8] = {
    0xFF0000, 0xef3434, 0xe56363, 0xe08d8d, 0xe0b1b1, 0xe5d0d0,
    0xefeaea, 0xFFFFFF, 0x00FF00, 0x3aaa0d, 0x83cb22, 0xbed053,
    0xd0c68a, 0xd8cbb9, 0xe8e2e0, 0xFFFFFF, 0x0000FF, 0x349fef,
    0x63e5d2, 0x8de0a4, 0xbee0b1, 0xe2e5d0, 0xefedea, 0xFFFFFF};

// Helper to create directory
static void ensure_directory(const char *path) { mkdir(path, 0755); }

// List all .bmp files in the images/ directory
static int list_bmp_files(char ***filenames) {
  DIR *dir = opendir("images");
  if (!dir) {
    fprintf(stderr, "Cannot open 'images' directory.\n");
    return -1;
  }

  struct dirent *entry;
  int count = 0;
  *filenames = NULL;

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG) {
      const char *name = entry->d_name;
      size_t len = strlen(name);
      if (len > 4 && strcasecmp(name + len - 4, ".bmp") == 0) {
        (*filenames) = realloc(*filenames, sizeof(char *) * (count + 1));
        (*filenames)[count] = strdup(name);
        count++;
      }
    }
  }
  closedir(dir);
  return count;
}

int main() {
  ensure_directory("results");

  char **bmp_files = NULL;
  int num_files = list_bmp_files(&bmp_files);
  if (num_files <= 0) {
    fprintf(stderr, "No .bmp files found in 'images/'.\n");
    return 1;
  }

  fprintf(stderr, "Available images:\n");
  for (int i = 0; i < num_files; i++) {
    fprintf(stderr, "  %d. %s\n", i + 1, bmp_files[i]);
  }
  fprintf(stderr, "Select image (1-%d): ", num_files);
  fflush(stderr);

  int choice;
  if (scanf("%d", &choice) != 1 || choice < 1 || choice > num_files) {
    fprintf(stderr, "Invalid selection.\n");
    for (int i = 0; i < num_files; i++)
      free(bmp_files[i]);
    free(bmp_files);
    return 1;
  }

  char input_path[256];
  snprintf(input_path, sizeof(input_path), "images/%s", bmp_files[choice - 1]);

  // Build output path: results/out_<filename>
  char output_path[256];
  const char *base = bmp_files[choice - 1];
  snprintf(output_path, sizeof(output_path), "results/out_%s", base);

  // Free the file list
  for (int i = 0; i < num_files; i++)
    free(bmp_files[i]);
  free(bmp_files);

  // Load and dither
  unsigned char *pixelsIn = NULL;
  unsigned int width, height;
  unsigned int err =
      loadbmp_decode_file(input_path, &pixelsIn, &width, &height, LOADBMP_RGBA);
  if (err) {
    fprintf(stderr, "LoadBMP error: %u\n", err);
    return 1;
  }

  Palette pal;
  pal.rows = 3;
  pal.cols = 8;
  pal.colors = my_palette_flat;

  DitherParams params;
  dither_default_params(&params, &pal);

  unsigned char *pixelsOut = (unsigned char *)malloc(width * height * 4);
  unsigned char *indexArray = (unsigned char *)malloc(width * height);
  if (!pixelsOut || !indexArray) {
    free(pixelsIn);
    free(pixelsOut);
    free(indexArray);
    return 1;
  }

  dither_image(pixelsIn, width, height, &params, pixelsOut, indexArray);

  // Write output BMP
  err =
      loadbmp_encode_file(output_path, pixelsOut, width, height, LOADBMP_RGBA);
  if (err) {
    fprintf(stderr, "Encode error: %u\n", err);
  } else {
    printf("Dithered image saved as: %s\n", output_path);
  }

  // Print C array header
  printf("int img_w = %u, img_h = %u;\n", width, height);
  printf("uint8_t img_buf[] = {");
  for (unsigned int i = 0; i < width * height; i++) {
    printf("%d%c", 7 - indexArray[i], (i == width * height - 1) ? ' ' : ',');
    if ((i + 1) % 16 == 0)
      printf("\n");
  }
  printf("};\n");

  free(pixelsIn);
  free(pixelsOut);
  free(indexArray);
  return 0;
}
