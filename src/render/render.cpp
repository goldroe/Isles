global R_D3D11_State *r_g_d3d11_state;

internal inline R_D3D11_State *r_d3d11_state() {
  return r_g_d3d11_state;
}

internal void r_clear_color(f32 r, f32 g, f32 b, f32 a) {
  R_D3D11_State *d3d11_state = r_d3d11_state();
  FLOAT color[4] = {r, g, b, a};
  d3d11_state->device_context->ClearRenderTargetView((ID3D11RenderTargetView*)d3d11_state->default_render_target->render_target_view, color);
}

UINT get_num_mip_levels(UINT width, UINT height) {
  UINT levels = 1;
  while(width > 1 || height > 1) {
    width = Max(width / 2, 1);
    height = Max(height / 2, 1);
    ++levels;
  }
  return levels;
}

internal ID3D11Buffer *make_uniform_buffer(u32 bytes) {
  ID3D11Buffer *buffer;
  D3D11_BUFFER_DESC desc = {};
  desc.Usage          = D3D11_USAGE_DYNAMIC;
  desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  desc.ByteWidth      = bytes;
  r_d3d11_state()->device->CreateBuffer(&desc, nullptr, &buffer);
  return buffer;
}

internal ID3D11Buffer *make_index_buffer(void *data, size_t bytes) {
  ID3D11Buffer *buffer = nullptr;
  D3D11_BUFFER_DESC desc = {};
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.ByteWidth = (UINT)bytes;
  desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;
  D3D11_SUBRESOURCE_DATA res = {};
  res.pSysMem = data;
  res.SysMemPitch = 0;
  res.SysMemSlicePitch = 0;
  r_g_d3d11_state->device->CreateBuffer(&desc, &res, &buffer);
  return buffer;
}

internal ID3D11Buffer *make_vertex_buffer(void *data, size_t elem_count, size_t elem_size) {
  UINT bytes = (UINT)(elem_count * elem_size);

  ID3D11Buffer *vertex_buffer = nullptr;
  D3D11_BUFFER_DESC desc = {};
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.ByteWidth = bytes;
  desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;
  D3D11_SUBRESOURCE_DATA res = {};
  res.pSysMem = data;
  res.SysMemPitch = 0;
  res.SysMemSlicePitch = 0;
  r_g_d3d11_state->device->CreateBuffer(&desc, &res, &vertex_buffer);
  return vertex_buffer;
}

internal void write_uniform_buffer(ID3D11Buffer *buffer, void *data, UINT bytes) {
  R_D3D11_State *d3d = r_d3d11_state();
  D3D11_MAPPED_SUBRESOURCE res = {};
  if (d3d->device_context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res) == S_OK) {
    memcpy((u8 *)res.pData, data, bytes);
    d3d->device_context->Unmap(buffer, 0);
  } else {
    logprint("Failed to write to constant buffer.\n");
  }
}

