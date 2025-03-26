
internal inline Vector3 to_vector3(Vector4 v) {
  Vector3 result = {v.x, v.y, v.z};
  return result;
}

internal inline Vector3 to_vector3(Vector3Int v) {
  Vector3 result = {
    (f32)v.x,
    (f32)v.y,
    (f32)v.z
  };
  return result;
}

internal inline Vector2Int truncate(Vector2 v) {
  Vector2Int result = {
    (int)v.x,
    (int)v.y,
  };
  return result;
}
internal inline Vector3Int truncate(Vector3 v) {
  Vector3Int result = {
    (int)v.x,
    (int)v.y,
    (int)v.z
  };
  return result;
}

internal inline Vector2 operator-(Vector2 v) {
  Vector2 result = {
    -v.x,
    -v.y
  };
  return result;
}
internal inline Vector3 operator-(Vector3 v) {
  Vector3 result = {
    -v.x,
    -v.y,
    -v.z
  };
  return result;
}
internal inline Vector4 operator-(Vector4 v) {
  Vector4 result = {
    -v.x,
    -v.y,
    -v.z,
    -v.w
  };
  return result;
}

internal inline Vector2Int operator-(Vector2Int v) {
  Vector2Int result = {
    -v.x,
    -v.y
  };
  return result;
}
internal inline Vector3Int operator-(Vector3Int v) {
  Vector3Int result = {
    -v.x,
    -v.y,
    -v.z
  };
  return result;
}
internal inline Vector4Int operator-(Vector4Int v) {
  Vector4Int result = {
    -v.x,
    -v.y,
    -v.z,
    -v.w
  };
  return result;
}

internal inline bool operator==(Vector2 a, Vector2 b) {
  return a.x == b.x &&
      a.y == b.y;
}
internal inline bool operator==(Vector3 a, Vector3 b) {
  return a.x == b.x &&
      a.y == b.y &&
      a.z == b.z;
}
internal inline bool operator==(Vector4 a, Vector4 b) {
  return a.x == b.x &&
      a.y == b.y &&
      a.z == b.z &&
      a.w == b.w;
}

internal inline bool operator==(Vector2Int a, Vector2Int b) {
  return a.x == b.x &&
      a.y == b.y;
}
internal inline bool operator==(Vector3Int a, Vector3Int b) {
  return a.x == b.x &&
      a.y == b.y &&
      a.z == b.z;
}
internal inline bool operator==(Vector4Int a, Vector4Int b) {
  return a.x == b.x &&
      a.y == b.y &&
      a.z == b.z &&
      a.w == b.w;
}

internal inline bool operator!=(Vector2 a, Vector2 b) {
  return !(a==b);
}
internal inline bool operator!=(Vector3 a, Vector3 b) {
  return !(a==b);
}
internal inline bool operator!=(Vector4 a, Vector4 b) {
  return !(a==b);
}

internal inline bool operator!=(Vector2Int a, Vector2Int b) {
  return !(a==b);
}
internal inline bool operator!=(Vector3Int a, Vector3Int b) {
  return !(a==b);
}
internal inline bool operator!=(Vector4Int a, Vector4Int b) {
  return !(a==b);
}

internal inline Vector2 operator+(Vector2 a, Vector2 b) {
  Vector2 result = {
    a.x + b.x,
    a.y + b.y
  };
  return result;
}

internal inline Vector3 operator+(Vector3 a, const Vector3 b) {
  Vector3 result = {
    a.x + b.x,
    a.y + b.y,
    a.z + b.z
  };
  return result;
}

internal inline Vector4 operator+(Vector4 a, Vector4 b) {
  Vector4 result = {
    a.x + b.x,
    a.y + b.y,
    a.z + b.z,
    a.w + b.w
  };
  return result;
}

internal inline Vector2Int operator+(Vector2Int a, Vector2Int b) {
  Vector2Int result = {
    a.x + b.x,
    a.y + b.y
  };
  return result;
}

internal inline Vector3Int operator+(Vector3Int a, const Vector3Int b) {
  Vector3Int result = {
    a.x + b.x,
    a.y + b.y,
    a.z + b.z
  };
  return result;
}

