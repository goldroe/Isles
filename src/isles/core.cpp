global Viewport *g_viewport;

internal inline Viewport *get_viewport() {
  return g_viewport;
}

void Byte_Buffer::advance() {
  Assert(ptr < len);
  ptr++;
}

void Byte_Buffer::put_byte(u8 b) {
  if (capacity < len + 1) {
    size_t new_capacity = capacity * 2 + 1;
    capacity = new_capacity;
    buffer = (u8 *)realloc(buffer, new_capacity);
  }
  buffer[len] = b;
  len++;
}

void Byte_Buffer::put_le16(u16 v) {
  u8 b0 = v & 0xFF;
  u8 b1 = (v>>8) & 0xFF;
  put_byte(b0);
  put_byte(b1);
}

void Byte_Buffer::put_le32(u32 v) {
  u16 b0 = v & 0xFFFF;
  u16 b1 = (v>>16) & 0xFFFF;
  put_le16(b0);
  put_le16(b1);
}

void Byte_Buffer::put_le64(u64 v) {
  u32 b0 = v & 0xFFFFFFFF;
  u32 b1 = (v>>32) & 0xFFFFFFFF;
  put_le32(b0);
  put_le32(b1);
}

void Byte_Buffer::put_f32(f32 v) {
  put_byte(0);
  put_byte(0);
  put_byte(0);
  put_byte(0);
  void *p = (void *)(buffer + len - 4);
  memcpy(p, &v, 4);
}

void Byte_Buffer::put_string(String8 string) {
  for (int i = 0; i < string.count; i++) {
    put_byte(string.data[i]);
  }
} 

u8 Byte_Buffer::get_byte() {
  u8 result = buffer[ptr];
  advance();
  return result;
}

u16 Byte_Buffer::get_le16() {
  u8 b0 = get_byte();
  u8 b1 = get_byte();
  u16 result = ((u16)b1<<8) | b0;
  return result;
}

u32 Byte_Buffer::get_le32() {
  u16 b0 = get_le16();
  u16 b1 = get_le16();
  u32 result = ((u32)b1 << 16) | b0;
  return result;
}

u64 Byte_Buffer::get_le64() {
  u32 b0 = get_le32();
  u32 b1 = get_le32();
  u64 result = ((u64)b1 << 32) | b0;
  return result;
}

f32 Byte_Buffer::get_f32() {
  f32 result = 0;
  void *p = buffer + ptr;
  memcpy(&result, p, 4);
  advance();
  advance();
  advance();
  advance();
  return result;
}

internal std::string get_parent_path(std::string file_name) {
  std::string parent = file_name.substr(0, file_name.find_last_of("/\\"));
  return parent;
}

internal Vector3 vector_from_axis(Axis axis) {
  switch (axis) {
  default:
    return Vector3();
  case AXIS_X:
    return Vector3(1, 0, 0);
  case AXIS_Y:
    return Vector3(0, 1, 0);
  case AXIS_Z:
    return Vector3(0, 0, 1);
  } 
}

internal void update_camera_matrix(Camera *camera) {
  Vector3 target = camera->origin + camera->forward;
  camera->view_matrix = look_at_rh(camera->origin, target, camera->up);
}

void Camera::update_euler_angles(f32 Yaw, f32 Pitch) {
  yaw = Yaw;
  pitch = Pitch;
  Vector3 F;
  F.x = cosf(yaw) * cosf(pitch);
  F.y = sinf(pitch);
  F.z = sinf(yaw) * cosf(pitch);
  forward = normalize(F);
  right = normalize(cross(forward, up));

  update_camera_matrix(this);
}

internal void update_camera_orientation(Camera *camera, Vector2 delta) {
  if (delta.x || delta.y) {
    f32 rot_speed = 0.08f;
    camera->yaw += delta.x * rot_speed * get_frame_delta();
    camera->yaw = (f32)fmod(camera->yaw, PI * 2.0f);
    camera->pitch -= rot_speed * delta.y * get_frame_delta();
    camera->pitch = Clamp(camera->pitch, -PI * 0.49f, PI * 0.49f);

    Vector3 forward;
    forward.x = cosf(camera->yaw) * cosf(camera->pitch);
    forward.y = sinf(camera->pitch);
    forward.z = sinf(camera->yaw) * cosf(camera->pitch);

    camera->forward = normalize(forward);
    camera->right = normalize(cross(camera->forward, camera->up));

    update_camera_matrix(camera);
  }
}