internal int bytes_from_format(DXGI_FORMAT fmt) {
  switch (fmt) {
  case DXGI_FORMAT_R32G32B32A32_TYPELESS:
  case DXGI_FORMAT_R32G32B32A32_FLOAT:
  case DXGI_FORMAT_R32G32B32A32_UINT:
  case DXGI_FORMAT_R32G32B32A32_SINT:
    return 16;

  case DXGI_FORMAT_R32G32B32_TYPELESS:
  case DXGI_FORMAT_R32G32B32_FLOAT:
  case DXGI_FORMAT_R32G32B32_UINT:
  case DXGI_FORMAT_R32G32B32_SINT:
    return 12;

  case DXGI_FORMAT_R16G16B16A16_TYPELESS:
  case DXGI_FORMAT_R16G16B16A16_FLOAT:
  case DXGI_FORMAT_R16G16B16A16_UNORM:
  case DXGI_FORMAT_R16G16B16A16_UINT:
  case DXGI_FORMAT_R16G16B16A16_SNORM:
  case DXGI_FORMAT_R16G16B16A16_SINT:
  case DXGI_FORMAT_R32G32_TYPELESS:
  case DXGI_FORMAT_R32G32_FLOAT:
  case DXGI_FORMAT_R32G32_UINT:
  case DXGI_FORMAT_R32G32_SINT:
  case DXGI_FORMAT_R32G8X24_TYPELESS:
  case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
  case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
  case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
  case DXGI_FORMAT_Y416:
  case DXGI_FORMAT_Y210:
  case DXGI_FORMAT_Y216:
    return 8;

  case DXGI_FORMAT_R10G10B10A2_TYPELESS:
  case DXGI_FORMAT_R10G10B10A2_UNORM:
  case DXGI_FORMAT_R10G10B10A2_UINT:
  case DXGI_FORMAT_R11G11B10_FLOAT:
  case DXGI_FORMAT_R8G8B8A8_TYPELESS:
  case DXGI_FORMAT_R8G8B8A8_UNORM:
  case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
  case DXGI_FORMAT_R8G8B8A8_UINT:
  case DXGI_FORMAT_R8G8B8A8_SNORM:
  case DXGI_FORMAT_R8G8B8A8_SINT:
  case DXGI_FORMAT_R16G16_TYPELESS:
  case DXGI_FORMAT_R16G16_FLOAT:
  case DXGI_FORMAT_R16G16_UNORM:
  case DXGI_FORMAT_R16G16_UINT:
  case DXGI_FORMAT_R16G16_SNORM:
  case DXGI_FORMAT_R16G16_SINT:
  case DXGI_FORMAT_R32_TYPELESS:
  case DXGI_FORMAT_D32_FLOAT:
  case DXGI_FORMAT_R32_FLOAT:
  case DXGI_FORMAT_R32_UINT:
  case DXGI_FORMAT_R32_SINT:
  case DXGI_FORMAT_R24G8_TYPELESS:
  case DXGI_FORMAT_D24_UNORM_S8_UINT:
  case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
  case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
  case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
  case DXGI_FORMAT_R8G8_B8G8_UNORM:
  case DXGI_FORMAT_G8R8_G8B8_UNORM:
  case DXGI_FORMAT_B8G8R8A8_UNORM:
  case DXGI_FORMAT_B8G8R8X8_UNORM:
  case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
  case DXGI_FORMAT_B8G8R8A8_TYPELESS:
  case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
  case DXGI_FORMAT_B8G8R8X8_TYPELESS:
  case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
  case DXGI_FORMAT_AYUV:
  case DXGI_FORMAT_Y410:
  case DXGI_FORMAT_YUY2:
  // case XBOX_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
  // case XBOX_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT:
  // case XBOX_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM:
    return 4;

  case DXGI_FORMAT_P010:
  case DXGI_FORMAT_P016:
  // case XBOX_DXGI_FORMAT_D16_UNORM_S8_UINT:
  // case XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
  // case XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT:
  // case WIN10_DXGI_FORMAT_V408:
    return 3;

  case DXGI_FORMAT_R8G8_TYPELESS:
  case DXGI_FORMAT_R8G8_UNORM:
  case DXGI_FORMAT_R8G8_UINT:
  case DXGI_FORMAT_R8G8_SNORM:
  case DXGI_FORMAT_R8G8_SINT:
  case DXGI_FORMAT_R16_TYPELESS:
  case DXGI_FORMAT_R16_FLOAT:
  case DXGI_FORMAT_D16_UNORM:
  case DXGI_FORMAT_R16_UNORM:
  case DXGI_FORMAT_R16_UINT:
  case DXGI_FORMAT_R16_SNORM:
  case DXGI_FORMAT_R16_SINT:
  case DXGI_FORMAT_B5G6R5_UNORM:
  case DXGI_FORMAT_B5G5R5A1_UNORM:
  case DXGI_FORMAT_A8P8:
  case DXGI_FORMAT_B4G4R4A4_UNORM:
  // case WIN10_DXGI_FORMAT_P208:
  // case WIN10_DXGI_FORMAT_V208:
    return 2;

  // case DXGI_FORMAT_NV12:
  // case DXGI_FORMAT_420_OPAQUE:
  // case DXGI_FORMAT_NV11:
    // return 12;//bits

  case DXGI_FORMAT_R8_TYPELESS:
  case DXGI_FORMAT_R8_UNORM:
  case DXGI_FORMAT_R8_UINT:
  case DXGI_FORMAT_R8_SNORM:
  case DXGI_FORMAT_R8_SINT:
  case DXGI_FORMAT_A8_UNORM:
  case DXGI_FORMAT_AI44:
  case DXGI_FORMAT_IA44:
  case DXGI_FORMAT_P8:
  // case XBOX_DXGI_FORMAT_R4G4_UNORM:
    return 1;

  case DXGI_FORMAT_R1_UNORM:
    return 1;

  // case DXGI_FORMAT_BC1_TYPELESS:
  // case DXGI_FORMAT_BC1_UNORM:
  // case DXGI_FORMAT_BC1_UNORM_SRGB:
  // case DXGI_FORMAT_BC4_TYPELESS:
  // case DXGI_FORMAT_BC4_UNORM:
  // case DXGI_FORMAT_BC4_SNORM:
  //   return 4; // bits

  case DXGI_FORMAT_BC2_TYPELESS:
  case DXGI_FORMAT_BC2_UNORM:
  case DXGI_FORMAT_BC2_UNORM_SRGB:
  case DXGI_FORMAT_BC3_TYPELESS:
  case DXGI_FORMAT_BC3_UNORM:
  case DXGI_FORMAT_BC3_UNORM_SRGB:
  case DXGI_FORMAT_BC5_TYPELESS:
  case DXGI_FORMAT_BC5_UNORM:
  case DXGI_FORMAT_BC5_SNORM:
  case DXGI_FORMAT_BC6H_TYPELESS:
  case DXGI_FORMAT_BC6H_UF16:
  case DXGI_FORMAT_BC6H_SF16:
  case DXGI_FORMAT_BC7_TYPELESS:
  case DXGI_FORMAT_BC7_UNORM:
  case DXGI_FORMAT_BC7_UNORM_SRGB:
    return 2;

  default:
    return 0;
  }
}

