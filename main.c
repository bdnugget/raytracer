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
void drawLine(Color *bitmap, int x1, int y1, int x2, int y2, Color color);

int main(void)
{
  Color *bitmap = malloc(CANVAS_WIDTH * CANVAS_HEIGHT * sizeof(Color));
  if (bitmap == NULL) {
    printf("Error allocating bitmap.\n");
    return EXIT_FAILURE;
  }

  drawLine(bitmap, 0, 0, CANVAS_WIDTH, CANVAS_HEIGHT, (Color){255, 0, 0});
  drawLine(bitmap, 0, CANVAS_HEIGHT, CANVAS_WIDTH, 0, (Color){0, 255, 0});
  

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
  FILE *fp = fopen("test2.ppm", "wb");
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

void drawLine(Color *bitmap, int x1, int y1, int x2, int y2, Color color)
{
  if (bitmap == NULL) {
    printf("Bitmap is NULL.\n");
    return;
  }
  int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
  int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
  int err = dx + dy, e2;

  while (1) {
    bitmap[y1 * CANVAS_WIDTH + x1] = color;
    if (x1 == x2 && y1 == y2) {
      break;
    }
    e2 = 2 * err;
    if (e2 >= dy) {
      err += dy; x1 += sx;
    }
    if (e2 <= dx) {
      err += dx; y1 += sy;
    }
  }
  return;
}
