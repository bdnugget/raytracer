#include <stdlib.h>
#include <stdio.h>

#define CANVAS_WIDTH 800
#define CANVAS_HEIGHT 800

typedef struct Color {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} Color;

void writeBitmap(Color *bitmap);

int main(void)
{
  Color *bitmap = malloc(CANVAS_WIDTH * CANVAS_HEIGHT * sizeof(Color));
  if (bitmap == NULL) {
    printf("Error allocating bitmap.\n");
    return EXIT_FAILURE;
  }
  for (int j = 0; j < CANVAS_HEIGHT; ++j)
  {
    for (int i = 0; i < CANVAS_WIDTH; ++i)
    {
      int idx = j * CANVAS_WIDTH + i;
      bitmap[idx].r = i % 256;
      bitmap[idx].g = j % 256;
      bitmap[idx].b = (i * j) % 256;
    }
  }
  writeBitmap(bitmap);
  free(bitmap);
  return EXIT_SUCCESS;
}

void writeBitmap(Color *bitmap)
{
  if (bitmap == NULL) {
    printf("Bitmap is NULL.\n");
    return;
  }
  FILE *fp = fopen("test.ppm", "wb");
  if (fp == NULL) {
    printf("Error opening file.\n");
    return;
  }
  fprintf(fp, "P6\n%d %d\n255\n", CANVAS_WIDTH, CANVAS_HEIGHT);
  setvbuf(fp, NULL, _IOFBF, CANVAS_WIDTH * CANVAS_HEIGHT * 3);
  fwrite(bitmap, sizeof(Color), CANVAS_WIDTH * CANVAS_HEIGHT, fp);
  if (fclose(fp) != 0) {
    printf("Error closing file.\n");
  }
}