internal Texture *r_create_texture(u8 *data, DXGI_FORMAT format, int w, int h, int flags) {
  HRESULT hr = S_OK;

  UINT mip_levels = 1;
  if (flags & TEXTURE_FLAG_GENERATE_MIPS) mip_levels = get_num_mip_levels(w, h);

  R_D3D11_State *d3d11_state = r_d3d11_state();

  Texture *texture = (Texture *)malloc(sizeof(Texture));
  texture->width = w;
  texture->height = h;
  texture->format  = format;

  ID3D11Texture2D *tex2d = nullptr;
  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = (UINT)w;
  desc.Height = (UINT)h;
  desc.MipLevels = mip_levels;
  desc.ArraySize = 1;
  desc.Format = format;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.MiscFlags = 0;
  if (flags & TEXTURE_FLAG_GENERATE_MIPS) {
    desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
    desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
  }

  hr = d3d11_state->device->CreateTexture2D(&desc, NULL, &tex2d);
  d3d11_state->device_context->UpdateSubresource(tex2d, 0, NULL, data, w * bytes_from_format(format), 0);
    
  ID3D11ShaderResourceView *view = nullptr;
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Format = desc.Format;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = desc.MipLevels;
	srv_desc.Texture2D.MostDetailedMip = 0;
  hr = d3d11_state->device->CreateShaderResourceView(tex2d, &srv_desc, &view);

  if (flags & TEXTURE_FLAG_GENERATE_MIPS) {
    d3d11_state->device_context->GenerateMips(view); 
  }

  texture->view = (void *)view;
  return texture;
}

internal Texture *r_create_texture_from_file(String8 file_name, int flags) {
  int x, y, n;
  u8 *data = stbi_load((char *)file_name.data, &x, &y, &n, 4);
  if (!data) {
    logprint("Cannot load '%S'\n", file_name);
    return nullptr;
  }
  Texture *texture = r_create_texture(data, DXGI_FORMAT_R8G8B8A8_UNORM, x, y, flags);
  return texture;
}

internal void r_resize_render_target(Vector2Int dimension) {
  R_D3D11_State *d3d11_state = r_d3d11_state();

  if (dimension.x == 0 || dimension.y == 0) return;

  if (dimension == d3d11_state->window_dimension) {
    return;
  }

  d3d11_state->window_dimension = dimension;

  d3d11_state->draw_region = {
    0, 0, (f32)dimension.x, (f32)dimension.y
  };

  UINT width = (UINT)dimension.x;
  UINT height = (UINT)dimension.y;

  HRESULT hr = S_OK;

  // NOTE: Resize render target view
  d3d11_state->device_context->OMSetRenderTargets(0, 0, 0);

  Render_Target *render_target = d3d11_state->default_render_target;

  // Release all outstanding references to the swap chain's buffers.
  if (render_target->render_target_view) ((ID3D11RenderTargetView*)render_target->render_target_view)->Release();

  // Preserve the existing buffer count and format.
  // Automatically choose the width and height to match the client rect for HWNDs.
  hr = d3d11_state->swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

  // Get buffer and create a render-target-view.
  ID3D11Texture2D *backbuffer;
  hr = d3d11_state->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&backbuffer);

  hr = d3d11_state->device->CreateRenderTargetView(backbuffer, NULL, (ID3D11RenderTargetView **)&render_target->render_target_view);

  render_target->texture = backbuffer;

  if (render_target->depth_stencil_view) ((ID3D11DepthStencilView *)render_target->depth_stencil_view)->Release();

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
    hr = d3d11_state->device->CreateTexture2D(&desc, NULL, &depth_stencil_buffer);
    hr = d3d11_state->device->CreateDepthStencilView(depth_stencil_buffer, NULL, (ID3D11DepthStencilView **)&render_target->depth_stencil_view);
  }

  d3d11_state->device_context->OMSetRenderTargets(1, (ID3D11RenderTargetView **)&render_target->render_target_view, (ID3D11DepthStencilView*)render_target->depth_stencil_view);
}