internal inline Vector4Int operator+(Vector4Int a, Vector4Int b) {
  Vector4Int result = {
    a.x + b.x,
    a.y + b.y,
    a.z + b.z,
    a.w + b.w
  };
  return result;
}



internal inline Vector2 operator-(Vector2 a, Vector2 b) {
  Vector2 result = {
    a.x - b.x,
    a.y - b.y
  };
  return result;
}

internal inline Vector3 operator-(Vector3 a, const Vector3 b) {
  Vector3 result = {
    a.x - b.x,
    a.y - b.y,
    a.z - b.z
  };
  return result;
}

internal inline Vector4 operator-(Vector4 a, Vector4 b) {
  Vector4 result = {
    a.x - b.x,
    a.y - b.y,
    a.z - b.z,
    a.w - b.w
  };
  return result;
}

internal inline Vector2Int operator-(Vector2Int a, Vector2Int b) {
  Vector2Int result = {
    a.x - b.x,
    a.y - b.y
  };
  return result;
}

internal inline Vector3Int operator-(Vector3Int a, const Vector3Int b) {
  Vector3Int result = {
    a.x - b.x,
    a.y - b.y,
    a.z - b.z
  };
  return result;
}

internal inline Vector4Int operator-(Vector4Int a, Vector4Int b) {
  Vector4Int result = {
    a.x - b.x,
    a.y - b.y,
    a.z - b.z,
    a.w - b.w
  };
  return result;
}

internal inline Vector2 operator*(Vector2 v, f32 s) {
  Vector2 result = {
    s * v.x,
    s * v.y,
  };
  return result;
}
internal inline Vector2 operator*(f32 s, Vector2 v) {
  Vector2 result = {
    s * v.x,
    s * v.y,
  };
  return result;
}

internal inline Vector3 operator*(Vector3 v, f32 s) {
  Vector3 result = {
    s * v.x,
    s * v.y,
    s * v.z
  };
  return result;
}
internal inline Vector3 operator*(f32 s, Vector3 v) {
  Vector3 result = {
    s * v.x,
    s * v.y,
    s * v.z
  };
  return result;
}

internal inline Vector4 operator*(Vector4 v, f32 s) {
  Vector4 result = {
    s * v.x,
    s * v.y,
    s * v.z,
    s * v.w
  };
  return result;
}
internal inline Vector4 operator*(f32 s, Vector4 v) {
  Vector4 result = {
    s * v.x,
    s * v.y,
    s * v.z,
    s * v.w
  };
  return result;
}

internal inline Vector2& operator+=(Vector2 &a, Vector2 b) {
  a = a + b;
  return a;
}
internal inline Vector3& operator+=(Vector3 &a, Vector3 b) {
  a = a + b;
  return a;
}
internal inline Vector4& operator+=(Vector4 &a, Vector4 b) {
  a = a + b;
  return a;
}

internal inline Vector2Int& operator+=(Vector2Int &a, Vector2Int b) {
  a = a + b;
  return a;
}
internal inline Vector3Int& operator+=(Vector3Int &a, Vector3Int b) {
  a = a + b;
  return a;
}
internal inline Vector4Int& operator+=(Vector4Int &a, Vector4Int b) {
  a = a + b;
  return a;
}

internal inline Vector2& operator-=(Vector2 &a, Vector2 b) {
  a = a - b;
  return a;
}
internal inline Vector3& operator-=(Vector3 &a, Vector3 b) {
  a = a - b;
  return a;
}
internal inline Vector4& operator-=(Vector4 &a, Vector4 b) {
  a = a - b;
  return a;
}

