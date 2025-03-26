global Draw_State *g_draw_state;

internal void draw_set_state(Draw_State *state) {
    g_draw_state = state;
    state->arena = NULL;
}

internal inline Draw_Bucket *draw_top() {
    return g_draw_state->bucket_list.last;
}

internal Draw_Bucket *draw_push() {
    Draw_Bucket *bucket = push_array(g_draw_state->arena, Draw_Bucket, 1);
    DLLPushBack(g_draw_state->bucket_list.first, g_draw_state->bucket_list.last, bucket, next, prev);
    bucket->stack_gen = g_draw_state->next_stack_gen++;
    return bucket;
}

internal void draw_begin(OS_Handle window_handle) {
    if (!g_draw_state->arena) {
        g_draw_state->arena = arena_alloc(get_virtual_allocator(), MB(4));
    }

    arena_clear(g_draw_state->arena);

    g_draw_state->bucket_list.first = nullptr;
    g_draw_state->bucket_list.last = nullptr;
    g_draw_state->bucket_list.count = 0;
    g_draw_state->next_stack_gen = 0;
}

internal void draw_end() {
}

internal inline void draw_set_texture(Texture *texture) {
    Draw_Bucket *top = draw_top();
    if (top->kind == DRAW_BUCKET_IMMEDIATE) {
        Draw_Immediate_Batch *batch = top->immediate.batches.last;
        if (batch == nullptr) {
            batch = push_array(g_draw_state->arena, Draw_Immediate_Batch, 1);
            SLLQueuePush(top->immediate.batches.first, top->immediate.batches.last, batch);
            top->immediate.batches.count++;
        } else if (batch->texture != texture && batch->vertices.count) {
            batch = push_array(g_draw_state->arena, Draw_Immediate_Batch, 1);
            SLLQueuePush(top->immediate.batches.first, top->immediate.batches.last, batch);
            top->immediate.batches.count++;
        }
        batch->texture = texture;
    }
}

internal inline void draw_immediate_begin() {
    Draw_Bucket *bucket = draw_push();
    bucket->kind = DRAW_BUCKET_IMMEDIATE;
}

internal inline Draw_Immediate_Batch *push_immediate_batch(Draw_Bucket *bucket) {
    Draw_Immediate_Batch *batch = push_array(g_draw_state->arena, Draw_Immediate_Batch, 1);
    SLLQueuePush(bucket->immediate.batches.first, bucket->immediate.batches.last, batch);
    return batch;
}

internal inline void draw_immediate_vertex(Draw_Immediate_Batch *batch, Vector3 position, Vector3 normal, Vector4 color, Vector2 uv) {
    Vertex_3D vertex;
    vertex.position = position;
    vertex.normal = normal;
    vertex.color = color;
    vertex.uv = uv;
    batch->vertices.push(vertex);
}

internal void draw_rectangle(Draw_Immediate_Batch *batch, Vector3 position, Vector3 size, Vector4 color) {
    Vector3 half = 0.5f * size;
    Vector2 uv = {0, 0};
    Vector3 normal = {0, 0, 0};
    Vector3 p0 = Vector3(position.x,          position.y,          position.z + size.z);
    Vector3 p1 = Vector3(position.x + size.x, position.y,          position.z + size.z);
    Vector3 p2 = Vector3(position.x + size.x, position.y + size.y, position.z + size.z);
    Vector3 p3 = Vector3(position.x,          position.y + size.y, position.z + size.z);
    Vector3 p4 = Vector3(position.x + size.x, position.y,          position.z);
    Vector3 p5 = Vector3(position.x,          position.y,          position.z);
    Vector3 p6 = Vector3(position.x,          position.y + size.y, position.z);
    Vector3 p7 = Vector3(position.x + size.x, position.y + size.y, position.z);

    // front
    draw_immediate_vertex(batch, p0, normal, color, uv);
    draw_immediate_vertex(batch, p1, normal, color, uv);
    draw_immediate_vertex(batch, p2, normal, color, uv);
    draw_immediate_vertex(batch, p0, normal, color, uv);
    draw_immediate_vertex(batch, p2, normal, color, uv);
    draw_immediate_vertex(batch, p3, normal, color, uv);

    // back
    draw_immediate_vertex(batch, p4, normal, color, uv);
    draw_immediate_vertex(batch, p5, normal, color, uv);
    draw_immediate_vertex(batch, p6, normal, color, uv);
    draw_immediate_vertex(batch, p4, normal, color, uv);
    draw_immediate_vertex(batch, p6, normal, color, uv);
    draw_immediate_vertex(batch, p7, normal, color, uv);

    // left
    draw_immediate_vertex(batch, p5, normal, color, uv);
    draw_immediate_vertex(batch, p0, normal, color, uv);
    draw_immediate_vertex(batch, p3, normal, color, uv);
    draw_immediate_vertex(batch, p5, normal, color, uv);
    draw_immediate_vertex(batch, p3, normal, color, uv);
    draw_immediate_vertex(batch, p6, normal, color, uv);

    // right
    draw_immediate_vertex(batch, p1, normal, color, uv);
    draw_immediate_vertex(batch, p4, normal, color, uv);
    draw_immediate_vertex(batch, p7, normal, color, uv);
    draw_immediate_vertex(batch, p1, normal, color, uv);
    draw_immediate_vertex(batch, p7, normal, color, uv);
    draw_immediate_vertex(batch, p2, normal, color, uv);

    // top
    draw_immediate_vertex(batch, p3, normal, color, uv);
    draw_immediate_vertex(batch, p2, normal, color, uv);
    draw_immediate_vertex(batch, p7, normal, color, uv);
    draw_immediate_vertex(batch, p3, normal, color, uv);
    draw_immediate_vertex(batch, p7, normal, color, uv);
    draw_immediate_vertex(batch, p6, normal, color, uv);

    // bottom
    draw_immediate_vertex(batch, p1, normal, color, uv);
    draw_immediate_vertex(batch, p0, normal, color, uv);
    draw_immediate_vertex(batch, p5, normal, color, uv);
    draw_immediate_vertex(batch, p1, normal, color, uv);
    draw_immediate_vertex(batch, p5, normal, color, uv);
    draw_immediate_vertex(batch, p4, normal, color, uv);
}