internal void r_d3d11_initialize(HWND window_handle) {
  r_g_d3d11_state = new R_D3D11_State();

  R_D3D11_State *d3d11_state = r_d3d11_state();

  d3d11_state->arena = arena_alloc(get_virtual_allocator(), MB(4));

  HRESULT hr;

  UINT device_flags = 0;
  device_flags |= D3D11_CREATE_DEVICE_DEBUG;

  D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };

  //@Note Initialize Swap Chain and Device
  {
    DXGI_MODE_DESC buffer_desc{};
    buffer_desc.Width = 0;
    buffer_desc.Height = 0;
    buffer_desc.RefreshRate.Numerator = 0;
    buffer_desc.RefreshRate.Denominator = 1;
    buffer_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    buffer_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    buffer_desc.Scaling = DXGI_MODE_SCALING_STRETCHED;
    DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
    swap_chain_desc.BufferDesc = buffer_desc;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.SampleDesc.Quality = 0;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount = 2;
    swap_chain_desc.OutputWindow = window_handle;
    swap_chain_desc.Windowed = TRUE;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, device_flags, feature_levels, ArrayCount(feature_levels), D3D11_SDK_VERSION, &swap_chain_desc, &d3d11_state->swap_chain, &d3d11_state->device, NULL, &d3d11_state->device_context);
  }

  //@Note Initialize render target view
  {
    d3d11_state->default_render_target = new Render_Target();
    ID3D11Texture2D *back_buffer;
    hr = d3d11_state->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&back_buffer);
    hr = d3d11_state->device->CreateRenderTargetView(back_buffer, NULL, (ID3D11RenderTargetView **)&d3d11_state->default_render_target->render_target_view);
    back_buffer->Release();
  }

  //@Note Create depth stencil states
  {
    D3D11_DEPTH_STENCIL_DESC desc = {};
    desc.DepthEnable = true;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    desc.StencilEnable = false;
    desc.FrontFace.StencilFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    desc.BackFace = desc.FrontFace;
    d3d11_state->device->CreateDepthStencilState(&desc, &d3d11_state->depth_stencil_states[DEPTH_STATE_DEFAULT]);

    desc = {};
    desc.DepthEnable = false;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    desc.StencilEnable = false;
    desc.FrontFace.StencilFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    desc.BackFace = desc.FrontFace;
    d3d11_state->device->CreateDepthStencilState(&desc, &d3d11_state->depth_stencil_states[DEPTH_STATE_DISABLE]);
  }

  //@Note Create rasterizer states
  {
    D3D11_RASTERIZER_DESC desc = {};
    desc.FillMode = D3D11_FILL_SOLID;
    desc.CullMode = D3D11_CULL_BACK;
    desc.FrontCounterClockwise = true;
    desc.DepthBias = 0;
    desc.DepthClipEnable = false;
    desc.ScissorEnable = false;
    desc.MultisampleEnable = true;
    desc.AntialiasedLineEnable = false;
    d3d11_state->device->CreateRasterizerState(&desc, &d3d11_state->rasterizer_states[RASTERIZER_STATE_DEFAULT]);
    rasterizer_cull_back = d3d11_state->rasterizer_states[RASTERIZER_STATE_DEFAULT];

    desc = {};
    desc.FillMode = D3D11_FILL_SOLID;
    desc.CullMode = D3D11_CULL_NONE;
    desc.FrontCounterClockwise = true;
    desc.DepthBias = 0;
    desc.DepthClipEnable = false;
    desc.ScissorEnable = false;
    desc.MultisampleEnable = false;
    desc.AntialiasedLineEnable = false;
    d3d11_state->device->CreateRasterizerState(&desc, &d3d11_state->rasterizer_states[RASTERIZER_STATE_NO_CULL]);

    desc = {};
    desc.FillMode = D3D11_FILL_SOLID;
    desc.CullMode = D3D11_CULL_FRONT;
    desc.FrontCounterClockwise = true;
    desc.DepthBias = 0;
    desc.DepthClipEnable = false;
    desc.ScissorEnable = false;
    desc.MultisampleEnable = false;
    desc.AntialiasedLineEnable = false;
    d3d11_state->device->CreateRasterizerState(&desc, &rasterizer_cull_front);

    desc = {};
    desc.FillMode = D3D11_FILL_SOLID;
    desc.CullMode = D3D11_CULL_NONE;
    desc.FrontCounterClockwise = true;
    desc.DepthBias = 0;
    desc.DepthClipEnable = false;
    desc.ScissorEnable = false;
    desc.MultisampleEnable = false;
    desc.AntialiasedLineEnable = false;
    d3d11_state->device->CreateRasterizerState(&desc, &d3d11_state->rasterizer_states[RASTERIZER_STATE_NO_CULL]);

    desc = {};
    desc.FillMode = D3D11_FILL_SOLID;
    desc.CullMode = D3D11_CULL_BACK;
    desc.ScissorEnable = false;
    desc.DepthClipEnable = false;
    desc.FrontCounterClockwise = true;
    desc.MultisampleEnable = true;
    d3d11_state->device->CreateRasterizerState(&desc, &d3d11_state->rasterizer_states[RASTERIZER_STATE_TEXT]);

    desc = {};
    desc.FillMode = D3D11_FILL_WIREFRAME;
    desc.CullMode = D3D11_CULL_NONE;
    desc.ScissorEnable = false;
    desc.DepthClipEnable = false;
    desc.DepthBias = -100000000;
    desc.FrontCounterClockwise = true;
    desc.MultisampleEnable = true;
    desc.AntialiasedLineEnable = false;
    d3d11_state->device->CreateRasterizerState(&desc, &rasterizer_wireframe);
  }

  {
    D3D11_BLEND_DESC desc = {};
    desc.AlphaToCoverageEnable = false;
    desc.IndependentBlendEnable = false;
    desc.RenderTarget[0].BlendEnable = false;
    desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    d3d11_state->device->CreateBlendState(&desc, &d3d11_state->blend_states[BLEND_STATE_DEFAULT]);

    desc = {};
    desc.AlphaToCoverageEnable = false;
    desc.IndependentBlendEnable = false;
    desc.RenderTarget[0].BlendEnable = true;
    desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    // desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    d3d11_state->device->CreateBlendState(&desc, &d3d11_state->blend_states[BLEND_STATE_ALPHA]);
  }

  //@Note Create Sampler states
  {
    D3D11_SAMPLER_DESC desc = {};
    desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.MinLOD = 0;
    desc.MaxLOD = D3D11_FLOAT32_MAX;
    desc.MipLODBias = 0;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    desc.Filter = D3D11_FILTER_ANISOTROPIC;
    desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.MaxAnisotropy = 4;
    desc.MinLOD = 0;
    desc.MaxLOD = D3D11_FLOAT32_MAX;
    desc.MipLODBias = 0;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    d3d11_state->device->CreateSamplerState(&desc, &d3d11_state->sampler_states[SAMPLER_STATE_LINEAR]);

    desc = {};
    desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.MipLODBias = 0;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    d3d11_state->device->CreateSamplerState(&desc, &d3d11_state->sampler_states[SAMPLER_STATE_POINT]);
  }

  {
    u32 white_bitmap[] = {
      0xFFFFFFFF, 0xFFFFFFFF,
      0xFFFFFFFF, 0xFFFFFFFF
    };
    d3d11_state->fallback_tex = r_create_texture((u8 *)white_bitmap, DXGI_FORMAT_R8G8B8A8_UNORM, 2, 2, 0);
  }

  //@Note Compile shaders
  D3D11_INPUT_ELEMENT_DESC basic_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_basic = r_d3d11_make_shader(str8_lit("data/shaders/basic_3d.hlsl"), str8_lit("Basic"), basic_ilay, ArrayCount(basic_ilay));

  D3D11_INPUT_ELEMENT_DESC mesh_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Vertex_XNCUU, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(Vertex_XNCUU, uv),       D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_mesh = r_d3d11_make_shader(str8_lit("data/shaders/mesh.hlsl"), str8_lit("Mesh"), mesh_ilay, ArrayCount(mesh_ilay));

  D3D11_INPUT_ELEMENT_DESC entity_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Vertex_XNCUU, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Vertex_XNCUU, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex_XNCUU, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(Vertex_XNCUU, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_entity = r_d3d11_make_shader(str8_lit("data/shaders/entity.hlsl"), str8_lit("Entity"), entity_ilay, ArrayCount(entity_ilay));

  D3D11_INPUT_ELEMENT_DESC skinned_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Vertex_Skinned, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Vertex_Skinned, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(Vertex_Skinned, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "BONEIDS",  0, DXGI_FORMAT_R32G32B32A32_SINT,  0, offsetof(Vertex_Skinned, bone_ids), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "WEIGHTS",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex_Skinned, bone_weights), D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_skinned = r_d3d11_make_shader(str8_lit("data/shaders/skinned.hlsl"), str8_lit("Skinned"), skinned_ilay, ArrayCount(skinned_ilay));

  shader_skinned_shadow_map = r_d3d11_make_shader(str8_lit("data/shaders/skinned_shadow_map.hlsl"), str8_lit("Skinned ShadowMap"), skinned_ilay, ArrayCount(skinned_ilay));

  D3D11_INPUT_ELEMENT_DESC argb_texture_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "ARGB",     0, DXGI_FORMAT_R32_UINT,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_argb_texture = r_d3d11_make_shader(str8_lit("data/shaders/argb_texture.hlsl"), str8_lit("ARGB Texture"), argb_texture_ilay, ArrayCount(argb_texture_ilay));

  D3D11_INPUT_ELEMENT_DESC rect_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "STYLE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_rect = r_d3d11_make_shader(str8_lit("data/shaders/rect.hlsl"), str8_lit("Rect"), rect_ilay, ArrayCount(rect_ilay));

  D3D11_INPUT_ELEMENT_DESC picker_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_picker = r_d3d11_make_shader(str8_lit("data/shaders/picker.hlsl"), str8_lit("Picker"), picker_ilay, ArrayCount(picker_ilay));

  D3D11_INPUT_ELEMENT_DESC shadowmap_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex_XNCUU, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_shadow_map = r_d3d11_make_shader(str8_lit("data/shaders/shadow_map.hlsl"), str8_lit("ShadowMap"), shadowmap_ilay, ArrayCount(shadowmap_ilay));

  sun_icon_texture = r_create_texture_from_file(str8_lit("data/textures/sun_icon.bmp"), 0);
  eye_of_horus_texture = r_create_texture_from_file(str8_lit("data/textures/eye_of_horus.png"), 0);

  water_plane_mesh = generate_plane_mesh(Vector2(100.0f, 100.0f));
}