internal void update_camera_position(Camera *camera, f32 forward_dt, f32 right_dt, f32 up_dt) {
  f32 camera_speed = 15.0f;
  Vector3 direction = normalize(camera->forward * forward_dt + camera->right * right_dt + camera->up * up_dt);
  Vector3 new_origin = camera->origin + direction * camera_speed * get_frame_delta();
  camera->origin = new_origin;
  update_camera_matrix(camera);
}


internal Vector3 get_mouse_ray(Camera camera, Vector2Int mouse_position, Vector2 window_dim) {
  //@Note Picking
  Vector3 ray = {0, 0, 0};
  // I know the camera position, the camera "target" position, camera "up" vector (simply set to [0,1,0]), and the camera's vertical FOV. These are used for gluLookAt() and GluPerspective(). 
  // So, I did this:
  // 1. camera target minus camera position gives me the vector that the camera is pointing in ("target vector"). I normalize this vector.
  // 2. I get the right-angle vector of the camera by the cross product of camera target vector and camera up vector.
  // 3. I re-calculate the camera's "up" vector by cross product between the camera target vector and the right-angle vector I just calculated. It's no longer simply straight up, but calculated properly.
  // 4. I multiply the right-angle vector by the X/Y ratio of the screen (about 1.77...).
  // 5. I multiply both the up vector and right-angle vectors by the tan() of the vertical FOV divided by 2.
  // 6. I get the ratio of the mouse's X and Y coordinates of the screen compared to the size of the screen. I also offset the coordinate so that the center screen is [0,0].
  // 7. I multiply the up vector by the mouse's Y offset ratio, and likewise multiply the right-angle vector by the mouse's X offset ratio.
  // 8. The vector for the ray is now the target vector plus the up vector plus the right-angle vector.

  // What I'm doing, conceptually, is that I'm constructing a model of the screen 1 unit in front of the camera, facing the camera directly, and "scaled" to exactly cover the FOV of the camera. I'm then simply calculating where on that screen the mouse is. Then I draw a line between the camera and where the mouse is calculated to be on that model screen. It's all done in world-space using vectors.

  // Member function of the Camera:

  // 		XYZf calculateRay(const Mouse& mouse, XY<unsigned short> screenResolution)
  // 		{
  // 			screenResolution /= 2;
  // 			XYi offset = XYi{ mouse.pos.x, mouse.pos.y } - screenResolution;

  // 			XYZf camV = target - pos;
  // 			camV.normalize();

  // 			XYZf camSide = camV.cross(up);
  // 			XYZf camUp = camV.cross(camSide);

  // 			float t = tan((fovY / 2) * 0.0174532925);

  // 			camSide *= ratio;
  // 			camUp *= t;
  // 			camSide *= t;
  // 			camUp *= float(offset.y) / screenResolution.y;
  // 			camSide *= float(offset.x) / screenResolution.x;

  // 			XYZf rayVec = camV + camUp + camSide;
  // 			rayVec.normalize();
  // 			return rayVec;
  // 		}

  float mouse_x = mouse_position.x / (window_dim.x  * 0.5f) - 1.0f;
  float mouse_y = mouse_position.y / (window_dim.y * 0.5f) - 1.0f;
  // mouse_y *= -1;

  Vector3 forward = camera.forward;
  Vector3 right = camera.right;
  Vector3 up = cross(forward, right);

  f32 aspect = (f32)window_dim.x / (f32)window_dim.y;

  f32 t = tanf((camera.fov / 2.0f));

  right = right * aspect;
  up = up * t;
  right = right * t;
  up = up * mouse_y;
  right = right * mouse_x;

  ray = normalize(forward + up + right);
  return ray;
}

internal inline Vector3 get_aabb_center(AABB box) {
  Vector3 result = Vector3(
    box.min.x + 0.5f * (box.max.x - box.min.x),
    box.min.y + 0.5f * (box.max.y - box.min.y),
    box.min.z + 0.5f * (box.max.z - box.min.z));
  return result;
}

internal inline Vector3 get_aabb_dimension(AABB box) {
  Vector3 result = Vector3(
    box.max.x - box.min.x,
    box.max.y - box.min.y,
    box.max.z - box.min.z);
  return result;
}

