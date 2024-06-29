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

typedef enum LightType {
  AMBIENT,
  POINT,
  DIRECTIONAL
} LightType;

typedef struct Light {
  float intensity;
  LightType type;
  union {
    Vector3 position;
    Vector3 direction;
  };
} Light;

void writeBitmap(Color *canvas, char *filename);
void setPixel(Color *canvas, Vector2 position, Color color);
void initRenderContext(void);
void initScene(void);

float dotProduct(Vector3 a, Vector3 b);
float lengthVector3(Vector3 v);
Vector3 normalize(Vector3 v);

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

static Light *lights = NULL;
static int numLights = 0;
static Color *canvas = NULL;

// Billiards table color dark green
static Color backgroundColor = {0, 64, 0};

int main(void) {
  initRenderContext();
  initScene();

  for (int y = 0; y < canvasSize.y; y++) {
    for (int x = 0; x < canvasSize.x; x++) {
      Vector2 canvasPos = {x, y};
      Vector3 rayDirection = normalize(canvasToViewport(canvasPos));
      Vector3 rayOrigin = cameraPosition;
      Color color = traceRay(rayOrigin, rayDirection, 0, INT_MAX);
      setPixel(canvas, (Vector2){(float)x, (float)y}, color);
    }
  }

  writeBitmap(canvas, "output.ppm");
  free(canvas);
  free(scene.spheres);
  return EXIT_SUCCESS;
}

void initRenderContext(void) {
  cameraPosition = (Vector3){0.0f, 0.0f, 0.0f};
  distanceCameraToViewport = 1.0f;
  viewportPosition = (Vector3){0.0f, 0.0f, distanceCameraToViewport};
  viewportSize = (Vector2){1.0f, 1.0f};
  canvas = malloc(canvasSize.x * canvasSize.y * sizeof(Color));
  if (canvas == NULL) {
    printf("Error allocating canvas.\n");
    exit(EXIT_FAILURE);
  }
}

void initScene(void) {
  // Set up 3 spheres
  scene.spheres = malloc(3 * sizeof(Sphere));
  if (scene.spheres == NULL) {
    printf("Error allocating spheres.\n");
    exit(EXIT_FAILURE);
  }
  scene.numSpheres = 3;

  // Carom billard ball colors
  scene.spheres[0].position = (Vector3){0.0f, -1.0f, 3.0f};
  scene.spheres[0].radius = 1.0f;
  scene.spheres[0].color = (Color){255, 0, 0};

  scene.spheres[1].position = (Vector3){2.0f, 0.0f, 4.0f};
  scene.spheres[1].radius = 1.0f;
  scene.spheres[1].color = (Color){255, 255, 0};

  scene.spheres[2].position = (Vector3){-2.0f, 0.0f, 4.0f};
  scene.spheres[2].radius = 1.0f;
  scene.spheres[2].color = (Color){255, 255, 255};

  // Set up lights
  lights = malloc(3 * sizeof(Light));
  if (lights == NULL) {
    printf("Error allocating lights.\n");
    exit(EXIT_FAILURE);
  }
  numLights = 3;

  lights[0].type = AMBIENT;
  lights[0].intensity = 0.2f;

  lights[1].type = POINT;
  lights[1].intensity = 0.6f;
  lights[1].position = (Vector3){2.0f, 1.0f, 0.0f};

  lights[2].type = DIRECTIONAL;
  lights[2].intensity = 0.2f;
  lights[2].direction = (Vector3){ 1.0f, 4.0f, 4.0f};
}

void writeBitmap(Color *canvas, char *filename) {
  if (canvas == NULL) {
    printf("Canvas is NULL.\n");
    exit(EXIT_FAILURE);
  }
  FILE *fp = fopen(filename, "wb");
  if (fp == NULL) {
    printf("Error opening file.\n");
    exit(EXIT_FAILURE);
  }
  fprintf(fp, "P6\n%d %d\n255\n", (int)canvasSize.x, (int)canvasSize.y);
  setvbuf(fp, NULL, _IOFBF, canvasSize.x * canvasSize.y * 3);
  fwrite(canvas, sizeof(Color), canvasSize.x * canvasSize.y, fp);
  if (fclose(fp) != 0) {
    printf("Error closing file.\n");
    exit(EXIT_FAILURE);
  }
}

void setPixel(Color *canvas, Vector2 position, Color color) {
  if (canvas == NULL) {
    printf("Canvas is NULL.\n");
    exit(EXIT_FAILURE);
  }
  if (position.x < 0 || position.x >= canvasSize.x || position.y < 0 ||
      position.y >= canvasSize.y) {
    printf("Position out of bounds: (%f, %f)\n", position.x, position.y);
    exit(EXIT_FAILURE);
  }
  canvas[(int)position.y * (int)canvasSize.x + (int)position.x] = color;
}

Vector3 canvasToViewport(Vector2 position) {
  return (Vector3){
      (position.x - canvasSize.x / 2.0f) * viewportSize.x / canvasSize.x,
      (position.y - canvasSize.y / 2.0f) * viewportSize.y / canvasSize.y,
      viewportPosition.z};
}

float dotProduct(Vector3 a, Vector3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

float lengthVector3(Vector3 v) { return sqrtf(dotProduct(v, v)); }

Vector3 normalize(Vector3 v) {
  float length = lengthVector3(v);
  return length == 0.0f ? (Vector3){0, 0, 0}
                        : (Vector3){v.x / length, v.y / length, v.z / length};
}

Vector3 subtractVector3(Vector3 a, Vector3 b) {
  return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z};
}

Color traceRay(Vector3 rayOrigin, Vector3 rayDirection, int t_min, int t_max) {
  float closest_t = INT_MAX;
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
    return backgroundColor;
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
    return (Vector2){INT_MAX, 0.0f};
  }

  float discriminantSqrt = sqrtf(discriminant);
  float t1 = (-b - discriminantSqrt) / (2.0f * a);
  float t2 = (-b + discriminantSqrt) / (2.0f * a);

  float t = (t1 > 0 && t2 > 0) ? fminf(t1, t2) : fmaxf(t1, t2);

  if (t < 0) {
    return (Vector2){INT_MAX, 0};
  }

  return (Vector2){t, 1};
}