internal void r_d3d11_begin(OS_Handle window_handle) {
  R_D3D11_State *d3d11_state = r_d3d11_state();
  d3d11_state->device_context->OMSetRenderTargets(1, (ID3D11RenderTargetView **)&d3d11_state->default_render_target->render_target_view, (ID3D11DepthStencilView *)d3d11_state->default_render_target->depth_stencil_view);

  HRESULT hr = S_OK;
  D3D11_VIEWPORT viewport{};
  viewport.TopLeftX = d3d11_state->draw_region.x0;
  viewport.TopLeftY = d3d11_state->draw_region.y0;
  viewport.Width = rect_width(d3d11_state->draw_region);
  viewport.Height = rect_height(d3d11_state->draw_region);
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;
  d3d11_state->device_context->RSSetViewports(1, &viewport);

  d3d11_state->device_context->RSSetState(d3d11_state->rasterizer_states[RASTERIZER_STATE_DEFAULT]);

  d3d11_state->device_context->ClearDepthStencilView((ID3D11DepthStencilView*)d3d11_state->default_render_target->depth_stencil_view, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

  d3d11_state->device_context->OMSetBlendState(nullptr, NULL, 0xffffffff);

  d3d11_state->device_context->VSSetShader(nullptr, nullptr, 0);
  d3d11_state->device_context->PSSetShader(nullptr, nullptr, 0);
  d3d11_state->device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  Matrix4 screen_projection = ortho_rh_zo(0, rect_width(d3d11_state->draw_region), 0.0f, rect_height(d3d11_state->draw_region));

  set_shader(shader_argb_texture);
  set_constant(str8_lit("projection"), screen_projection);
  apply_constants();

  set_shader(nullptr);
}

internal void r_d3d11_recompile_shader(Shader *shader) {
  R_D3D11_State *d3d11_state = r_d3d11_state();
  ID3DBlob *vs_blob, *ps_blob;
  ID3DBlob *vs_error, *ps_error;

  String8 contents = {};
  OS_Handle file_handle = os_open_file(shader->file_name, OS_AccessFlag_Read);
  if (os_valid_handle(file_handle)) {
    shader->last_write_time = os_file_last_write_time(file_handle);
    contents = os_read_file_string(file_handle);
    os_close_handle(file_handle);
  }

  HRESULT hr;
  int compilation_success = true;

  UINT compile_flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
  compile_flags |= D3DCOMPILE_DEBUG;
#endif

  hr = D3DCompile(contents.data, contents.count, (LPCSTR)shader->name.data, NULL, NULL, "vs_main", "vs_5_0", compile_flags, 0, &vs_blob, &vs_error);
  if (hr != S_OK) {
    compilation_success = false;
    logprint("Error compiling vertex shader %S\n", shader->name);
    logprint("%s\n", vs_error->GetBufferPointer());
  }
    
  hr = D3DCompile(contents.data, contents.count, (LPCSTR)shader->name.data, NULL, NULL, "ps_main", "ps_5_0", compile_flags, 0, &ps_blob, &ps_error);
  if (hr != S_OK) {
    compilation_success = false;
    logprint("Error compiling pixel shader %S\n", shader->name);
    logprint("%s\n", ps_error->GetBufferPointer());
  }

  ID3D11VertexShader *vertex_shader = nullptr;
  ID3D11PixelShader *pixel_shader = nullptr;

  if (compilation_success) {
    hr = d3d11_state->device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), NULL, &vertex_shader);
    hr = d3d11_state->device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), NULL, &pixel_shader);
  }

  if (vs_blob)  vs_blob->Release();
  if (ps_blob)  ps_blob->Release();
  if (vs_error) vs_error->Release();
  if (ps_error) ps_error->Release();

  shader->vertex_shader = vertex_shader;
  shader->pixel_shader = pixel_shader;
}

