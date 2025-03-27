
global R_D3D11_State *r_g_d3d11_state;

internal inline R_D3D11_State *r_d3d11_state() {
  return r_g_d3d11_state;
}

internal void r_clear_color(f32 r, f32 g, f32 b, f32 a) {
  R_D3D11_State *d3d11_state = r_d3d11_state();
  d3d11_state->clear_color = Vector4(r, g, b, a);
}

internal DXGI_FORMAT r_dxgi_format_from(R_Texture_Format format) {
  switch (format) {
  case R_TEXTURE_FORMAT_R8G8B8A8:
    return DXGI_FORMAT_R8G8B8A8_UNORM;
  }
  return DXGI_FORMAT_UNKNOWN;
}

internal u32 r_size_from_format(R_Texture_Format format) {
  switch (format) {
  case R_TEXTURE_FORMAT_R8G8B8A8:
    return 4;
  }
  return 0;
}

UINT get_num_mip_levels(UINT width, UINT height) {
  UINT levels = 1;
  while(width > 1 || height > 1) {
    width = max(width / 2, 1);
    height = max(height / 2, 1);
    ++levels;
  }
  return levels;
}

internal Texture *r_create_texture(u8 *data, R_Texture_Format format, int w, int h) {
  HRESULT hr = S_OK;

  UINT mip_levels = get_num_mip_levels(w, h);

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
  desc.Format = r_dxgi_format_from(format);
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

  hr = d3d11_state->device->CreateTexture2D(&desc, NULL, &tex2d);
  d3d11_state->device_context->UpdateSubresource(tex2d, 0, NULL, data, w * r_size_from_format(format), 0);
    
  ID3D11ShaderResourceView *view = nullptr;
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Format = desc.Format;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = desc.MipLevels;
	srv_desc.Texture2D.MostDetailedMip = 0;
  hr = d3d11_state->device->CreateShaderResourceView(tex2d, &srv_desc, &view);

  d3d11_state->device_context->GenerateMips(view);

  texture->view = (void *)view;
  return texture;
}

