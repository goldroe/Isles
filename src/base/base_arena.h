#ifndef BASE_ARENA_H
#define BASE_ARENA_H

internal Arena *make_arena(Base_Allocator *allocator);
internal void *arena_push(Arena *arena, u64 size);
internal void arena_clear(Arena *arena);
internal void arena_pop_to(Arena *arena, u64 pos);

internal Arena_Temp arena_temp_begin(Arena *arena);
internal void arena_temp_end(Arena_Temp temp);

#define push_array(arena, type, count) (type*)MemoryZero((arena_push((arena), sizeof(type) * (count))), sizeof(type)*(count))
#define push_array_no_zero(arena, type, count) (type*)arena_push((arena), sizeof(type) * (count))

#endif // BASE_ARENA_H