internal char *copy_str(const char *src) {
  size_t len = strlen(src);
  char *dst = (char *)malloc(len + 1);
  strcpy(dst, src);
  return dst;
}

internal void fill_shader_bindings(Shader_Bindings *bindings, void *byte_code, size_t byte_code_size, bool is_vertex_shader) {
  ID3D11ShaderReflection* reflection = nullptr;

  if (D3DReflect(byte_code, byte_code_size, IID_ID3D11ShaderReflection, (void**)&reflection) == S_OK) {
    D3D11_SHADER_DESC shader_desc;
    reflection->GetDesc(&shader_desc);

    for (UINT i = 0; i < shader_desc.BoundResources; i++) {
      D3D11_SHADER_INPUT_BIND_DESC desc;
      reflection->GetResourceBindingDesc(i, &desc);
      char *str = copy_str(desc.Name);
      String8 name = str8_cstring(str);
      u32 bind = desc.BindPoint;

      switch (desc.Type) {
      case D3D_SIT_CBUFFER: {
        bool found = false;
        for (int i = 0; i < bindings->uniform_locations.count; i++) {
          Shader_Loc *loc = &bindings->uniform_locations[i]; 
          if (str8_equal(loc->name, name)) {
            found = true;
            if (is_vertex_shader) {
              loc->vertex = bind;
            } else {
              loc->pixel = bind;
            }
            break;
          }
        }

        if (!found) {
          Shader_Loc loc;
          loc.name = name;
          if (is_vertex_shader) {
            loc.vertex = bind;
          } else {
            loc.pixel = bind;
          }
          bindings->uniform_locations.push(loc);
        }
        break;
      }

      case D3D_SIT_TEXTURE:
      {
        bool found = false;
        for (int i = 0; i < bindings->texture_locations.count; i++) {
          Shader_Loc *loc = &bindings->texture_locations[i]; 
          if (str8_equal(loc->name, name)) {
            found = true;
            if (is_vertex_shader) {
              loc->vertex = bind;
            } else {
              loc->pixel = bind;
            }
            break;
          }
        }

        if (!found) {
          Shader_Loc loc;
          loc.name = name;
          if (is_vertex_shader) {
            loc.vertex = bind;
          } else {
            loc.pixel = bind;
          }
          bindings->texture_locations.push(loc);
        }
        break;
      }

      case D3D_SIT_SAMPLER:
      {
        bool found = false;
        for (int i = 0; i < bindings->sampler_locations.count; i++) {
          Shader_Loc *loc = &bindings->sampler_locations[i]; 
          if (str8_equal(loc->name, name)) {
            found = true;
            if (is_vertex_shader) {
              loc->vertex = bind;
            } else {
              loc->pixel = bind;
            }
            break;
          }
        }

        if (!found) {
          Shader_Loc loc;
          loc.name = name;
          if (is_vertex_shader) {
            loc.vertex = bind;
          } else {
            loc.pixel = bind;
          }
          bindings->sampler_locations.push(loc);
        }
        break;
      }
      }
    }

    for (UINT cb_idx = 0; cb_idx < shader_desc.ConstantBuffers; cb_idx++) {
      ID3D11ShaderReflectionConstantBuffer *cbuffer = reflection->GetConstantBufferByIndex(cb_idx);
      D3D11_SHADER_BUFFER_DESC buffer_desc;
      cbuffer->GetDesc(&buffer_desc);

      Shader_Uniform *uniform = nullptr;
      String8 buffer_name = str8_cstring(copy_str(buffer_desc.Name));
      for (int i = 0; i < bindings->uniforms.count; i++) {
        uniform = bindings->uniforms[i];
        if (str8_equal(buffer_name, uniform->name)) {
          break;
        }
        uniform = nullptr;
      }
      if (!uniform) {
        ID3D11Buffer *uniform_buffer = make_uniform_buffer(buffer_desc.Size);
        uniform = new Shader_Uniform();
        uniform->name = buffer_name;
        uniform->buffer = uniform_buffer;
        uniform->size = buffer_desc.Size;
        uniform->memory = new u8[uniform->size];
        uniform->dirty = 0;
        bindings->uniforms.push(uniform);
     }

      for (UINT var_idx = 0; var_idx < buffer_desc.Variables; var_idx++) {
        ID3D11ShaderReflectionVariable *reflect_variable = cbuffer->GetVariableByIndex(var_idx);
        D3D11_SHADER_VARIABLE_DESC var_desc;
        reflect_variable->GetDesc(&var_desc);

        char *str = copy_str(var_desc.Name);
        String8 name = str8_cstring(str);

        Shader_Constant *found = nullptr;
        for (int i = 0; i < bindings->constants.count; i++) {
          Shader_Constant *constant = &bindings->constants[i];
          if (str8_equal(name, constant->name)) {
            found = constant;
            break;
          }
        }
        if (found) continue;

        Shader_Constant constant;
        constant.name = name;
        constant.offset = var_desc.StartOffset;
        constant.size = var_desc.Size;
        constant.uniform = uniform;
        bindings->constants.push(constant);
      }
    }
  }

  if (reflection) reflection->Release();
}