internal Texture *r_create_texture_from_file(String8 file_name) {
  int x, y, n;
  u8 *data = stbi_load((char *)file_name.data, &x, &y, &n, 4);
  if (!data) {
    fprintf(stderr, "cannot load '%s'\n", (char *)file_name.data);
  }
  Texture *texture = r_create_texture(data, R_TEXTURE_FORMAT_R8G8B8A8, x, y);
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

  // Release all outstanding references to the swap chain's buffers.
  if (d3d11_state->render_target_view) d3d11_state->render_target_view->Release();

  // Preserve the existing buffer count and format.
  // Automatically choose the width and height to match the client rect for HWNDs.
  hr = d3d11_state->swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

  // Get buffer and create a render-target-view.
  ID3D11Texture2D *backbuffer;
  hr = d3d11_state->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&backbuffer);

  hr = d3d11_state->device->CreateRenderTargetView(backbuffer, NULL, &d3d11_state->render_target_view);

  backbuffer->Release();

  if (d3d11_state->depth_stencil_view) d3d11_state->depth_stencil_view->Release();

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
    hr = d3d11_state->device->CreateDepthStencilView(depth_stencil_buffer, NULL, &d3d11_state->depth_stencil_view);
  }

  d3d11_state->device_context->OMSetRenderTargets(1, &d3d11_state->render_target_view, d3d11_state->depth_stencil_view);
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
    ID3D11Texture2D *back_buffer;
    hr = d3d11_state->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&back_buffer);
    hr = d3d11_state->device->CreateRenderTargetView(back_buffer, NULL, &d3d11_state->render_target_view);
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
    d3d11_state->device->CreateDepthStencilState(&desc, &d3d11_state->depth_stencil_states[R_DEPTH_STENCIL_STATE_DEFAULT]);

    memset(&desc, sizeof(desc), 0);
    desc.DepthEnable = false;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    desc.StencilEnable = false;
    desc.FrontFace.StencilFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    desc.BackFace = desc.FrontFace;
    d3d11_state->device->CreateDepthStencilState(&desc, &d3d11_state->depth_stencil_states[R_DEPTH_STENCIL_STATE_DISABLE]);
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
    d3d11_state->device->CreateRasterizerState(&desc, &d3d11_state->rasterizer_states[R_RASTERIZER_STATE_DEFAULT]);

    desc = {};
    desc.FillMode = D3D11_FILL_SOLID;
    desc.CullMode = D3D11_CULL_NONE;
    desc.ScissorEnable = false;
    desc.DepthClipEnable = false;
    desc.FrontCounterClockwise = true;
    desc.MultisampleEnable = true;
    d3d11_state->device->CreateRasterizerState(&desc, &d3d11_state->rasterizer_states[R_RASTERIZER_STATE_TEXT]);
  }

  {
    D3D11_BLEND_DESC desc = {};
    desc.AlphaToCoverageEnable = false;
    desc.IndependentBlendEnable = false;
    D3D11_RENDER_TARGET_BLEND_DESC rt = {};
    rt.BlendEnable = true;
    rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    rt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    rt.BlendOp = D3D11_BLEND_OP_ADD;
    rt.SrcBlendAlpha = D3D11_BLEND_ONE;
    rt.DestBlendAlpha = D3D11_BLEND_ZERO;
    // rt.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    desc.RenderTarget[0] = rt;
    d3d11_state->device->CreateBlendState(&desc, &d3d11_state->blend_states[R_BLEND_STATE_ALPHA]);
  }

  //@Note Create Sampler states
  {
    D3D11_SAMPLER_DESC desc = {};
    // desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    // desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    // desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    // desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    // desc.MinLOD = 0;
    // desc.MaxLOD = D3D11_FLOAT32_MAX;
    // desc.MipLODBias = 0;
    // desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    desc.Filter = D3D11_FILTER_ANISOTROPIC;
    desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.MaxAnisotropy = 4;
    desc.MinLOD = 0;
    desc.MaxLOD = D3D11_FLOAT32_MAX;
    desc.MipLODBias = 0;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    d3d11_state->device->CreateSamplerState(&desc, &d3d11_state->sampler_states[R_SAMPLER_STATE_LINEAR]);

    desc = {};
    desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.MipLODBias = 0;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    d3d11_state->device->CreateSamplerState(&desc, &d3d11_state->sampler_states[R_SAMPLER_STATE_POINT]);
  }

  {
    u32 white_bitmap[] = {
      0xFFFFFFFF, 0xFFFFFFFF,
      0xFFFFFFFF, 0xFFFFFFFF
    };
    d3d11_state->fallback_tex = (ID3D11ShaderResourceView *)r_create_texture((u8 *)white_bitmap, R_TEXTURE_FORMAT_R8G8B8A8, 2, 2)->view;
  }

  {
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = sizeof(R_D3D11_Uniform_Basic_3D);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    d3d11_state->device->CreateBuffer(&desc, nullptr, &d3d11_state->uniform_buffers[D3D11_UNIFORM_BASIC_3D]);

    desc.ByteWidth = sizeof(R_D3D11_Uniform_Picker);
    d3d11_state->device->CreateBuffer(&desc, nullptr, &d3d11_state->uniform_buffers[D3D11_UNIFORM_PICKER]);

    desc.ByteWidth = sizeof(R_D3D11_Uniform_Rect);
    d3d11_state->device->CreateBuffer(&desc, nullptr, &d3d11_state->uniform_buffers[D3D11_UNIFORM_RECT]);
  }

  //@Note Compile shaders
  D3D11_INPUT_ELEMENT_DESC mesh_3d_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  r_d3d11_make_shader(str8_lit("data/shaders/mesh_3d.hlsl"), str8_lit("Mesh3D"), D3D11_SHADER_BASIC_3D, mesh_3d_ilay, ArrayCount(mesh_3d_ilay));

  D3D11_INPUT_ELEMENT_DESC rect_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "STYLE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  r_d3d11_make_shader(str8_lit("data/shaders/rect.hlsl"), str8_lit("Rect"), D3D11_SHADER_RECT, rect_ilay, ArrayCount(rect_ilay));


  D3D11_INPUT_ELEMENT_DESC picker_ilay[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  r_d3d11_make_shader(str8_lit("data/shaders/picker.hlsl"), str8_lit("Picker"), D3D11_SHADER_PICKER, picker_ilay, ArrayCount(picker_ilay));
}

