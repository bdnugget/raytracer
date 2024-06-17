#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define CANVAS_WIDTH 800
#define CANVAS_HEIGHT 800

typedef struct Color {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} Color;

typedef struct Vector2 {
  float x;
  float y;
} Vector2;

typedef struct Vector3 {
  float x;
  float y;
  float z;
} Vector3;

void writeBitmap(Color *bitmap, char *filename);
void setPixel(Color *bitmap, Vector2 position, Color color);
void drawLine(Color *bitmap, Vector2 start, Vector2 end, Color color);
void initScene(void);
Vector2 viewportToCanvas(Vector2 *position);

static Vector3 cameraPosition = { 0 };
static float distanceCameraToViewport = 1.0f;
static Vector3 viewportPosition = { 0 };
static Vector2 viewportSize = { 0 };
static Vector2 canvasSize = { 0 };

int main(void) {
  initScene();
  Color *bitmap = malloc(canvasSize.x * canvasSize.y * sizeof(Color));
  if (bitmap == NULL) {
    printf("Error allocating bitmap.\n");
    return EXIT_FAILURE;
  }

  drawLine(bitmap, (Vector2){0, 0}, (Vector2){canvasSize.x, canvasSize.y},
           (Color){255, 255, 255});
  drawLine(bitmap, (Vector2){canvasSize.x, 0}, (Vector2){0, canvasSize.y},
           (Color){255, 255, 0});
  drawLine(bitmap, (Vector2){0, canvasSize.y - 300},
           (Vector2){canvasSize.x, 0}, (Color){255, 0, 255});
  drawLine(bitmap, (Vector2){canvasSize.x / 2.0f, 0},
           (Vector2){canvasSize.x / 2.0f, canvasSize.y}, (Color){0, 255, 255});
  drawLine(bitmap, (Vector2){0, canvasSize.y / 2.0f},
           (Vector2){canvasSize.x - 69, canvasSize.y / 2.0f},
           (Color){255, 0, 0});

  writeBitmap(bitmap, "output.ppm");
  free(bitmap);
  return EXIT_SUCCESS;
}

void initScene(void) {
  cameraPosition = (Vector3){0, 0, 0};
  distanceCameraToViewport = 1.0f;
  viewportPosition = (Vector3){0, 0, distanceCameraToViewport};
  viewportSize = (Vector2){ 1.0f, 1.0f };
  canvasSize = (Vector2){ CANVAS_WIDTH, CANVAS_HEIGHT };
}

void writeBitmap(Color *bitmap, char *filename) {
  if (bitmap == NULL) {
    printf("Bitmap is NULL.\n");
    return;
  }
  FILE *fp = fopen(filename, "wb");
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

void setPixel(Color *bitmap, Vector2 position, Color color) {
  if (bitmap == NULL) {
    printf("Bitmap is NULL.\n");
    return;
  }
  if (position.x < 0 || position.x >= CANVAS_WIDTH || position.y < 0 ||
      position.y >= CANVAS_HEIGHT) {
    printf("Position out of bounds: (%f, %f)\n", position.x, position.y);
    return;
  }
  bitmap[(int)position.y * CANVAS_WIDTH + (int)position.x] = color;
}

void drawLine(Color *bitmap, Vector2 start, Vector2 end, Color color) {
  if (bitmap == NULL) {
    printf("Bitmap is NULL.\n");
    return;
  }
  if (start.x == end.x) {
    // Vertical line
    int yStart = (int)start.y;
    int yEnd = (int)end.y;
    int yInc = yStart < yEnd ? 1 : -1;
    for (int y = yStart; y != yEnd + yInc; y += yInc) {
      setPixel(bitmap, (Vector2){start.x, (float)y}, color);
    }
    return;
  }
  if (start.y == end.y) {
    // Horizontal line
    int xStart = (int)start.x;
    int xEnd = (int)end.x;
    int xInc = xStart < xEnd ? 1 : -1;
    for (int x = xStart; x != xEnd + xInc; x += xInc) {
      setPixel(bitmap, (Vector2){(float)x, start.y}, color);
    }
    return;
  }

  // Diagonal line using Bresenham's algorithm
  int dx = fabsf(end.x - start.x), sx = start.x < end.x ? 1 : -1;
  int dy = -fabsf(end.y - start.y), sy = start.y < end.y ? 1 : -1;
  int err = dx + dy, e2;

  Vector2 current = start;
  while (1) {
    setPixel(bitmap, current, color);
    if (current.x == end.x && current.y == end.y) {
      break;
    }
    e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      current.x += sx;
    }
    if (e2 <= dx) {
      err += dx;
      current.y += sy;
    }
  }
}