internal void r_d3d11_compile_shader(String8 file_name, String8 program_name, ID3D11VertexShader **vertex_shader, ID3D11PixelShader **pixel_shader, ID3D11InputLayout **input_layout, D3D11_INPUT_ELEMENT_DESC input_elements[], int elements_count, Shader_Bindings *bindings) {
  R_D3D11_State *d3d11_state = r_d3d11_state();
  ID3DBlob *vs_blob, *ps_blob;
  ID3DBlob *vs_error, *ps_error;

  OS_Handle file_handle = os_open_file(file_name, OS_AccessFlag_Read);
  String8 contents = {};
  if (os_valid_handle(file_handle)) {
    contents = os_read_file_string(file_handle);
    os_close_handle(file_handle);
  }

  HRESULT hr;
  int compilation_success = true;

  UINT compile_flags = D3DCOMPILE_ENABLE_STRICTNESS;
  // #ifdef _DEBUG
  compile_flags |= D3DCOMPILE_DEBUG;
  // #endif

  hr = D3DCompile(contents.data, contents.count, (LPCSTR)program_name.data, NULL, NULL, "vs_main", "vs_5_0", compile_flags, 0, &vs_blob, &vs_error);
  if (hr != S_OK) {
    compilation_success = false;
    logprint("Error compiling vertex shader %S\n", program_name);
    logprint("%s\n", vs_error->GetBufferPointer());
  }
    
  hr = D3DCompile(contents.data, contents.count, (LPCSTR)program_name.data, NULL, NULL, "ps_main", "ps_5_0", compile_flags, 0, &ps_blob, &ps_error);
  if (hr != S_OK) {
    compilation_success = false;
    logprint("Error compiling pixel shader %S\n", program_name);
    logprint("%s\n", ps_error->GetBufferPointer());
  }

  if (compilation_success) {
    hr = d3d11_state->device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), NULL, vertex_shader);

    hr = d3d11_state->device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), NULL, pixel_shader);

    hr = d3d11_state->device->CreateInputLayout(input_elements, elements_count, vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), input_layout);

    if (hr != S_OK) {
      logprint("Error creating input layout.\n");
    }
  }

  fill_shader_bindings(bindings, vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), true);
  fill_shader_bindings(bindings, ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), false);

  if (vs_blob)  vs_blob->Release();
  if (ps_blob)  ps_blob->Release();
  if (vs_error) vs_error->Release();
  if (ps_error) ps_error->Release();
}

