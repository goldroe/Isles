#include "font.h"

global Font *default_fonts[FONT_COUNT];

internal Font *load_font(Arena *arena, String8 font_name, int font_height) {
    FT_Library ft_lib;
    int err = FT_Init_FreeType(&ft_lib);
    if (err) {
        printf("Error initializing freetype library: %d\n", err);
    }

    FT_Face ft_face;
    err = FT_New_Face(ft_lib, (const char *)font_name.data, 0, &ft_face);
    if (err == FT_Err_Unknown_File_Format) {
        printf("Format not supported\n");
    } else if (err) {
        printf("Font file could not be read\n");
    }

    err = FT_Set_Pixel_Sizes(ft_face, 0, font_height);
    if (err) {
        printf("Error setting pixel sizes of font\n");
    }

    Font *font = push_array(arena, Font, 1);

    int atlas_width = 1024;
    int atlas_height = 1024;

    int bbox_ymax = FT_MulFix(ft_face->bbox.yMax, ft_face->size->metrics.y_scale) >> 6;
    int bbox_ymin = FT_MulFix(ft_face->bbox.yMin, ft_face->size->metrics.y_scale) >> 6;
    int height = bbox_ymax - bbox_ymin;
    float ascend = ft_face->size->metrics.ascender / 64.f;
    float descend = ft_face->size->metrics.descender / 64.f;
    float bbox_height = (float)(bbox_ymax - bbox_ymin);
    float glyph_height = (float)ft_face->size->metrics.height / 64.f;
    float glyph_width = (float)(ft_face->size->metrics.max_advance) / 64.f;

    int glyphs_count = 0;
    stbrp_rect *glyph_rects = new stbrp_rect[1024];
    int max_bmp_height = 0;

    for (u32 c = 0, i = 0; c < 2048 && i < 1024; c++) {
        if (FT_Load_Char(ft_face, c, FT_LOAD_RENDER)) {
            printf("Error loading char %c\n", c);
            continue;
        } else {
            i += 1;
        }

        int width = ft_face->glyph->bitmap.width;
        int height = (int)ft_face->glyph->bitmap.rows;

        int bmp_height = ft_face->glyph->bitmap.rows + ft_face->glyph->bitmap_top;
        if (max_bmp_height < bmp_height) {
            max_bmp_height = bmp_height;
        }

        stbrp_rect rect = {};
        rect.id = (int)c;
        rect.w = width;
        rect.h = bmp_height;
        glyph_rects[glyphs_count] = rect;
        glyphs_count++;
    }

    stbrp_node *glyph_rect_nodes = new stbrp_node[glyphs_count];

    stbrp_context context;
    stbrp_init_target(&context, 1024, 1024, glyph_rect_nodes, glyphs_count);

    bool packed = stbrp_pack_rects(&context, glyph_rects, glyphs_count);

    delete [] glyph_rect_nodes;

    FT_GlyphSlot slot = ft_face->glyph;

    Arena_Temp temp = arena_temp_begin(arena);
    // Pack glyph bitmaps
    int pixel_size = 4;
    u8 *bitmap = push_array(arena, u8, pixel_size * atlas_width * atlas_height);

    for (int i = 0; i < glyphs_count; i++) {
      stbrp_rect rect = glyph_rects[i];
      if (!rect.was_packed) continue;

      int c = rect.id;

      err = FT_Load_Char(ft_face, c, FT_LOAD_DEFAULT);
      if (err) {
        printf("Error loading glyph '%c': %d\n", c, err);
        continue;
      }

      err = FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL);
      if (err) {
        printf("Error rendering glyph '%c': %d\n", c, err);
        continue;
      }

      Glyph *glyph = &font->glyphs[c];
      glyph->ax = (float)(ft_face->glyph->advance.x >> 6);
      glyph->ay = (float)(ft_face->glyph->advance.y >> 6);
      glyph->bx = (float)ft_face->glyph->bitmap.width;
      glyph->by = (float)ft_face->glyph->bitmap.rows;
      glyph->bt = (float)ft_face->glyph->bitmap_top;
      glyph->bl = (float)ft_face->glyph->bitmap_left;
      glyph->tx = (float)rect.x / atlas_width;
      glyph->ty = (float)rect.y / atlas_height;

      // Write glyph bitmap to atlas
      for (int y = 0; y < glyph->by; y++) {
        for (int x = 0; x < glyph->bx; x++) {
          int Y = y + rect.y;
          int X = x + rect.x;
          u8 *dest = bitmap + (Y * atlas_width + X) * pixel_size;
          u8 *source = ft_face->glyph->bitmap.buffer + y * ft_face->glyph->bitmap.width + x;
          dest[0] = dest[1] = dest[2] = 255;
          dest[3] = *source;
        }
      }
    }

    font->width = atlas_width;
    font->height = atlas_height;
    font->max_bmp_height = max_bmp_height;
    font->ascend = ascend;
    font->descend = descend;
    font->bbox_height = height;
    font->glyph_width = glyph_width;
    font->glyph_height = glyph_height;
    font->texture = create_texture(bitmap, DXGI_FORMAT_R8G8B8A8_UNORM, atlas_width, atlas_height, 0);

    arena_temp_end(temp);

    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft_lib);
    return font;
}


