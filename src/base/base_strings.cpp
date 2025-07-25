internal u64 cstr8_length(const char *c) {
    if (c == nullptr) return 0;
    u64 result = 0;
    while (*c++) {
        result++;
    }
    return result;
}

internal String8 str8_zero() {
    String8 result = {0, 0};
    return result;
}

internal String8 str8(u8 *c, u64 count) {
    String8 result = {(u8 *)c, count};
    return result;
}

internal bool str8_equal(String8 first, String8 second) {
  return first.count == second.count && (strncmp((const char *)first.data, (const char *)second.data, first.count) == 0); 
}

bool operator==(String8 first, String8 second) {
  return str8_equal(first, second); 
}

internal String8 str8_cstring(const char *c) {
    String8 result;
    result.count = cstr8_length(c);
    result.data = (u8 *)c;
    return result;
}

internal String8 str8_rng(String8 string, Rng_U64 rng) {
    String8 result;
    result.data = string.data + rng.min;
    result.count = rng.max - rng.min;
    return result;
}

internal String8 str8_copy(Allocator allocator, String8 string) {
    String8 result;
    result.count = string.count;
    result.data = array_alloc(allocator, u8, result.count + 1);
    MemoryCopy(result.data, string.data, string.count);
    result.data[result.count] = 0;
    return result;
}

internal String8 str8_concat(Allocator allocator, String8 first, String8 second) {
    String8 result;
    result.count = first.count + second.count;
    result.data = array_alloc(allocator, u8, result.count + 1);
    MemoryCopy(result.data, first.data, first.count);
    MemoryCopy(result.data + first.count, second.data, second.count);
    result.data[result.count] = 0;
    return result;
}

internal bool str8_match(String8 first, String8 second, String_Match_Flags flags) {
    if (first.count != second.count) return false;
    u8 a, b;
    for (u64 i = 0; i < first.count; i++) {
        a = first.data[i];
        b = second.data[i];
        if (flags & StringMatchFlag_CaseInsensitive) {
            a = (u8)tolower(a);
            b = (u8)tolower(b);
        }
        if (a != b) {
            return false;
        }
    }
    return true;
}

internal String8 str8_pushfv(Allocator allocator, const char *fmt, va_list args) {
    va_list args_;
    va_copy(args_, args);
    String8 result;
    int bytes = stbsp_vsnprintf(NULL, NULL, fmt, args_) + 1;
    result.data = array_alloc(allocator, u8, bytes);
    result.count = stbsp_vsnprintf((char *)result.data, bytes, fmt, args_);
    result.data[result.count] = 0;
    va_end(args_);
    return result;
}

internal String8 str8_pushf(Allocator allocator, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    String8 result;
    int bytes = stbsp_vsnprintf(NULL, NULL, fmt, args) + 1;
    result.data = array_alloc(allocator, u8, bytes);
    result.count = stbsp_vsnprintf((char *)result.data, bytes, fmt, args);
    result.data[result.count] = 0;
    va_end(args);
    return result;
}

internal String8 str8_jump(String8 string, u64 count) {
    String8 result;
    result.data = string.data + count;
    result.count = string.count - count;
    return result;
}

internal u64 str8_find_substr(String8 string, String8 substring) {
    if (substring.count > string.count) {
        return string.count;
    }

    u64 result = string.count;
    for (u64 string_cursor = 0; string_cursor < string.count; string_cursor++) {
        u64 rem = string.count - string_cursor;
        if (rem < substring.count) {
            break;
        }
        
        if (string.data[string_cursor] == substring.data[0]) {
            int cmp = memcmp(string.data + string_cursor, substring.data, substring.count);
            if (cmp == 0) {
                result = string_cursor;
                break;
            }
        }
    }
    return result;
}

internal u64 djb2_hash(const char *str) {
  u64 hash = 5381;
  int c;
  while (c = *str++) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

internal u64 djb2_hash_string(String8 string) {
    u64 result = 5381;
    for (u64 i = 0; i < string.count; i++) {
        result = ((result << 5) + result) + string.data[i];
    }
    return result;
}


internal cstring cstring_alloc(u64 len) {
  cstring_header *header = (cstring_header *)malloc(offsetof(cstring_header, data) + len + 1);
  header->len = len;
  header->cap = len;
  return header->data;
}

internal void cstring_free(cstring str) {
  if (str) {
    cstring_header *header = CSTRING_HEADER(str);
    free(header);
  }
}

//@Note Cstring
internal cstring make_cstring_len(const char *str, u64 len) {
  cstring result = cstring_alloc(len);
  MemoryCopy(result, str, len);
  result[len] = 0;
  return result;
}

internal cstring make_cstring(const char *str) {
    u64 len = str ? strlen(str) : 0;
    return make_cstring_len(str, len);
}

internal void cstring__set_len(cstring string, u64 len) {
    cstring_header *header = CSTRING_HEADER(string);
    header->len = len;
}

internal void cstring__set_cap(cstring string, u64 cap) {
    cstring_header *header = CSTRING_HEADER(string);
    header->cap = cap;
}

internal cstring cstring__append(cstring string, const char *s) {
    cstring result = string;
    cstring_header *old_header = CSTRING_HEADER(string);
    u64 len = old_header ? old_header->len : 0;
    u64 s_len = s ? strlen(s) : 0;
    u64 new_len = len + s_len;
    u64 cap = old_header ? old_header->cap : 0;
    u64 new_cap = cap;
    if (len + s_len > cap) {
        new_cap = new_len * 2 + 1;
        cstring_header *new_header = (cstring_header *)realloc(old_header, offsetof(cstring_header, data) + new_cap + 1);
        result = (cstring)new_header->data;
    }

    cstring__set_len(result, new_len);
    cstring__set_cap(result, new_cap);
    MemoryCopy(result + len, s, s_len);
    result[new_len] = 0;
    return result;
}

internal cstring cstring_append(cstring string, const char *s) {
    cstring result;
    if (string) {
        result = cstring__append(string, s);
    } else {
        result = make_cstring(s);
    }
    return result;
}

internal void cstring_append(cstring *string, const char *s) {
    cstring result = cstring_append(*string, s);
    *string = result;
}

internal cstring cstring_format_va(const char *fmt, va_list args) {
    va_list args_;
    va_copy(args_, args);

    int len = stbsp_vsnprintf(NULL, NULL, fmt, args_);
    cstring result = cstring_alloc(len);
    stbsp_vsnprintf(result, len + 1, fmt, args_);
    result[len] = 0;

    va_end(args_);
    return result;
}

internal cstring cstring_fmt(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int len = stbsp_vsnprintf(NULL, NULL, fmt, args);
    cstring result = cstring_alloc(len);
    stbsp_vsnprintf(result, len, fmt, args);
    result[len] = 0;

    va_end(args);
    return result;
}

internal cstring cstring_append_fmt(cstring string, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len = stbsp_vsnprintf(NULL, NULL, fmt, args);
    char *str_fmt = (char *)malloc(len + 1);
    stbsp_vsnprintf(str_fmt, len + 1, fmt, args);
    va_end(args);
    cstring result = cstring__append(string, str_fmt);
    free(str_fmt);
    return result;
}

internal void cstring_append_fmt(cstring *string, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int len = stbsp_vsnprintf(NULL, NULL, fmt, args);
    char *str_fmt = (char *)malloc(len + 1);
    stbsp_vsnprintf(str_fmt, len + 1, fmt, args);
    va_end(args);
    cstring result = cstring__append(*string, str_fmt);
    *string = result;
    free(str_fmt);
}
