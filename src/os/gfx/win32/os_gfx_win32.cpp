global HCURSOR win32_hcursor;
global Arena *win32_event_arena;
global OS_Event_List win32_events;
global bool win32_resizing;

internal OS_Event *win32_push_event(OS_Event_Kind kind) {
    OS_Event *result = push_array(win32_event_arena, OS_Event, 1);
    result->kind = kind;
    result->next = nullptr;
    result->flags = os_event_flags();
    SLLQueuePush(win32_events.first, win32_events.last, result);
    win32_events.count += 1;
    return result;
}

internal LRESULT CALLBACK win32_proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    OS_Event *event = nullptr;
    bool release = false;

    LRESULT result = 0;
    switch (Msg) {
    case WM_MOUSEWHEEL:
    {
        event = win32_push_event(OS_EventKind_Scroll);
        int delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
        event->delta.y = delta;
        break;
    }

    case WM_MOUSEMOVE:
    {
        int x = GET_X_LPARAM(lParam); 
        int y = GET_Y_LPARAM(lParam); 
        event = win32_push_event(OS_EventKind_MouseMove);
        event->pos.x = x;
        event->pos.y = y;
        break;
    }

    case WM_SETCURSOR:
    {
        SetCursor(win32_hcursor);
        break;
    }

    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        release = true;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
        event = win32_push_event(release ? OS_EventKind_MouseUp : OS_EventKind_MouseDown);
        switch (Msg) {
        case WM_LBUTTONUP:
        case WM_LBUTTONDOWN:
            event->key = OS_KEY_LEFTMOUSE;
            break;
        case WM_MBUTTONUP:
        case WM_MBUTTONDOWN:
            event->key = OS_KEY_MIDDLEMOUSE;
            break;
        case WM_RBUTTONUP:
        case WM_RBUTTONDOWN:
            event->key = OS_KEY_RIGHTMOUSE; 
            break;
        }
        int x = GET_X_LPARAM(lParam); 
        int y = GET_Y_LPARAM(lParam); 
        event->pos.x = x;
        event->pos.y = y;
        if (release) {
            ReleaseCapture();
        } else {
            SetCapture(hWnd);
        }
        break;
    }

    case WM_SYSCHAR:
    {
        // result = DefWindowProcA(hWnd, Msg, wParam, lParam);
        break;
    }
    case WM_CHAR:
    {
        u16 vk = wParam & 0x0000ffff;
        u8 c = (u8)vk;
        if (c == '\r') c = '\n';
        if (c >= 32 && c != 127) {
            event = win32_push_event(OS_EventKind_Text);
            event->text = str8_copy(heap_allocator, str8(&c, 1));
        }
        break;
    }

    case WM_SYSKEYUP:
    case WM_KEYUP:
        release = true;
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    {
        u8 down = ((lParam >> 31) == 0);
        u8 alt_mod = (lParam & (1 << 29)) != 0;
        u32 virtual_keycode = (u32)wParam;
        if (virtual_keycode < 256) {
            event = win32_push_event(release ? OS_EventKind_Release : OS_EventKind_Press);
            event->key = os_key_from_vk(virtual_keycode);
        }

        if (wParam == VK_F4 && alt_mod) {
            PostQuitMessage(0);
        }
        break;
    }

    case WM_SIZE:
    {
      R_D3D11_State *d3d = r_d3d11_state();
      UINT width = LOWORD(lParam);
      UINT height = HIWORD(lParam);

      //@Note Resize Swap Chain and Buffers
      if (d3d->swap_chain) {
        // Release all outstanding references to the swap chain's buffers.
        d3d->devcon->OMSetRenderTargets(0, 0, 0);
        if (d3d->render_target) d3d->render_target->Release();

        HRESULT hr;
        // Preserve the existing buffer count and format.
        // Automatically choose the width and height to match the client rect for HWNDs.
        hr = d3d->swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

        // Perform error handling here!

        // Get buffer and create a render-target-view.
        ID3D11Texture2D* pBuffer;
        hr = d3d->swap_chain->GetBuffer(0, __uuidof( ID3D11Texture2D), (void **)&pBuffer);
        // Perform error handling here!

        hr = d3d->device->CreateRenderTargetView(pBuffer, NULL, &d3d->render_target);
        // Perform error handling here!
        pBuffer->Release();

        // depth stencil view
        if (d3d->depth_stencil) ((ID3D11DepthStencilView *)d3d->depth_stencil)->Release();
        {
          D3D11_TEXTURE2D_DESC desc{};
          desc.Width = width;
          desc.Height = height;
          desc.MipLevels = 1;
          desc.ArraySize = 1;
          desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
          desc.SampleDesc.Count = 1;
          desc.SampleDesc.Quality = 0;
          desc.Usage = D3D11_USAGE_DEFAULT;
          desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
          desc.CPUAccessFlags = 0;
          desc.MiscFlags = 0;

          ID3D11Texture2D *depth_stencil_buffer = nullptr;
          hr = d3d->device->CreateTexture2D(&desc, NULL, &depth_stencil_buffer);
          hr = d3d->device->CreateDepthStencilView(depth_stencil_buffer, NULL, &d3d->depth_stencil);
        }

        d3d->devcon->OMSetRenderTargets(1, &d3d->render_target, d3d->depth_stencil);

        // Set up the viewport.
        D3D11_VIEWPORT vp;
        vp.Width  = (float)width;
        vp.Height = (float)height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        d3d->devcon->RSSetViewports( 1, &vp);
      }
      result = 1;
      break;
    }

    case WM_DROPFILES:
    {
        HDROP hDrop = (HDROP)wParam;
        UINT len = DragQueryFileA(hDrop, 0, NULL, 0);
        String8 string = str8_zero();
        string.data = push_array(win32_event_arena, u8, len + 1);
        string.count = len;
        DragQueryFileA(hDrop, 0, (LPSTR)string.data, len + 1);
        event = win32_push_event(OS_EventKind_DropFile);
        event->text = string;
        DragFinish(hDrop);
        break;
    }

    case WM_ERASEBKGND:
    {
      HDC hdc = (HDC)wParam;
      RECT rect;
      GetClientRect(hWnd, &rect);
      HBRUSH brush = CreateSolidBrush(RGB(39, 57, 61));
      FillRect(hdc, &rect, brush);
      DeleteObject(brush);
      result = 1;
      break;
    }
   
    // case WM_SIZING:
    // {
    //     win32_resizing = true;
    //     break;
    // }
    // case WM_ENTERSIZEMOVE:
    // {
    //     win32_resizing = true;
    //     break;
    // }
    // case WM_EXITSIZEMOVE:
    // {
    //     win32_resizing = false;
    //     break;
    // }
    
    case WM_CREATE:
    {
        break;
    }
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    default:
        result = DefWindowProcA(hWnd, Msg, wParam, lParam);
    }
    return result;
}