internal inline Vector2 mix(Vector2 start, Vector2 end, f32 t) {
  Vector2 result;
  result.x = ClampTop(start.x + (end.x - start.x) * t, end.x);
  result.y = ClampTop(start.y + (end.y - start.y) * t, end.y);
  return result;
}
internal inline Vector3 mix(Vector3 start, Vector3 end, f32 t) {
  Vector3 result;
  result.x = ClampTop(start.x + (end.x - start.x) * t, end.x);
  result.y = ClampTop(start.y + (end.y - start.y) * t, end.y);
  result.z = ClampTop(start.z + (end.z - start.z) * t, end.z);
  return result;
}
internal inline Vector4 mix(Vector4 start, Vector4 end, f32 t) {
  Vector4 result;
  result.x = ClampTop(start.x + (end.x - start.x) * t, end.x);
  result.y = ClampTop(start.y + (end.y - start.y) * t, end.y);
  result.z = ClampTop(start.z + (end.z - start.z) * t, end.z);
  result.w = ClampTop(start.w + (end.w - start.w) * t, end.w);
  return result;
}

internal inline f32 dot(Vector2 a, Vector2 b) {
  f32 result = a.x * b.x + a.y * b.y;
  return result;
}
internal inline f32 dot(Vector3 a, Vector3 b) {
  f32 result = a.x * b.x + a.y * b.y + a.z * b.z;
  return result;
}
internal inline f32 dot(Vector4 a, Vector4 b) {
  f32 result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
  return result;
}

internal inline f32 magnitude(Vector2 v) {
  f32 result = sqrtf(v.x * v.x + v.y * v.y);
  return result;
}
internal inline f32 magnitude(Vector3 v) {
  f32 result = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
  return result;
}
internal inline f32 magnitude(Vector4 v) {
  f32 result = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
  return result;
}

internal inline f32 magnitude(Vector2Int v) {
  f32 result = sqrtf((f32)(v.x * v.x + v.y * v.y));
  return result;
}
internal inline f32 magnitude(Vector3Int v) {
  f32 result = sqrtf((f32)(v.x * v.x + v.y * v.y + v.z * v.z));
  return result;
}
internal inline f32 magnitude(Vector4Int v) {
  f32 result = sqrtf((f32)(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w));
  return result;
}

internal inline f32 magnitude2(Vector2 v) {
  f32 result = v.x * v.x + v.y * v.y;
  return result;
}
internal inline f32 magnitude2(Vector3 v) {
  f32 result = v.x * v.x + v.y * v.y + v.z * v.z;
  return result;
}
internal inline f32 magnitude2(Vector4 v) {
  f32 result = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
  return result;
}

