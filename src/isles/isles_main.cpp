
#include <string>
#include <unordered_map>
#include <vector>

#pragma warning(push)
#pragma warning( disable : 4244)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning(pop)

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <ft2build.h>
#include <freetype/freetype.h>

#include <fmod_studio.hpp>
#include <fmod_errors.h>
#include <fmod.hpp>

#include "base/base_core.h"
#include "core/core_math.h"
#include "base/base_arena.h"
#include "base/base_strings.h"
#include "auto_array.h"
#include "os/os.h"
#include "path/path.h"
#include "render/render.h"
#include "font/font.h"

#undef near
#undef far

#include "core.h"
#include "mesh.h"
#include "audio.h"
#include "entity.h"
#include "ui/ui_core.h"
#include "editor.h"
#include "isles.h"
#include "draw.h"
#include "input.h"
#include "undo.h"
#include "render_target.h"


#define STB_SPRINTF_IMPLEMENTATION
// #define STB_SPRINTF_DECORATE(name) base_##name
#include <stb_sprintf.h>

#include "base/base_core.cpp"
#include "core/core_math.cpp"
#include "base/base_arena.cpp"
#include "base/base_strings.cpp"

#include "os/os.cpp"
#include "path/path.cpp"
#include "font/font.cpp"

#include "log.cpp"

#include "core.cpp"
#include "audio.cpp"
#include "draw.cpp"
#include "render/render.cpp"
#include "ui/ui_core.cpp"
#include "ui/ui_widgets.cpp"
#include "mesh.cpp"
#include "input.cpp"
#include "editor.cpp"
#include "undo.cpp"
#include "render_target.cpp"
#include "isles.cpp"

global bool window_should_close;

int main() {
  QueryPerformanceFrequency((LARGE_INTEGER *)&win32_performance_frequency);
  timeBeginPeriod(1);

  win32_event_arena = make_arena(get_malloc_allocator());
  permanent_arena = arena_alloc(get_virtual_allocator(), MB(2));
  temporary_arena = arena_alloc(get_virtual_allocator(), MB(2));

  char *class_name = "ISLES_WINDOW_CLASS";
  HINSTANCE hinstance = GetModuleHandle(NULL);
  WNDCLASSA window_class{};
  window_class.style = CS_HREDRAW | CS_VREDRAW;
  window_class.lpfnWndProc = win32_proc;
  window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  window_class.lpszClassName = class_name;
  window_class.hInstance = hinstance;
  window_class.hCursor = LoadCursorA(NULL, IDC_ARROW);
  if (!RegisterClassA(&window_class)) {
    printf("RegisterClassA failed, err:%d\n", GetLastError());
  }

  HWND hWnd;
  {
    RECT client_rect = {0, 0, 1600, 900};
    AdjustWindowRect(&client_rect, WS_OVERLAPPEDWINDOW, FALSE);
    hWnd = CreateWindowA(class_name, "Isles", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, client_rect.right - client_rect.left, client_rect.bottom - client_rect.top, NULL, NULL, hinstance, NULL);
  }

  OS_Handle window_handle = (OS_Handle)hWnd;
  r_d3d11_initialize(hWnd);

  V2_F32 old_window_dim = {};

  int target_frames_per_second = 75;
  int target_ms_per_frame = (int)(1000.f / (f32)target_frames_per_second);
  f32 dt = 0.0f;
  s64 start_clock, last_clock;
  start_clock = last_clock = get_wall_clock(); 

  while (!window_should_close) {
    MSG message;
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
      if (message.message == WM_QUIT) {
        window_should_close = true;
      }
      TranslateMessage(&message);
      DispatchMessageA(&message);
    }

    update_and_render(&win32_events, window_handle, dt);

    win32_events.first = nullptr;
    win32_events.last = nullptr;
    win32_events.count = 0;
    arena_clear(win32_event_arena);

    s64 work_clock = get_wall_clock();
    f32 work_ms_elapsed = get_ms_elapsed(last_clock, work_clock);
    if ((int)work_ms_elapsed < target_ms_per_frame) {
      DWORD sleep_ms = target_ms_per_frame - (int)work_ms_elapsed;
      Sleep(sleep_ms);
    }

    s64 end_clock = get_wall_clock();
    dt = get_ms_elapsed(last_clock, end_clock);

    last_clock = end_clock;
  }

  return 0;
}
