#include "dither.h"
#include "loadbmp.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
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

static KernelType parse_kernel(const char *arg) {
  if (strcasecmp(arg, "floyd") == 0 || strcasecmp(arg, "fs") == 0)
    return KERNEL_FLOYD_STEINBERG;
  if (strcasecmp(arg, "jarvis") == 0)
    return KERNEL_JARVIS;
  if (strcasecmp(arg, "atkinson") == 0)
    return KERNEL_ATKINSON;
  if (strcasecmp(arg, "burkes") == 0)
    return KERNEL_BURKES;
  if (strcasecmp(arg, "stucki") == 0)
    return KERNEL_STUCKI;
  if (strcasecmp(arg, "sierra") == 0 || strcasecmp(arg, "sierralite") == 0)
    return KERNEL_SIERRA_LITE;
  fprintf(stderr, "Unknown kernel '%s', using Stucki.\n", arg);
  return KERNEL_STUCKI;
}

static const char *kernel_to_string(KernelType kt) {
  switch (kt) {
  case KERNEL_FLOYD_STEINBERG:
    return "floyd";
  case KERNEL_JARVIS:
    return "jarvis";
  case KERNEL_ATKINSON:
    return "atkinson";
  case KERNEL_BURKES:
    return "burkes";
  case KERNEL_STUCKI:
    return "stucki";
  case KERNEL_SIERRA_LITE:
    return "sierra";
  default:
    return "unknown";
  }
}

int main(int argc, char **argv) {
  KernelType kt = KERNEL_STUCKI; // Default kernel

  // Parse command line: -k <kernel>
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-k") == 0 && i + 1 < argc) {
      kt = parse_kernel(argv[++i]);
    }
  }

  ensure_directory("results");

  char **bmp_files = NULL;
  int num_files = list_bmp_files(&bmp_files);
  if (num_files <= 0) {
    fprintf(stderr, "No .bmp files found in 'images/'.\n");
    return 1;
  }

  fprintf(stderr, "Available images:\n");
  for (int i = 0; i < num_files; i++)
    fprintf(stderr, "  %d. %s\n", i + 1, bmp_files[i]);
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

  char input_path[256], output_path[256];
  snprintf(input_path, sizeof(input_path), "images/%s", bmp_files[choice - 1]);

  char stem[128];
  strncpy(stem, bmp_files[choice - 1], sizeof(stem) - 1);
  stem[sizeof(stem) - 1] = '\0';
  char *dot = strrchr(stem, '.');
  if (dot)
    *dot = '\0';

  // Build output path with kernel name
  const char *kname = kernel_to_string(kt);
  snprintf(output_path, sizeof(output_path), "results/out_%s_%s.bmp", stem,
           kname);

  // Free list
  for (int i = 0; i < num_files; i++)
    free(bmp_files[i]);
  free(bmp_files);

  // Load image
  unsigned char *pixelsIn = NULL;
  unsigned int width, height;
  unsigned int err =
      loadbmp_decode_file(input_path, &pixelsIn, &width, &height, LOADBMP_RGBA);
  if (err) {
    fprintf(stderr, "Load error: %u\n", err);
    return 1;
  }

  Palette pal;
  pal.rows = 3;
  pal.cols = 8;
  pal.colors = my_palette_flat;

  DitherParams params;
  dither_init_params(&params, &pal, kt);

  unsigned char *pixelsOut = malloc(width * height * 4);
  unsigned char *indexArray = malloc(width * height);
  if (!pixelsOut || !indexArray) {
    free(pixelsIn);
    free(pixelsOut);
    free(indexArray);
    return 1;
  }

  dither_image(pixelsIn, width, height, &params, pixelsOut, indexArray);

  err =
      loadbmp_encode_file(output_path, pixelsOut, width, height, LOADBMP_RGBA);

  if (err)
    fprintf(stderr, "Encode error: %u\n", err);
  else
    fprintf(stderr, "Saved: %s (kernel %d)\n", output_path, kt);

  // Print C array
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