internal inline f32 magnitude2(Vector2Int v) {
  f32 result = (f32)(v.x * v.x + v.y * v.y);
  return result;
}
internal inline f32 magnitude2(Vector3Int v) {
  f32 result = (f32)(v.x * v.x + v.y * v.y + v.z * v.z);
  return result;
}
internal inline f32 magnitude2(Vector4Int v) {
  f32 result = (f32)(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
  return result;
}

internal inline Vector3 cross(Vector3 a, Vector3 b) {
  Vector3 result = {
    (a.y * b.z) - (a.z * b.y),
    (a.z * b.x) - (a.x * b.z),
    (a.x * b.y) - (a.y * b.x)
  };
  return result;
}

internal inline Vector3 normalize(Vector3 v) {
  Vector3 result = {0, 0, 0};
  f32 mag = magnitude(v);
  if (mag != 0) {
    result.x = v.x / mag;
    result.y = v.y / mag;
    result.z = v.z / mag;
  }
  return result;
} 
internal inline Vector4 normalize(Vector4 v) {
  Vector4 result = {0, 0, 0, 0};
  f32 mag = magnitude(v);
  if (mag != 0) {
    result.x = v.x / mag;
    result.y = v.y / mag;
    result.z = v.z / mag;
    result.w = v.w / mag;
  }
  return result;
} 

internal inline Vector3 projection(Vector3 a, Vector3 b) {
  Vector3 result;
  f32 d = dot(a, b);
  f32 m = magnitude(b);
  result = d / (m * m) * b;
  return result;
}

internal inline Matrix4 make_matrix4(f32 d) {
  Matrix4 result;
  memset(&result, 0, sizeof(Matrix4));
  result._00 = d;
  result._11 = d;
  result._22 = d;
  result._33 = d;
  return result;
}

internal inline Matrix4 transpose(Matrix4 m) {
  Matrix4 result = m;
  result._01 = m._10;
  result._02 = m._20;
  result._03 = m._30;
  result._10 = m._01;
  result._12 = m._21;
  result._13 = m._31;
  result._20 = m._02;
  result._21 = m._12;
  result._23 = m._32;
  result._30 = m._03;
  result._31 = m._13;
  result._32 = m._23;
  return result;
}

internal inline Matrix3 inverse(Matrix3 m) {
}

internal inline Matrix3 transpose(Matrix3 m) {
  Matrix3 result = m;
  result._01 = m._10;
  result._02 = m._20;
  result._10 = m._01;
  result._12 = m._21;
  result._20 = m._02;
  result._21 = m._12;
  return result;
}

internal bool inverse(const float m[16], float invOut[16]) {
  float inv[16], det;
  int i;

  inv[0] = m[5]  * m[10] * m[15] - 
      m[5]  * m[11] * m[14] - 
      m[9]  * m[6]  * m[15] + 
      m[9]  * m[7]  * m[14] +
      m[13] * m[6]  * m[11] - 
      m[13] * m[7]  * m[10];

  inv[4] = -m[4]  * m[10] * m[15] + 
      m[4]  * m[11] * m[14] + 
      m[8]  * m[6]  * m[15] - 
      m[8]  * m[7]  * m[14] - 
      m[12] * m[6]  * m[11] + 
      m[12] * m[7]  * m[10];

  inv[8] = m[4]  * m[9] * m[15] - 
      m[4]  * m[11] * m[13] - 
      m[8]  * m[5] * m[15] + 
      m[8]  * m[7] * m[13] + 
      m[12] * m[5] * m[11] - 
      m[12] * m[7] * m[9];

  inv[12] = -m[4]  * m[9] * m[14] + 
      m[4]  * m[10] * m[13] +
      m[8]  * m[5] * m[14] - 
      m[8]  * m[6] * m[13] - 
      m[12] * m[5] * m[10] + 
      m[12] * m[6] * m[9];

  inv[1] = -m[1]  * m[10] * m[15] + 
      m[1]  * m[11] * m[14] + 
      m[9]  * m[2] * m[15] - 
      m[9]  * m[3] * m[14] - 
      m[13] * m[2] * m[11] + 
      m[13] * m[3] * m[10];

  inv[5] = m[0]  * m[10] * m[15] - 
      m[0]  * m[11] * m[14] - 
      m[8]  * m[2] * m[15] + 
      m[8]  * m[3] * m[14] + 
      m[12] * m[2] * m[11] - 
      m[12] * m[3] * m[10];

  inv[9] = -m[0]  * m[9] * m[15] + 
      m[0]  * m[11] * m[13] + 
      m[8]  * m[1] * m[15] - 
      m[8]  * m[3] * m[13] - 
      m[12] * m[1] * m[11] + 
      m[12] * m[3] * m[9];

  inv[13] = m[0]  * m[9] * m[14] - 
      m[0]  * m[10] * m[13] - 
      m[8]  * m[1] * m[14] + 
      m[8]  * m[2] * m[13] + 
      m[12] * m[1] * m[10] - 
      m[12] * m[2] * m[9];

  inv[2] = m[1]  * m[6] * m[15] - 
      m[1]  * m[7] * m[14] - 
      m[5]  * m[2] * m[15] + 
      m[5]  * m[3] * m[14] + 
      m[13] * m[2] * m[7] - 
      m[13] * m[3] * m[6];

  inv[6] = -m[0]  * m[6] * m[15] + 
      m[0]  * m[7] * m[14] + 
      m[4]  * m[2] * m[15] - 
      m[4]  * m[3] * m[14] - 
      m[12] * m[2] * m[7] + 
      m[12] * m[3] * m[6];

  inv[10] = m[0]  * m[5] * m[15] - 
      m[0]  * m[7] * m[13] - 
      m[4]  * m[1] * m[15] + 
      m[4]  * m[3] * m[13] + 
      m[12] * m[1] * m[7] - 
      m[12] * m[3] * m[5];

  inv[14] = -m[0]  * m[5] * m[14] + 
      m[0]  * m[6] * m[13] + 
      m[4]  * m[1] * m[14] - 
      m[4]  * m[2] * m[13] - 
      m[12] * m[1] * m[6] + 
      m[12] * m[2] * m[5];

  inv[3] = -m[1] * m[6] * m[11] + 
      m[1] * m[7] * m[10] + 
      m[5] * m[2] * m[11] - 
      m[5] * m[3] * m[10] - 
      m[9] * m[2] * m[7] + 
      m[9] * m[3] * m[6];

  inv[7] = m[0] * m[6] * m[11] - 
      m[0] * m[7] * m[10] - 
      m[4] * m[2] * m[11] + 
      m[4] * m[3] * m[10] + 
      m[8] * m[2] * m[7] - 
      m[8] * m[3] * m[6];

  inv[11] = -m[0] * m[5] * m[11] + 
      m[0] * m[7] * m[9] + 
      m[4] * m[1] * m[11] - 
      m[4] * m[3] * m[9] - 
      m[8] * m[1] * m[7] + 
      m[8] * m[3] * m[5];

  inv[15] = m[0] * m[5] * m[10] - 
      m[0] * m[6] * m[9] - 
      m[4] * m[1] * m[10] + 
      m[4] * m[2] * m[9] + 
      m[8] * m[1] * m[6] - 
      m[8] * m[2] * m[5];

  det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

  if (det == 0)
      return false;

  det = 1.0f / det;

  for (i = 0; i < 16; i++)
      invOut[i] = inv[i] * det;

  return true;
}

internal inline Vector4 linear_combine(Vector4 left, Matrix4 right) {
  Vector4 result;

  result.x = left.elements[0] * right.columns[0].x;
  result.y = left.elements[0] * right.columns[0].y;
  result.z = left.elements[0] * right.columns[0].z;
  result.w = left.elements[0] * right.columns[0].w;

  result.x += left.elements[1] * right.columns[1].x;
  result.y += left.elements[1] * right.columns[1].y;
  result.z += left.elements[1] * right.columns[1].z;
  result.w += left.elements[1] * right.columns[1].w;

  result.x += left.elements[2] * right.columns[2].x;
  result.y += left.elements[2] * right.columns[2].y;
  result.z += left.elements[2] * right.columns[2].z;
  result.w += left.elements[2] * right.columns[2].w;

  result.x += left.elements[3] * right.columns[3].x;
  result.y += left.elements[3] * right.columns[3].y;
  result.z += left.elements[3] * right.columns[3].z;
  result.w += left.elements[3] * right.columns[3].w;

  return result;
}

internal inline Matrix4 translate(f32 x, f32 y, f32 z) {
  Matrix4 result = make_matrix4(1.0f);
  result._30 = x;
  result._31 = y;
  result._32 = z;
  return result;
}

internal inline Matrix4 translate(Vector3 v) {
  Matrix4 result = make_matrix4(1.0f);
  result._30 = v.x;
  result._31 = v.y;
  result._32 = v.z;
  return result;
}

internal inline Matrix4 scale(Vector3 v) {
  Matrix4 result = make_matrix4(1.0f);
  result._00 = v.x;
  result._11 = v.y;
  result._22 = v.z;
  result._33 = 1.0f;
  return result;
}

internal inline Matrix4 look_at_rh(Vector3 eye, Vector3 target, Vector3 up) {
  Vector3 F = normalize(target - eye);
  Vector3 R = normalize(cross(F, up));
  Vector3 U = cross(R, F);

  Matrix4 result;
  result._00 = R.x;
  result._01 = U.x;
  result._02 = -F.x;
  result._03 = 0.f;
  result._10 = R.y;
  result._11 = U.y;
  result._12 = -F.y;
  result._13 = 0.f;
  result._20 = R.z;
  result._21 = U.z;
  result._22 = -F.z;
  result._23 = 0.f;
  result._30 = -dot(R, eye);
  result._31 = -dot(U, eye);
  result._32 =  dot(F, eye);
  result._33 = 1.f;
  return result;
}

internal inline Matrix4 rotate_rh(f32 angle, Vector3 axis) {
  Matrix4 result = make_matrix4(1.0f);
  axis = normalize(axis);

  f32 s = sinf(angle);
  f32 c = cosf(angle);
  f32 c_inv = 1.0f - c;

  result._00 = (c_inv * axis.x * axis.x) + c;
  result._01 = (c_inv * axis.x * axis.y) + (axis.z * s);
  result._02 = (c_inv * axis.x * axis.z) - (axis.y * s);

  result._10 = (c_inv * axis.y * axis.x) - (axis.z * s);
  result._11 = (c_inv * axis.y * axis.y) + c;
  result._12 = (c_inv * axis.y * axis.z) + (axis.x * s);

  result._20 = (c_inv * axis.z * axis.x) + (axis.y * s);
  result._21 = (c_inv * axis.z * axis.y) - (axis.x * s);
  result._22 = (c_inv * axis.z * axis.z) + c;

  return result; 
}

internal inline Matrix4 ortho_rh_zo(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) {
  Matrix4 result = make_matrix4(1.0f);
  result._00 = 2.0f / (right - left);
  result._11 = 2.0f / (top - bottom);
  result._22 = 1.0f / (far - near);
  result._30 = - (right + left) / (right - left);
  result._31 = - (top + bottom) / (top - bottom);
  // result._32 = - near / (far - near);
  result._32 = -(near + far) / (far - near);
  return result;
}

internal inline Matrix4 perspective_rh(f32 fov, f32 aspect, f32 near, f32 far) {
  Matrix4 result = make_matrix4(1.0f);
  f32 c = 1.0f / tanf(fov / 2.0f);
  result._00 = c / aspect;
  result._11 = c;
  result._23 = -1.0f;
  result._22 = (far) / (near - far);
  result._32 = (near * far) / (near - far);
  return result;
}

internal inline Matrix4 operator*(Matrix4 a, Matrix4 b) {
  Matrix4 result;
  result.columns[0] = linear_combine(b.columns[0], a);
  result.columns[1] = linear_combine(b.columns[1], a);
  result.columns[2] = linear_combine(b.columns[2], a);
  result.columns[3] = linear_combine(b.columns[3], a);
  return result;
}

internal inline Vector4 operator*(Matrix4 m, Vector4 v) {
  Vector4 result;
  result.elements[0] = v.elements[0] * m._00 + v.elements[1] * m._01 + v.elements[2] * m._02 + v.elements[3] * m._03;
  result.elements[1] = v.elements[0] * m._10 + v.elements[1] * m._11 + v.elements[2] * m._12 + v.elements[3] * m._13;
  result.elements[2] = v.elements[0] * m._20 + v.elements[1] * m._21 + v.elements[2] * m._22 + v.elements[3] * m._23;
  result.elements[3] = v.elements[0] * m._30 + v.elements[1] * m._31 + v.elements[2] * m._32 + v.elements[3] * m._33;
  return result;
}

internal inline Vector3 get_nearest_axis(Vector3 v) {
  Vector3 result = Vector3(0, 0, 0);
  f32 x = Abs(v.x);
  f32 y = Abs(v.y);
  f32 z = Abs(v.z);
  if (x > y) {
    if (x > z) {
      result.x = v.x < 0 ? -1.0f : 1.0f;
    } else {
      result.z = v.z < 0 ? -1.0f : 1.0f;
    }
  } else {
    if (y > z) {
      result.y = v.y < 0 ? -1.0f : 1.0f;
    } else {
      result.z = v.z < 0 ? -1.0f : 1.0f;
    }
  }
  return result;
}

internal inline Vector3 forward_from_theta(f32 theta) {
  Vector4 forward = rotate_rh(theta, Vector3(0, 1, 0)) * Vector4(1, 0, 0, 1);
  Vector3 result = to_vector3(forward);
  return result;
}