internal void draw_rectangle_center(Draw_Immediate_Batch *batch, Vector3 center, Vector3 size, Vector4 color) {
  Vector3 half_size = size * 0.5f;
  draw_rectangle(batch, center - half_size, size, color);
}

internal void draw_cube(Draw_Immediate_Batch *batch, Vector3 center, f32 size, Vector4 color) {
    f32 half = 0.5f * size;
    Vector2 uv = {0, 0};
    Vector3 normal = {0, 0, 0};
    Vector3 p0 = Vector3(center.x - half, center.y - half, center.z + half);
    Vector3 p1 = Vector3(center.x + half, center.y - half, center.z + half);
    Vector3 p2 = Vector3(center.x + half, center.y + half, center.z + half);
    Vector3 p3 = Vector3(center.x - half, center.y + half, center.z + half);
    Vector3 p4 = Vector3(center.x + half, center.y - half, center.z - half);
    Vector3 p5 = Vector3(center.x - half, center.y - half, center.z - half);
    Vector3 p6 = Vector3(center.x - half, center.y + half, center.z - half);
    Vector3 p7 = Vector3(center.x + half, center.y + half, center.z - half);

    // front
    draw_immediate_vertex(batch, p0, normal, color, uv);
    draw_immediate_vertex(batch, p1, normal, color, uv);
    draw_immediate_vertex(batch, p2, normal, color, uv);
    draw_immediate_vertex(batch, p0, normal, color, uv);
    draw_immediate_vertex(batch, p2, normal, color, uv);
    draw_immediate_vertex(batch, p3, normal, color, uv);

    // back
    draw_immediate_vertex(batch, p4, normal, color, uv);
    draw_immediate_vertex(batch, p5, normal, color, uv);
    draw_immediate_vertex(batch, p6, normal, color, uv);
    draw_immediate_vertex(batch, p4, normal, color, uv);
    draw_immediate_vertex(batch, p6, normal, color, uv);
    draw_immediate_vertex(batch, p7, normal, color, uv);

    // left
    draw_immediate_vertex(batch, p5, normal, color, uv);
    draw_immediate_vertex(batch, p0, normal, color, uv);
    draw_immediate_vertex(batch, p3, normal, color, uv);
    draw_immediate_vertex(batch, p5, normal, color, uv);
    draw_immediate_vertex(batch, p3, normal, color, uv);
    draw_immediate_vertex(batch, p6, normal, color, uv);

    // right
    draw_immediate_vertex(batch, p1, normal, color, uv);
    draw_immediate_vertex(batch, p4, normal, color, uv);
    draw_immediate_vertex(batch, p7, normal, color, uv);
    draw_immediate_vertex(batch, p1, normal, color, uv);
    draw_immediate_vertex(batch, p7, normal, color, uv);
    draw_immediate_vertex(batch, p2, normal, color, uv);

    // top
    draw_immediate_vertex(batch, p3, normal, color, uv);
    draw_immediate_vertex(batch, p2, normal, color, uv);
    draw_immediate_vertex(batch, p7, normal, color, uv);
    draw_immediate_vertex(batch, p3, normal, color, uv);
    draw_immediate_vertex(batch, p7, normal, color, uv);
    draw_immediate_vertex(batch, p6, normal, color, uv);

    // bottom
    draw_immediate_vertex(batch, p1, normal, color, uv);
    draw_immediate_vertex(batch, p0, normal, color, uv);
    draw_immediate_vertex(batch, p5, normal, color, uv);
    draw_immediate_vertex(batch, p1, normal, color, uv);
    draw_immediate_vertex(batch, p5, normal, color, uv);
    draw_immediate_vertex(batch, p4, normal, color, uv);
}
