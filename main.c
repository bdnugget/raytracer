#include <limits.h>
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

typedef struct Sphere {
  Vector3 position;
  float radius;
  Color color;
} Sphere;

void writeBitmap(Color *canvas, char *filename);
void setPixel(Color *canvas, Vector2 position, Color color);
void initRenderContext(void);
void initScene(void);

float dotProduct(Vector3 a, Vector3 b);
float lengthVector3(Vector3 v);
float normalize(Vector3 v);

Color traceRay(Vector3 rayOrigin, Vector3 rayDirection, int t_min, int t_max);

Vector2 intersectRaySphere(Vector3 rayOrigin, Vector3 rayDirection,
                           Sphere *sphere);
Vector3 canvasToViewport(Vector2 position);
Vector3 subtractVector3(Vector3 a, Vector3 b);

static Vector3 cameraPosition = {0};
static float distanceCameraToViewport = 1.0f;
static Vector3 viewportPosition = {0};
static Vector2 viewportSize = {0};
static Vector2 canvasSize = {.x = CANVAS_WIDTH, .y = CANVAS_HEIGHT};
static struct Scene {
  Sphere *spheres;
  int numSpheres;
} scene;

// The global canvas that we draw on as a sort of bitmap
static Color *canvas = NULL;

int main(void) {
  initRenderContext();
  initScene();

  for (int x = 0; x < canvasSize.x; x++) {
    for (int y = 0; y < canvasSize.y; y++) {
      Vector3 rayDirection = (Vector3){(float)x / canvasSize.x,
                                       (float)y / canvasSize.y, 0};
      Vector3 rayOrigin = (Vector3){0, 0, 0};
      Color color = traceRay(rayOrigin, rayDirection, 0, INT_MAX);
      setPixel(canvas, (Vector2){(float)x, (float)y}, color);
    }
  }

  writeBitmap(canvas, "output.ppm");
  free(canvas);
  return EXIT_SUCCESS;
}

void initRenderContext(void) {
  cameraPosition = (Vector3){0, 0, 0};
  distanceCameraToViewport = 1.0f;
  viewportPosition = (Vector3){0, 0, distanceCameraToViewport};
  viewportSize = (Vector2){1.0f, 1.0f};
  canvas = malloc(canvasSize.x * canvasSize.y * sizeof(Color));
  if (canvas == NULL) {
    printf("Error allocating canvas.\n");
  }
}

void initScene(void) {
  // Set up 3 spheres
  scene.spheres = malloc(3 * sizeof(Sphere));
  if (scene.spheres == NULL) {
    printf("Error allocating spheres.\n");
  }
  scene.numSpheres = 3;

  scene.spheres[0].position = (Vector3){0, -1, 3};
  scene.spheres[0].radius = 1.0f;
  scene.spheres[0].color = (Color){255, 0, 0};

  scene.spheres[1].position = (Vector3){2, 0, 4};
  scene.spheres[1].radius = 1.0f;
  scene.spheres[1].color = (Color){0, 0, 255};

  scene.spheres[2].position = (Vector3){-2, 0, 4};
  scene.spheres[2].radius = 1.0f;
  scene.spheres[2].color = (Color){0, 255, 0};
}

void writeBitmap(Color *canvas, char *filename) {
  if (canvas == NULL) {
    printf("Canvas is NULL.\n");
    return;
  }
  FILE *fp = fopen(filename, "wb");
  if (fp == NULL) {
    printf("Error opening file.\n");
    return;
  }
  fprintf(fp, "P6\n%d %d\n255\n", (int)canvasSize.x, (int)canvasSize.y);
  setvbuf(fp, NULL, _IOFBF, canvasSize.x * canvasSize.y * 3);
  fwrite(canvas, sizeof(Color), canvasSize.x * canvasSize.y, fp);
  if (fclose(fp) != 0) {
    printf("Error closing file.\n");
  }
}

void setPixel(Color *canvas, Vector2 position, Color color) {
  if (canvas == NULL) {
    printf("Canvas is NULL.\n");
    return;
  }
  if (position.x < 0 || position.x >= canvasSize.x || position.y < 0 ||
      position.y >= canvasSize.y) {
    printf("Position out of bounds: (%f, %f)\n", position.x, position.y);
    return;
  }
  canvas[(int)position.y * (int)canvasSize.x + (int)position.x] = color;
}

Vector3 canvasToViewport(Vector2 position) {
  return (Vector3){(float)position.x * (viewportSize.x / canvasSize.x),
                   (float)position.y * (viewportSize.y / canvasSize.y),
                   viewportPosition.z};
  // return position.x vw/cw
}

float dotProduct(Vector3 a, Vector3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

float lengthVector3(Vector3 v) { return sqrtf(dotProduct(v, v)); }

float normalize(Vector3 v) {
  float length = lengthVector3(v);
  return length == 0 ? 0 : 1.0f / length;
}

Vector3 subtractVector3(Vector3 a, Vector3 b) {
  return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z};
}

Color traceRay(Vector3 rayOrigin, Vector3 rayDirection, int t_min, int t_max) {
  int closest_t = INT_MAX;
  Sphere *closestSphere = NULL;

  for (int i = 0; i < scene.numSpheres; i++) {
    Vector2 intersection =
        intersectRaySphere(rayOrigin, rayDirection, &scene.spheres[i]);
    if (intersection.y == 1 && intersection.x < closest_t) {
      closest_t = intersection.x;
      closestSphere = &scene.spheres[i];
    }
  }

  if (closestSphere == NULL) {
    return (Color){0, 0, 0};
  }

  return closestSphere->color;
}

Vector2 intersectRaySphere(Vector3 rayOrigin, Vector3 rayDirection,
                           Sphere *sphere) {
  Vector3 oc = subtractVector3(rayOrigin, sphere->position);
  float a = dotProduct(rayDirection, rayDirection);
  float b = 2.0f * dotProduct(oc, rayDirection);
  float c = dotProduct(oc, oc) - sphere->radius * sphere->radius;
  float discriminant = b * b - 4.0f * a * c;
  if (discriminant < 0) {
    return (Vector2){0, 0};
  }
  float t1 = (-b + sqrtf(discriminant)) / (2.0f * a);
  float t2 = (-b - sqrtf(discriminant)) / (2.0f * a);
  return (Vector2){t1, t2};
}