global R_D3D11_State *r_g_d3d11_state;

internal inline R_D3D11_State *r_d3d11_state() {
  return r_g_d3d11_state;
}

template <class T> void SafeRelease(T **ppT) {
  if (*ppT) {
    (*ppT)->Release();
    *ppT = nullptr;
  }
}

internal void r_clear_color(f32 r, f32 g, f32 b, f32 a) {
  R_D3D11_State *d3d = r_d3d11_state();
  FLOAT color[4] = {r, g, b, a};
  d3d->devcon->ClearRenderTargetView(d3d->render_target, color);
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
  if (d3d->devcon->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res) == S_OK) {
    memcpy((u8 *)res.pData, data, bytes);
    d3d->devcon->Unmap(buffer, 0);
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

internal Blend_State *create_blend_state(D3D11_BLEND_DESC *desc) {
  Blend_State *blend_state = new Blend_State;
  R_D3D11_State *d3d = r_d3d11_state();
  HRESULT hr = d3d->device->CreateBlendState(desc, &blend_state->resource);
  return blend_state;
}

internal Depth_State *create_depth_state(D3D11_DEPTH_STENCIL_DESC *desc) {
  Depth_State *depth_state = new Depth_State;
  R_D3D11_State *d3d = r_d3d11_state();
  HRESULT hr = d3d->device->CreateDepthStencilState(desc, &depth_state->resource);
  return depth_state;
}

internal Rasterizer *create_rasterizer(D3D11_RASTERIZER_DESC *desc) {
  Rasterizer *rasterizer = new Rasterizer;
  R_D3D11_State *d3d = r_d3d11_state();
  HRESULT hr = d3d->device->CreateRasterizerState(desc, &rasterizer->resource);
  return rasterizer;
}

internal Sampler *create_sampler(D3D11_SAMPLER_DESC *desc) {
  Sampler *sampler = new Sampler;
  R_D3D11_State *d3d = r_d3d11_state();
  HRESULT hr = d3d->device->CreateSamplerState(desc, &sampler->resource);
  return sampler;
}

internal Texture *create_texture_cube(String8 file_names[6]) {
  R_D3D11_State *d3d = r_d3d11_state();

  DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

  int x, y, n;
  D3D11_SUBRESOURCE_DATA data[6] = {};
  for (int i = 0; i < 6; i++) {
    u8 *mem = stbi_load((char *)file_names[i].data, &x, &y, &n, 4);
    if (!mem) {
      logprint("Cannot load '%S'\n", file_names[i]);
    }
    data[i].pSysMem = mem;
    data[i].SysMemPitch = x * 4;
    data[i].SysMemSlicePitch = 0;
  }

  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = x;
  desc.Height = y;
  desc.MipLevels = 1;
  desc.ArraySize = 6;
  desc.Format = format;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

  ID3D11Texture2D *tex2d = NULL;
  ID3D11ShaderResourceView *view = NULL;
  HRESULT hr = d3d->device->CreateTexture2D(&desc, data, &tex2d);
  if (FAILED(hr)) {
    printf("Failed to create shader resource view\n");
    return nullptr;
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
  srv_desc.Format = desc.Format;
  srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
  srv_desc.Texture2D.MostDetailedMip = 0;
  srv_desc.Texture2D.MipLevels = 1;
  hr = d3d->device->CreateShaderResourceView(tex2d, &srv_desc, &view);
  if (FAILED(hr)) {
    printf("Failed to create texture2d\n");
    return nullptr; 
  }

  Texture *texture = new Texture;
  texture->width = x;
  texture->height = y;
  texture->format = format;
  texture->view = view;

  return texture;
}

internal Texture *create_texture(u8 *data, DXGI_FORMAT format, int w, int h, int flags) {
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
  d3d11_state->devcon->UpdateSubresource(tex2d, 0, NULL, data, w * bytes_from_format(format), 0);
    
  ID3D11ShaderResourceView *view = nullptr;
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Format = desc.Format;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = desc.MipLevels;
	srv_desc.Texture2D.MostDetailedMip = 0;
  hr = d3d11_state->device->CreateShaderResourceView(tex2d, &srv_desc, &view);

  if (flags & TEXTURE_FLAG_GENERATE_MIPS) {
    d3d11_state->devcon->GenerateMips(view); 
  }

  texture->view = view;
  return texture;
}

internal Texture *create_texture_from_file(String8 file_name, int flags) {
  int x, y, n;
  u8 *data = stbi_load((char *)file_name.data, &x, &y, &n, 4);
  if (!data) {
    logprint("Cannot load '%S'\n", file_name);
    return nullptr;
  }
  Texture *texture = create_texture(data, DXGI_FORMAT_R8G8B8A8_UNORM, x, y, flags);
  return texture;
}

internal void r_d3d11_initialize(HWND window_handle) {
  R_D3D11_State *d3d = r_d3d11_state();

  d3d->arena = arena_alloc(get_virtual_allocator(), MB(4));

  HRESULT hr;

  UINT device_flags = 0;
#if _DEBUG
  device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

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

    hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, device_flags, feature_levels, ArrayCount(feature_levels), D3D11_SDK_VERSION, &swap_chain_desc, &d3d->swap_chain, &d3d->device, NULL, &d3d->devcon);
  }

  //@Note Initialize render target view
  {
    ID3D11Texture2D *back_buffer;
    hr = d3d->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&back_buffer);
    hr = d3d->device->CreateRenderTargetView(back_buffer, NULL, &d3d->render_target);
    back_buffer->Release();
  }

  //@Note Initialize depth stencil view
  {
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = os_get_window_width((OS_Handle)window_handle);
    desc.Height = os_get_window_height((OS_Handle)window_handle);
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

  //@Note Create depth stencil states
  {
    D3D11_DEPTH_STENCIL_DESC desc = {};
    desc.DepthEnable = true;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    desc.DepthFunc = D3D11_COMPARISON_LESS;
    desc.StencilEnable = false;
    desc.FrontFace.StencilFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    desc.BackFace = desc.FrontFace;
    depth_state_default = create_depth_state(&desc);

    desc = {};
    desc.DepthEnable = false;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    desc.StencilEnable = false;
    desc.FrontFace.StencilFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    desc.BackFace = desc.FrontFace;
    depth_state_disable = create_depth_state(&desc);

    desc = {};
    desc.DepthEnable = true;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    desc.DepthFunc = D3D11_COMPARISON_LESS;
    desc.StencilEnable = false;
    desc.FrontFace.StencilFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    desc.BackFace = desc.FrontFace;
    depth_state_no_write = create_depth_state(&desc);

    desc = {};
    desc.DepthEnable = true;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    desc.StencilEnable = false;
    desc.FrontFace.StencilFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    desc.BackFace = desc.FrontFace;
    depth_state_skybox = create_depth_state(&desc);

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
    rasterizer_default = create_rasterizer(&desc);

    desc = {};
    desc.FillMode = D3D11_FILL_SOLID;
    desc.CullMode = D3D11_CULL_NONE;
    desc.FrontCounterClockwise = true;
    desc.DepthBias = 0;
    desc.DepthClipEnable = false;
    desc.ScissorEnable = false;
    desc.MultisampleEnable = false;
    desc.AntialiasedLineEnable = false;
    rasterizer_no_cull = create_rasterizer(&desc);

    desc = {};
    desc.FillMode = D3D11_FILL_SOLID;
    desc.CullMode = D3D11_CULL_FRONT;
    desc.FrontCounterClockwise = true;
    desc.DepthBias = 0;
    desc.DepthClipEnable = false;
    desc.ScissorEnable = false;
    desc.MultisampleEnable = false;
    desc.AntialiasedLineEnable = false;
    rasterizer_cull_front = create_rasterizer(&desc);

    desc = {};
    desc.FillMode = D3D11_FILL_SOLID;
    desc.CullMode = D3D11_CULL_BACK;
    desc.ScissorEnable = false;
    desc.DepthClipEnable = false;
    desc.FrontCounterClockwise = true;
    desc.MultisampleEnable = true;
    rasterizer_text = create_rasterizer(&desc);

    desc = {};
    desc.FillMode = D3D11_FILL_WIREFRAME;
    desc.CullMode = D3D11_CULL_NONE;
    desc.ScissorEnable = false;
    desc.DepthClipEnable = false;
    desc.DepthBias = -100000000;
    desc.FrontCounterClockwise = true;
    desc.MultisampleEnable = true;
    desc.AntialiasedLineEnable = false;
    rasterizer_wireframe = create_rasterizer(&desc);

    desc = {};
    desc.FillMode = D3D11_FILL_SOLID;
    desc.CullMode = D3D11_CULL_BACK;
    desc.ScissorEnable = false;
    desc.DepthClipEnable = false;
    desc.DepthBias = 100000;
    desc.DepthBiasClamp = 0.0f;
    desc.SlopeScaledDepthBias = 1.0f;
    desc.FrontCounterClockwise = true;
    desc.MultisampleEnable = false;
    desc.AntialiasedLineEnable = false;
    rasterizer_shadow_map = create_rasterizer(&desc);
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
    blend_state_default = create_blend_state(&desc);

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
    blend_state_alpha = create_blend_state(&desc);

    desc = {};
    desc.AlphaToCoverageEnable = false;
    desc.IndependentBlendEnable = false;
    desc.RenderTarget[0].BlendEnable = true;
    desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blend_state_additive = create_blend_state(&desc);
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
    sampler_linear = create_sampler(&desc);

    desc.Filter = D3D11_FILTER_ANISOTROPIC;
    desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.MaxAnisotropy = 4;
    desc.MinLOD = 0;
    desc.MaxLOD = D3D11_FLOAT32_MAX;
    desc.MipLODBias = 0;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampler_anisotropic = create_sampler(&desc);

    desc = {};
    desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.MipLODBias = 0;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampler_point = create_sampler(&desc);

    desc = {};
    desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.MinLOD = 0;
    desc.MaxLOD = D3D11_FLOAT32_MAX;
    desc.MipLODBias = 0;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampler_skybox = create_sampler(&desc);
  }

  {
    u32 white_bitmap[] = {
      0xFFFFFFFF, 0xFFFFFFFF,
      0xFFFFFFFF, 0xFFFFFFFF
    };
    d3d->fallback_tex = create_texture((u8 *)white_bitmap, DXGI_FORMAT_R8G8B8A8_UNORM, 2, 2, 0);
  }

  //@Note Compile shaders
  D3D11_INPUT_ELEMENT_DESC basic_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_basic = shader_create(str8_lit("data/shaders/basic_3d.hlsl"), str8_lit("Basic"), basic_ilay, ArrayCount(basic_ilay), false);

  D3D11_INPUT_ELEMENT_DESC mesh_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Vertex_XNCUU, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(Vertex_XNCUU, uv),       D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_mesh = shader_create(str8_lit("data/shaders/mesh.hlsl"), str8_lit("Mesh"), mesh_ilay, ArrayCount(mesh_ilay), false);

  D3D11_INPUT_ELEMENT_DESC entity_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Vertex_XNCUU, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Vertex_XNCUU, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex_XNCUU, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(Vertex_XNCUU, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_entity = shader_create(str8_lit("data/shaders/entity.hlsl"), str8_lit("Entity"), entity_ilay, ArrayCount(entity_ilay), false);
  shader_entity->bindings->xform = find_shader_constant(shader_entity, str8_lit("xform"));
  shader_entity->bindings->world = find_shader_constant(shader_entity, str8_lit("world"));
  shader_entity->bindings->diffuse_texture = find_shader_texture(shader_entity, str8_lit("diffuse_texture"));

  D3D11_INPUT_ELEMENT_DESC particle_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Particle_Pt, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Particle_Pt, color),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "SCALE",    0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(Particle_Pt, scale),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_particle = shader_create(str8_lit("data/shaders/particle.hlsl"), str8_lit("Particle"), particle_ilay, ArrayCount(particle_ilay), true);

  D3D11_INPUT_ELEMENT_DESC skinned_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Vertex_Skinned, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Vertex_Skinned, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(Vertex_Skinned, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "BONEIDS",  0, DXGI_FORMAT_R32G32B32A32_SINT,  0, offsetof(Vertex_Skinned, bone_ids), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "WEIGHTS",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex_Skinned, bone_weights), D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_skinned = shader_create(str8_lit("data/shaders/skinned.hlsl"), str8_lit("Skinned"), skinned_ilay, ArrayCount(skinned_ilay), false);

  shader_skinned_shadow_map = shader_create(str8_lit("data/shaders/skinned_shadow_map.hlsl"), str8_lit("Skinned ShadowMap"), skinned_ilay, ArrayCount(skinned_ilay), false);

  D3D11_INPUT_ELEMENT_DESC argb_texture_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "ARGB",     0, DXGI_FORMAT_R32_UINT,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_argb_texture = shader_create(str8_lit("data/shaders/argb_texture.hlsl"), str8_lit("ARGB Texture"), argb_texture_ilay, ArrayCount(argb_texture_ilay), false);

  D3D11_INPUT_ELEMENT_DESC ui_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "STYLE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_ui = shader_create(str8_lit("data/shaders/rect.hlsl"), str8_lit("UI"), ui_ilay, ArrayCount(ui_ilay), false);

  D3D11_INPUT_ELEMENT_DESC picker_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_picker = shader_create(str8_lit("data/shaders/picker.hlsl"), str8_lit("Picker"), picker_ilay, ArrayCount(picker_ilay), false);

  D3D11_INPUT_ELEMENT_DESC shadowmap_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex_XNCUU, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_shadow_map = shader_create(str8_lit("data/shaders/shadow_map.hlsl"), str8_lit("ShadowMap"), shadowmap_ilay, ArrayCount(shadowmap_ilay), false);

  D3D11_INPUT_ELEMENT_DESC color_wheel_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,  0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_color_wheel = shader_create(str8_lit("data/shaders/color_wheel.hlsl"), str8_lit("Color Wheel"), color_wheel_ilay, ArrayCount(color_wheel_ilay), false);

  D3D11_INPUT_ELEMENT_DESC water_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex_XNCUU, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex_XNCUU, normal),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_water = shader_create(str8_lit("data/shaders/water.hlsl"), str8_lit("Water"), water_ilay, ArrayCount(water_ilay), false);

  D3D11_INPUT_ELEMENT_DESC skybox_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  shader_skybox = shader_create(str8_lit("data/shaders/skybox.hlsl"), str8_lit("Skybox"), skybox_ilay, ArrayCount(skybox_ilay), false);

  // shader_create(str8_lit("data/shaders/foo.hlsl"), str8_lit("foo"), skybox_ilay, ArrayCount(skybox_ilay), false);

  sun_icon_texture = create_texture_from_file(str8_lit("data/textures/sun_icon.bmp"), 0);
  eye_of_horus_texture = create_texture_from_file(str8_lit("data/textures/eye_of_horus.png"), 0);
  flare_texture = create_texture_from_file(str8_lit("data/textures/flare0.png"), 0);
}

