// This is crap that I don't need anymore but feel bad about throwing away

#include <stdio.h>
#include "main.c"

void drawLine(Color *canvas, Vector2 start, Vector2 end, Color color) {
  if (canvas == NULL) {
    printf("Canvas is NULL.\n");
    return;
  }
  if (start.x == end.x) {
    // Vertical line
    int yStart = (int)start.y;
    int yEnd = (int)end.y;
    int yInc = yStart < yEnd ? 1 : -1;
    for (int y = yStart; y != yEnd + yInc; y += yInc) {
      setPixel(canvas, (Vector2){start.x, (float)y}, color);
    }
    return;
  }
  if (start.y == end.y) {
    // Horizontal line
    int xStart = (int)start.x;
    int xEnd = (int)end.x;
    int xInc = xStart < xEnd ? 1 : -1;
    for (int x = xStart; x != xEnd + xInc; x += xInc) {
      setPixel(canvas, (Vector2){(float)x, start.y}, color);
    }
    return;
  }

  // Diagonal line using Bresenham's algorithm
  int dx = fabsf(end.x - start.x), sx = start.x < end.x ? 1 : -1;
  int dy = -fabsf(end.y - start.y), sy = start.y < end.y ? 1 : -1;
  int err = dx + dy, e2;

  Vector2 current = start;
  while (1) {
    setPixel(canvas, current, color);
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