internal void r_d3d11_render(OS_Handle window_handle, Draw_Bucket_List *bucket_list) {
  R_D3D11_State *d3d11_state = r_d3d11_state();

  d3d11_state->device_context->OMSetRenderTargets(1, &d3d11_state->render_target_view, d3d11_state->depth_stencil_view);

  HRESULT hr = S_OK;
  D3D11_VIEWPORT viewport{};
  viewport.TopLeftX = d3d11_state->draw_region.x0;
  viewport.TopLeftY = d3d11_state->draw_region.y0;
  viewport.Width = rect_width(d3d11_state->draw_region);
  viewport.Height = rect_height(d3d11_state->draw_region);
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;
  d3d11_state->device_context->RSSetViewports(1, &viewport);

  float clear_color[4] = { d3d11_state->clear_color.x, d3d11_state->clear_color.y, d3d11_state->clear_color.z, d3d11_state->clear_color.w };
  d3d11_state->device_context->ClearRenderTargetView(d3d11_state->render_target_view, clear_color);
  d3d11_state->device_context->ClearDepthStencilView(d3d11_state->depth_stencil_view, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

  d3d11_state->device_context->OMSetBlendState(nullptr, NULL, 0xffffffff);

  for (Draw_Bucket *bucket = bucket_list->first; bucket; bucket = bucket->next) {
    switch (bucket->kind) {
    case DRAW_BUCKET_IMMEDIATE:
    {
      if (bucket->immediate.clear_depth_buffer) {
        d3d11_state->device_context->ClearDepthStencilView(d3d11_state->depth_stencil_view, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
      }
      ID3D11DepthStencilState *depth_stencil_state = d3d11_state->depth_stencil_states[bucket->immediate.depth_state];
      d3d11_state->device_context->OMSetDepthStencilState(depth_stencil_state, 0);
      d3d11_state->device_context->RSSetState(d3d11_state->rasterizer_states[R_RASTERIZER_STATE_DEFAULT]);

      ID3D11BlendState *blend_state = d3d11_state->blend_states[bucket->immediate.blend_state];
      d3d11_state->device_context->OMSetBlendState(blend_state, NULL, 0xffffffff);

      ID3D11Buffer *uniform_buffer = d3d11_state->uniform_buffers[D3D11_UNIFORM_BASIC_3D];
      d3d11_state->device_context->VSSetConstantBuffers(0, 1, &uniform_buffer);
      d3d11_state->device_context->PSSetConstantBuffers(0, 1, &uniform_buffer);

      d3d11_state->device_context->PSSetSamplers(0, 1, &d3d11_state->sampler_states[R_SAMPLER_STATE_LINEAR]);

      Shader *shader = d3d11_state->shaders[D3D11_SHADER_BASIC_3D];
            
      ID3D11VertexShader *vertex_shader = shader->vertex_shader;
      ID3D11PixelShader *pixel_shader = shader->pixel_shader;
      d3d11_state->device_context->VSSetShader(vertex_shader, nullptr, 0);
      d3d11_state->device_context->PSSetShader(pixel_shader, nullptr, 0);

      ID3D11InputLayout *input_layout = shader->input_layout;
      d3d11_state->device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
      d3d11_state->device_context->IASetInputLayout(input_layout);

      for (Draw_Immediate_Batch *batch = bucket->immediate.batches.first; batch; batch = batch->next) {
        if (batch->texture) {
          d3d11_state->device_context->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)&batch->texture->view);
        } else {
          d3d11_state->device_context->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)&d3d11_state->fallback_tex);
        }

        R_D3D11_Uniform_Basic_3D uniform_mesh = {};
        uniform_mesh.transform = batch->projection * batch->view * batch->world;
        uniform_mesh.world_matrix = batch->world;
        // uniform_mesh.projection_matrix = batch->projection;
        // uniform_mesh.view_matrix = batch->view;
        uniform_mesh.light_dir = bucket->immediate.light_dir;

        {
          D3D11_MAPPED_SUBRESOURCE resource = {};
          if (d3d11_state->device_context->Map(uniform_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource) == S_OK) {
            memcpy(resource.pData, &uniform_mesh, sizeof(uniform_mesh));
            d3d11_state->device_context->Unmap(uniform_buffer, 0);
          }
        }

        int vertices_count = (int)batch->vertices.count;
        Vertex_3D *vertices = batch->vertices.data;

        ID3D11Buffer *vertex_buffer = nullptr;
        {
          D3D11_BUFFER_DESC desc = {};
          desc.Usage = D3D11_USAGE_DEFAULT;
          desc.ByteWidth = sizeof(Vertex_3D) * vertices_count;
          desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
          desc.CPUAccessFlags = 0;
          desc.MiscFlags = 0;
          D3D11_SUBRESOURCE_DATA data = {};
          data.pSysMem = vertices;
          data.SysMemPitch = 0;
          data.SysMemSlicePitch = 0;
          hr = d3d11_state->device->CreateBuffer(&desc, &data, &vertex_buffer);
        }
        UINT stride = sizeof(Vertex_3D), offset = 0;
        d3d11_state->device_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);

        /*
          int indices_count = vertices_count * 6 / 4;
          u32 *indices = (u32 *)malloc(indices_count * sizeof(u32));

          for (int index = 0, it = 0; index < vertices_count; index += 4) {
          indices[it++] = index + 0;
          indices[it++] = index + 1;
          indices[it++] = index + 2;
          indices[it++] = index + 0;
          indices[it++] = index + 2;
          indices[it++] = index + 3;
          }

          ID3D11Buffer *index_buffer = nullptr;
          {
          D3D11_BUFFER_DESC desc = {};
          desc.Usage = D3D11_USAGE_DEFAULT;
          desc.ByteWidth = sizeof(u32) * indices_count;
          desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
          desc.CPUAccessFlags = 0;
          desc.MiscFlags = 0;
          D3D11_SUBRESOURCE_DATA data = {};
          data.pSysMem = indices;
          data.SysMemPitch = 0;
          data.SysMemSlicePitch = 0;
          hr = d3d11_state->device->CreateBuffer(&desc, &data, &index_buffer);
          }
          d3d11_state->device_context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);
        */
        // d3d11_state->device_context->DrawIndexed((UINT)indices_count, 0, 0);

        d3d11_state->device_context->Draw(vertices_count, 0);

        // if (index_buffer) index_buffer->Release();
        if (vertex_buffer) vertex_buffer->Release();
        batch->vertices.clear();
        // free(indices);
      }
      break;
    }
    }
  }
}