internal String8 os_get_clipboard_text(Arena *arena) {
    String8 result = str8_zero();
    if (IsClipboardFormatAvailable(CF_TEXT) && OpenClipboard(0)) {
        HANDLE handle = GetClipboardData(CF_TEXT);
        if (handle) {
            u8 *buffer = (u8 *)GlobalLock(handle);
            if (buffer) {
                result = str8_copy(arena, str8_cstring((char *)buffer));
                GlobalUnlock(handle);
            }
        }
        CloseClipboard();
    }
    return result;
}

internal void os_set_clipboard_text(String8 text) {
    if (OpenClipboard(0)) {
        HANDLE handle = GlobalAlloc(GMEM_MOVEABLE, text.count + 1);
        if (handle) {
            u8 *buffer = (u8 *)GlobalLock(handle);
            MemoryCopy(buffer, text.data, text.count);
            buffer[text.count] = 0;
            GlobalUnlock(handle);
            SetClipboardData(CF_TEXT, handle);
        }
    }
}

internal void os_set_cursor(OS_Cursor cursor) {
    local_persist HCURSOR hcursor;
    switch (cursor) {
    default:
    case OS_Cursor_Hidden:
        hcursor = NULL;
        break;
    case OS_Cursor_Arrow:
        hcursor = LoadCursorA(NULL, IDC_ARROW);
        break;
    case OS_Cursor_Ibeam:
        hcursor = LoadCursorA(NULL, IDC_IBEAM);
        break;
    case OS_Cursor_Hand:
        hcursor = LoadCursorA(NULL, IDC_HAND);
        break;
    case OS_Cursor_SizeNS:
        hcursor = LoadCursorA(NULL, IDC_SIZENS);
        break;
    case OS_Cursor_SizeWE:
        hcursor = LoadCursorA(NULL, IDC_SIZEWE);
        break;
    }
    
    if (win32_hcursor != hcursor) {
        PostMessageA(0, WM_SETCURSOR, 0, 0);
        win32_hcursor = hcursor;
    }
}

internal Rect os_client_rect_from_window(OS_Handle window_handle) {
    RECT client_rect;
    GetClientRect((HWND)window_handle, &client_rect);
    Rect result;
    result.x0 = (f32)client_rect.left;
    result.x1 = (f32)client_rect.right;
    result.y0 = (f32)client_rect.top;
    result.y1 = (f32)client_rect.bottom;
    return result;
}

internal Vector2 os_get_window_dim(OS_Handle window_handle) {
    Vector2 result{};
    RECT rect;
    int width = 0, height = 0;
    if (GetClientRect((HWND)window_handle, &rect)) {
        result.x = (f32)(rect.right - rect.left);
        result.y = (f32)(rect.bottom - rect.top);
    }
    return result;
}

internal int os_get_window_width(OS_Handle window_handle) {
  RECT rect;
  if (GetClientRect((HWND)window_handle, &rect)) {
    return rect.right - rect.left;
  }
  return 0;
}

internal int os_get_window_height(OS_Handle window_handle) {
  RECT rect;
  if (GetClientRect((HWND)window_handle, &rect)) {
    return rect.bottom - rect.top;
  }
  return 0;
}

internal Vector2 os_mouse_from_window(OS_Handle window_handle) {
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient((HWND)window_handle, &pt);
    Vector2 result;
    result.x = (f32)pt.x;
    result.y = (f32)pt.y;
    return result;
}

internal bool os_window_is_focused(OS_Handle window_handle) {
    HWND active_hwnd = GetActiveWindow();
    bool result = (OS_Handle)active_hwnd == window_handle;
    return result;
}