internal Font *load_icon_font(Arena *arena, String8 font_name, int font_height, u32 *glyph_array, int glyph_array_count) {
    FT_Library ft_lib;
    int err = FT_Init_FreeType(&ft_lib);
    if (err) {
        printf("Error initializing freetype library: %d\n", err);
    }

    FT_Face ft_face;
    err = FT_New_Face(ft_lib, (const char *)font_name.data, 0, &ft_face);
    if (err == FT_Err_Unknown_File_Format) {
        printf("Format not supported\n");
    } else if (err) {
        printf("Font file could not be read\n");
    }

    err = FT_Set_Pixel_Sizes(ft_face, 0, font_height);
    if (err) {
        printf("Error setting pixel sizes of font\n");
    }

    Font *font = push_array(arena, Font, 1);

    int atlas_width = 1024;
    int atlas_height = 1024;

    int bbox_ymax = FT_MulFix(ft_face->bbox.yMax, ft_face->size->metrics.y_scale) >> 6;
    int bbox_ymin = FT_MulFix(ft_face->bbox.yMin, ft_face->size->metrics.y_scale) >> 6;
    int height = bbox_ymax - bbox_ymin;
    float ascend = ft_face->size->metrics.ascender / 64.f;
    float descend = ft_face->size->metrics.descender / 64.f;
    float bbox_height = (float)(bbox_ymax - bbox_ymin);
    float glyph_height = (float)ft_face->size->metrics.height / 64.f;
    float glyph_width = (float)(ft_face->size->metrics.max_advance) / 64.f;

    int glyphs_count = 0;
    stbrp_rect *glyph_rects = new stbrp_rect[glyph_array_count];
    int max_bmp_height = 0;
    
    for (int i = 0; i < glyph_array_count; i++) {
      u32 c = glyph_array[i];
      if (FT_Load_Char(ft_face, c, FT_LOAD_RENDER)) {
        printf("Error loading char %c\n", c);
        continue;
      }

      int width = ft_face->glyph->bitmap.width;
      int height = (int)ft_face->glyph->bitmap.rows;

      int bmp_height = ft_face->glyph->bitmap.rows + ft_face->glyph->bitmap_top;
      if (max_bmp_height < bmp_height) {
        max_bmp_height = bmp_height;
      }

      stbrp_rect rect = {};
      rect.id = (int)c;
      rect.w = width;
      rect.h = bmp_height;
      glyph_rects[glyphs_count] = rect;
      glyphs_count++;
    }

    stbrp_node *glyph_rect_nodes = new stbrp_node[glyphs_count];

    stbrp_context context;
    stbrp_init_target(&context, 1024, 1024, glyph_rect_nodes, glyphs_count);

    bool packed = stbrp_pack_rects(&context, glyph_rects, glyphs_count);

    delete [] glyph_rect_nodes;

    FT_GlyphSlot slot = ft_face->glyph;

    Arena_Temp temp = arena_temp_begin(arena);
    // Pack glyph bitmaps
    int pixel_size = 4;
    u8 *bitmap = push_array(arena, u8, pixel_size * atlas_width * atlas_height);

    for (int i = 0; i < glyphs_count; i++) {
      stbrp_rect rect = glyph_rects[i];
      if (!rect.was_packed) continue;

      int c = rect.id;

      err = FT_Load_Char(ft_face, c, FT_LOAD_DEFAULT);
      if (err) {
        printf("Error loading glyph '%c': %d\n", c, err);
        continue;
      }

      err = FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL);
      if (err) {
        printf("Error rendering glyph '%c': %d\n", c, err);
        continue;
      }

      Glyph *glyph = &font->glyphs[i + 'A'];
      glyph->ax = (float)(ft_face->glyph->advance.x >> 6);
      glyph->ay = (float)(ft_face->glyph->advance.y >> 6);
      glyph->bx = (float)ft_face->glyph->bitmap.width;
      glyph->by = (float)ft_face->glyph->bitmap.rows;
      glyph->bt = (float)ft_face->glyph->bitmap_top;
      glyph->bl = (float)ft_face->glyph->bitmap_left;
      glyph->tx = (float)rect.x / atlas_width;
      glyph->ty = (float)rect.y / atlas_height;

      // Write glyph bitmap to atlas
      for (int y = 0; y < glyph->by; y++) {
        for (int x = 0; x < glyph->bx; x++) {
          int Y = y + rect.y;
          int X = x + rect.x;
          u8 *dest = bitmap + (Y * atlas_width + X) * pixel_size;
          u8 *source = ft_face->glyph->bitmap.buffer + y * ft_face->glyph->bitmap.width + x;
          dest[0] = dest[1] = dest[2] = 255;
          dest[3] = *source;
        }
      }
    }

    font->width = atlas_width;
    font->height = atlas_height;
    font->max_bmp_height = max_bmp_height;
    font->ascend = ascend;
    font->descend = descend;
    font->bbox_height = height;
    font->glyph_width = glyph_width;
    font->glyph_height = glyph_height;
    font->texture = create_texture(bitmap, DXGI_FORMAT_R8G8B8A8_UNORM, atlas_width, atlas_height, 0);

    arena_temp_end(temp);

    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft_lib);
    return font;
}