internal Shader *r_d3d11_make_shader(String8 file_name, String8 program_name, D3D11_INPUT_ELEMENT_DESC input_elements[], int elements_count) {
  R_D3D11_State *d3d11_state = r_d3d11_state();

  Shader *shader = new Shader();
  shader->bindings = new Shader_Bindings();

  shader->name = str8_copy(d3d11_state->arena, program_name);
  shader->file_name = str8_copy(d3d11_state->arena, file_name);

  r_d3d11_compile_shader(file_name, program_name, &shader->vertex_shader, &shader->pixel_shader, &shader->input_layout, input_elements, elements_count, shader->bindings);

  OS_Handle file_handle = os_open_file(file_name, OS_AccessFlag_Read);
  if (os_valid_handle(file_handle)) {
    shader->last_write_time = os_file_last_write_time(file_handle);
    os_close_handle(file_handle);
  }

  d3d11_state->shaders.push(shader);
  return shader;
}

internal void r_d3d11_update_dirty_shaders() {
  R_D3D11_State *d3d11_state = r_d3d11_state();

  for (int i = 0; i < d3d11_state->shaders.count; i++) {
    Shader *shader = d3d11_state->shaders[i];
    OS_Handle file = os_open_file(shader->file_name, OS_AccessFlag_Read);
    u64 last_write_time = os_file_last_write_time(file);
    os_close_handle(file);

    if (shader->last_write_time != last_write_time) {
      r_d3d11_recompile_shader(shader);
    }
  }
}


Shader_Uniform* Shader_Bindings::lookup_uniform(String8 name) {
  for (int i = 0; i < uniforms.count; i++) {
    Shader_Uniform *uniform = uniforms[i];
    if (str8_equal(name, uniform->name)) {
      return uniform;
    }
  }
  return nullptr;
}
