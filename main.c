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
  float specular;
} Sphere;

typedef enum LightType { AMBIENT, POINT, DIRECTIONAL } LightType;

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

unsigned char clamp(float value) {
  return (unsigned char)(value > 255 ? 255 : value < 0 ? 0 : value);
}

float dotProduct(Vector3 a, Vector3 b);
float lengthVector3(Vector3 v);
float computeLighting(Vector3 pointOnSurface, Vector3 surfaceNormal,
                      Vector3 pointToCamera, float specular);
Vector3 normalize(Vector3 v);

Color traceRay(Vector3 rayOrigin, Vector3 rayDirection, int t_min, int t_max);

Vector2 intersectRaySphere(Vector3 rayOrigin, Vector3 rayDirection,
                           Sphere *sphere);
Vector3 canvasToViewport(Vector2 position);
Vector3 subtractVector3(Vector3 a, Vector3 b);
Vector3 addVector3(Vector3 a, Vector3 b);
Vector3 scaleVector3(Vector3 v, float scale);

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

static Color backgroundColor = {10, 10, 10};

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
  // Set up spheres
  scene.numSpheres = 4;
  scene.spheres = malloc(scene.numSpheres * sizeof(Sphere));
  if (scene.spheres == NULL) {
    printf("Error allocating spheres.\n");
    exit(EXIT_FAILURE);
  }

  // Carom billard ball colors
  scene.spheres[0].position = (Vector3){0.0f, 1.0f, 3.0f};
  scene.spheres[0].radius = 1.0f;
  scene.spheres[0].color = (Color){255, 0, 0};
  scene.spheres[0].specular = 500.0f;

  scene.spheres[1].position = (Vector3){2.0f, 0.0f, 4.0f};
  scene.spheres[1].radius = 1.0f;
  scene.spheres[1].color = (Color){255, 255, 0};
  scene.spheres[1].specular = 500.0f;

  scene.spheres[2].position = (Vector3){-2.0f, 0.0f, 4.0f};
  scene.spheres[2].radius = 1.0f;
  scene.spheres[2].color = (Color){255, 255, 255};
  scene.spheres[2].specular = 10.0f;

  // Huge green ogre ball lol
  scene.spheres[3].position = (Vector3){0.0f, 5001.0f, 0.0f};
  scene.spheres[3].radius = 5000.0f;
  scene.spheres[3].color = (Color){0, 255, 0};
  scene.spheres[3].specular = 1000.0f;

  // Set up lights
  lights = malloc(3 * sizeof(Light));
  if (lights == NULL) {
    printf("Error allocating lights.\n");
    exit(EXIT_FAILURE);
  }
  numLights = 3;

  // Intensities add up to 1 so we don't get "under- or overexposure"
  lights[0].type = AMBIENT;
  lights[0].intensity = 0.2f;

  lights[1].type = POINT;
  lights[1].intensity = 0.6f;
  lights[1].position = (Vector3){2.0f, 1.0f, 0.0f};

  lights[2].type = DIRECTIONAL;
  lights[2].intensity = 0.2f;
  lights[2].direction = (Vector3){1.0f, 4.0f, 4.0f};
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

Vector3 addVector3(Vector3 a, Vector3 b) {
  return (Vector3){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vector3 scaleVector3(Vector3 v, float s) {
  return (Vector3){v.x * s, v.y * s, v.z * s};
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

  Vector3 point = addVector3(rayOrigin, scaleVector3(rayDirection, closest_t));
  Vector3 normal = subtractVector3(point, closestSphere->position);
  normal = normalize(normal);

  float lighting =
      computeLighting(point, normal, rayDirection, closestSphere->specular);
  Color result = (Color){
      .r = clamp(lighting * closestSphere->color.r),
      .g = clamp(lighting * closestSphere->color.g),
      .b = clamp(lighting * closestSphere->color.b)
  };
  return result;
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

float computeLighting(Vector3 surfacePosition, Vector3 surfaceNormal,
                      Vector3 pointToCamera, float specular) {
  float lighting = 0.0f;
  for (int i = 0; i < numLights; i++) {
    if (lights[i].type == AMBIENT) {
      lighting += lights[i].intensity;
    } else {
      Vector3 l = {0};
      if (lights[i].type == POINT) {
        l = normalize(subtractVector3(lights[i].position, surfacePosition));
      } else {
        l = normalize(lights[i].direction);
      }

      // Diffuse reflection
      float nDotL = dotProduct(surfaceNormal, l);
      if (nDotL > 0) {
        lighting += lights[i].intensity * nDotL;
      }

      // Specular reflection
      if (specular > 0) {
        Vector3 r =
            normalize(subtractVector3(scaleVector3(l, -1.0f), surfaceNormal));
        float rDotV = dotProduct(r, pointToCamera);
        if (rDotV > 0) {
          lighting += lights[i].intensity * powf(rDotV, specular);
        }
      }
    }
  }
  return lighting;
}