internal void r_d3d11_recompile_shader(D3D11_Shader_Kind kind) {
  R_D3D11_State *d3d11_state = r_d3d11_state();
  Shader *shader = d3d11_state->shaders[kind];
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
    printf("Error compiling vertex shader %s\n", (char *)shader->name.data);
    printf("%s\n", (char *)vs_error->GetBufferPointer());
  }
    
  hr = D3DCompile(contents.data, contents.count, (LPCSTR)shader->name.data, NULL, NULL, "ps_main", "ps_5_0", compile_flags, 0, &ps_blob, &ps_error);
  if (hr != S_OK) {
    compilation_success = false;
    printf("Error compiling pixel shader %s\n", (char *)shader->name.data);
    printf("%s\n", (char *)ps_error->GetBufferPointer());
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

internal void r_d3d11_compile_shader(String8 file_name, String8 program_name, ID3D11VertexShader **vertex_shader, ID3D11PixelShader **pixel_shader, ID3D11InputLayout **input_layout, D3D11_INPUT_ELEMENT_DESC input_elements[], int elements_count) {
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
    printf("Error compiling vertex shader %s\n", (char *)program_name.data);
    printf("%s\n", (char *)vs_error->GetBufferPointer());
  }
    
  hr = D3DCompile(contents.data, contents.count, (LPCSTR)program_name.data, NULL, NULL, "ps_main", "ps_5_0", compile_flags, 0, &ps_blob, &ps_error);
  if (hr != S_OK) {
    compilation_success = false;
    printf("Error compiling pixel shader %s\n", (char *)program_name.data);
    printf("%s\n", (char *)ps_error->GetBufferPointer());
  }

  if (compilation_success) {
    hr = d3d11_state->device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), NULL, vertex_shader);

    hr = d3d11_state->device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), NULL, pixel_shader);

    hr = d3d11_state->device->CreateInputLayout(input_elements, elements_count, vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), input_layout);
  }

  if (vs_blob)  vs_blob->Release();
  if (ps_blob)  ps_blob->Release();
  if (vs_error) vs_error->Release();
  if (ps_error) ps_error->Release();
}

internal Shader *r_d3d11_make_shader(String8 file_name, String8 program_name, D3D11_Shader_Kind shader_kind, D3D11_INPUT_ELEMENT_DESC input_elements[], int elements_count) {
  R_D3D11_State *d3d11_state = r_d3d11_state();

  Shader *shader = new Shader();
  shader->name = str8_copy(d3d11_state->arena, program_name);
  shader->file_name = str8_copy(d3d11_state->arena, file_name);

  r_d3d11_compile_shader(file_name, program_name, &shader->vertex_shader, &shader->pixel_shader, &shader->input_layout, input_elements, elements_count);

  OS_Handle file_handle = os_open_file(file_name, OS_AccessFlag_Read);
  if (os_valid_handle(file_handle)) {
    shader->last_write_time = os_file_last_write_time(file_handle);
    os_close_handle(file_handle);
  }

  d3d11_state->shaders[shader_kind] = shader;
  return shader;
}

internal void r_d3d11_update_dirty_shaders() {
  R_D3D11_State *d3d11_state = r_d3d11_state();

  for (int i = D3D11_SHADER_BASIC_3D; i < D3D11_SHADER_COUNT; i++) {
    D3D11_Shader_Kind shader_kind = (D3D11_Shader_Kind)i;
    Shader *shader = d3d11_state->shaders[i];

    if (!shader) continue;

    OS_Handle file = os_open_file(shader->file_name, OS_AccessFlag_Read);
    u64 last_write_time = os_file_last_write_time(file);
    os_close_handle(file);

    if (shader->last_write_time != last_write_time) {
      r_d3d11_recompile_shader(shader_kind);

    }
  }
}