internal void r_d3d11_begin(OS_Handle window_handle) {
  R_D3D11_State *d3d = r_d3d11_state();
  d3d->devcon->OMSetRenderTargets(1, &d3d->render_target, d3d->depth_stencil);

  Vector2 window_dim = os_get_window_dim(window_handle);
  window_dim.x = Max(window_dim.x, 1);
  window_dim.y = Max(window_dim.y, 1);
  d3d->window_dimension = to_vec2i(window_dim);
  
  d3d->draw_region = {
    0, 0, window_dim.x, window_dim.y
  };

  HRESULT hr = S_OK;
  D3D11_VIEWPORT viewport{};
  viewport.TopLeftX = 0;
  viewport.TopLeftY = 0;
  viewport.Width = window_dim.x;
  viewport.Height = window_dim.y;
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;
  d3d->devcon->RSSetViewports(1, &viewport);

  set_rasterizer(rasterizer_default);
  d3d->devcon->ClearDepthStencilView(d3d->depth_stencil, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

  d3d->devcon->OMSetBlendState(nullptr, NULL, 0xffffffff);

  d3d->devcon->VSSetShader(nullptr, nullptr, 0);
  d3d->devcon->PSSetShader(nullptr, nullptr, 0);
  d3d->devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  Matrix4 screen_projection = ortho_rh_zo(0, (f32)d3d->window_dimension.x, 0.0f, (f32)d3d->window_dimension.y);

  set_shader(shader_argb_texture);
  set_constant(str8_lit("projection"), screen_projection);
  apply_constants();
}

internal char *copy_str(const char *src) {
  size_t len = strlen(src);
  char *dst = (char *)malloc(len + 1);
  strcpy(dst, src);
  return dst;
}

internal inline void set_shader_type_loc(Shader_Loc *loc, int shader_type, int bind) {
  int *ptr = nullptr;
  switch (shader_type) {
  case 0: ptr = &loc->vertex; break;
  case 1: ptr = &loc->pixel; break;
  case 2: ptr = &loc->geo; break;
  }
  *ptr = bind;
}

internal void fill_shader_bindings(Shader_Bindings *bindings, void *byte_code, size_t byte_code_size, int shader_type) {
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
            set_shader_type_loc(loc, shader_type, bind);
            break;
          }
        }

        if (!found) {
          Shader_Loc loc;
          loc.name = name;
          set_shader_type_loc(&loc, shader_type, bind);
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
            set_shader_type_loc(loc, shader_type, bind);
            break;
          }
        }

        if (!found) {
          Shader_Loc loc;
          loc.name = name;
          set_shader_type_loc(&loc, shader_type, bind);
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
            set_shader_type_loc(loc, shader_type, bind);
            break;
          }
        }

        if (!found) {
          Shader_Loc loc;
          loc.name = name;
          set_shader_type_loc(&loc, shader_type, bind);
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

internal void shader_compile(Shader *shader) {
  R_D3D11_State *d3d = r_d3d11_state();
  ID3DBlob *vs_blob = nullptr, *ps_blob = nullptr, *gs_blob = nullptr;
  ID3DBlob *vs_error = nullptr, *ps_error = nullptr, *gs_error = nullptr;
  int compilation_success = true;

  OS_Handle file_handle = os_open_file(shader->file_name, OS_AccessFlag_Read);
  String8 contents = {};
  if (os_valid_handle(file_handle)) {
    u64 last_write_time = os_file_last_write_time(file_handle);
    shader->last_write_time = last_write_time;
    contents = os_read_file_string(file_handle);
    os_close_handle(file_handle);
  } else {
    DWORD err = GetLastError();
    logprint("Could not read file '%S' for shader compilation: ERROR %d.\n", shader->file_name, err);
    return;
  }

  ID3D11VertexShader *vertex_shader = nullptr;
  ID3D11PixelShader *pixel_shader = nullptr;
  ID3D11GeometryShader *geometry_shader = nullptr;
  ID3D11InputLayout *input_layout = nullptr;

  HRESULT hr;
  UINT compile_flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
  compile_flags |= D3DCOMPILE_DEBUG;
#endif

  hr = D3DCompile(contents.data, contents.count, (LPCSTR)shader->file_name.data, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "vs_main", "vs_5_0", compile_flags, 0, &vs_blob, &vs_error);
  if (hr == S_OK) {
    hr = d3d->device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), NULL, &vertex_shader);
  } else {
    compilation_success = false;
    logprint("Error compiling vertex shader %S\n", shader->file_name);
    logprint("%s\n", vs_error->GetBufferPointer());
  }
    
  hr = D3DCompile(contents.data, contents.count, (LPCSTR)shader->file_name.data, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "ps_main", "ps_5_0", compile_flags, 0, &ps_blob, &ps_error);
  if (hr == S_OK) {
    hr = d3d->device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), NULL, &pixel_shader);
  } else {
    compilation_success = false;
    logprint("Error compiling pixel shader %S\n", shader->file_name);
    logprint("%s\n", ps_error->GetBufferPointer());
  }

  if (shader->has_geometry_shader) {
    hr = D3DCompile(contents.data, contents.count, (LPCSTR)shader->file_name.data, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "gs_main", "gs_5_0", compile_flags, 0, &gs_blob, &gs_error);
    if (hr == S_OK) {
      if (shader->has_geometry_shader) hr = d3d->device->CreateGeometryShader(gs_blob->GetBufferPointer(), gs_blob->GetBufferSize(), NULL, &geometry_shader);
    } else {
      compilation_success = false;
      logprint("Error compiling geometry shader %S\n", shader->file_name);
      logprint("%s\n", gs_error->GetBufferPointer());
    }
  }

  if (vs_blob) {
    hr = d3d->device->CreateInputLayout(shader->input_elements, shader->input_element_count, vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &input_layout);
    if (hr != S_OK) {
      compilation_success = false;
      logprint("Error creating input layout.\n");
    }
  }

  if (shader->initialized && compilation_success) {
    SafeRelease(&shader->input_layout);
    SafeRelease(&shader->vertex_shader);
    SafeRelease(&shader->pixel_shader);
    SafeRelease(&shader->geometry_shader);

    Shader_Bindings *bindings = shader->bindings;
    bindings->constants.clear();
    bindings->texture_locations.clear();
    bindings->sampler_locations.clear();
    bindings->uniform_locations.clear();
    for (int i = 0; i < bindings->uniforms.count; i++) {
      delete bindings->uniforms[i];
    }
    bindings->uniforms.clear();
  }

  shader->input_layout = input_layout;
  shader->vertex_shader = vertex_shader;
  shader->pixel_shader = pixel_shader;
  shader->geometry_shader = geometry_shader;
  if (vs_blob) {
    fill_shader_bindings(shader->bindings, vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), 0);
    fill_shader_bindings(shader->bindings, ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), 1);
    if (shader->has_geometry_shader) fill_shader_bindings(shader->bindings, gs_blob->GetBufferPointer(), gs_blob->GetBufferSize(), 2);
  }

  shader->initialized = true;

  if (vs_blob)  vs_blob->Release();
  if (ps_blob)  ps_blob->Release();
  if (gs_blob)  gs_blob->Release();
  if (vs_error) vs_error->Release();
  if (ps_error) ps_error->Release();
  if (gs_error) gs_error->Release();

  if (contents.data) free(contents.data);
}

internal Shader *shader_create(String8 file_name, String8 program_name, D3D11_INPUT_ELEMENT_DESC input_elements[], int element_count, bool has_geometry_shader) {
  R_D3D11_State *d3d11_state = r_d3d11_state();

  Shader *shader = new Shader();
  shader->bindings = new Shader_Bindings();

  shader->name = str8_copy(d3d11_state->arena, program_name);
  shader->file_name = str8_copy(d3d11_state->arena, file_name);
  shader->has_geometry_shader = has_geometry_shader;

  shader->input_elements = new D3D11_INPUT_ELEMENT_DESC[element_count];
  shader->input_element_count = element_count;
  MemoryCopy(shader->input_elements, input_elements, element_count * sizeof(D3D11_INPUT_ELEMENT_DESC));

  shader_compile(shader);

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
      shader_compile(shader);
    }
  }
}
