#ifndef CORE_MATH_H
#define CORE_MATH_H

#include <math.h>

#ifndef PI
#define PI 3.141592653589793238f
#endif

#ifndef TAU
#define TAU 6.283185307179586f
#endif

#define RadToDeg(R) (R*180.0f/PI)
#define DegToRad(R) (R*PI/180.0f)

union Quaternion {
  struct {
    float x, y, z, w;
  };
  float elements[4];

  Quaternion() { x = y = z = w = 0; }
  Quaternion(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
  f32 &operator[](int idx) {
    assert(idx < 4);
    return elements[idx];
  }
};

union Vector2 {
  struct {
    float x;
    float y;
  };
  float elements[2];

  Vector2() { x = y = 0; }
  Vector2(float x, float y) : x(x), y(y) {}
  float &operator[](int idx) {
    assert(idx < 2);
    return elements[idx];
  }
};

union Vector3 {
  struct {
    float x;
    float y;
    float z;
  };
  float elements[3];

  Vector3() { x = y = z = 0; }
  Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
  float &operator[](int idx) {
    assert(idx < 3);
    return elements[idx];
  }

  Vector3 Zero() { return {0, 0, 0}; }
  Vector3 Up()   { return {0, 1, 0}; }
};

union Vector4 {
  struct {
    float x;
    float y;
    float z;
    float w;
  };
  float elements[4];

  Vector4() { x = y = z = w = 0; }
  Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
  Vector4(Vector3 v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}
  float &operator[](int idx) {
    assert(idx < 4);
    return elements[idx];
  }
};

union Vector2Int {
  struct {
    int x;
    int y;
  };
  int elements[2];

  Vector2Int() { x = y = 0; }
  Vector2Int(int x, int y) : x(x), y(y) {}
  int &operator[](int idx) {
    assert(idx < 2);
    return elements[idx];
  }
};

union Vector3Int {
  struct {
    int x;
    int y;
    int z;
  };
  int elements[3];

  Vector3Int() { x = y = z = 0; }
  Vector3Int(int x, int y, int z) : x(x), y(y), z(z) {}
  int &operator[](int idx) {
    assert(idx < 3);
    return elements[idx];
  }

  static Vector3Int Zero() { return {0, 0, 0}; }
  static Vector3Int Up()   { return {0, 1, 0}; }
};

union Vector4Int {
  struct {
    int x;
    int y;
    int z;
    int w;
  };
  int elements[4];

  Vector4Int() { x = y = z = w = 0; }
  Vector4Int(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) {}
  int &operator[](int idx) {
    assert(idx < 4);
    return elements[idx];
  }
};

union Matrix3 {
  Vector3 columns[3];
  struct {
    float _00, _01, _02;
    float _10, _11, _12;
    float _20, _21, _22;
  };
  float elements[9];

  Matrix3() {}

  Vector3 &operator[](int idx) {
    assert(idx < 3);
    return columns[idx];
  }
};

union Matrix4 {
  Vector4 columns[4];
  struct {
    float _00, _01, _02, _03;
    float _10, _11, _12, _13;
    float _20, _21, _22, _23;
    float _30, _31, _32, _33;
  };
  float elements[16];

  Matrix4() {}

  Vector4 &operator[](int idx) {
    assert(idx < 16);
    return columns[idx];
  }
};

internal inline Matrix4 operator*(Matrix4 a, Matrix4 b);

internal inline Vector4 operator*(Matrix4 m, Vector4 v);
internal inline Matrix4 rotate_rh(f32 angle, Vector3 axis);

#endif // CORE_MATH_H
