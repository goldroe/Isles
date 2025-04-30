
internal Render_Target *make_render_target(int width, int height, DXGI_FORMAT format, DXGI_FORMAT depth_format) {
  R_D3D11_State *d3d = r_d3d11_state();
  Render_Target *render_target = new Render_Target();
  render_target->width = width;
  render_target->height = height;

  ID3D11Texture2D *tex2d = nullptr;
  ID3D11RenderTargetView *render_target_view = nullptr;
  ID3D11DepthStencilView *depth_stencil_view = nullptr;
  ID3D11ShaderResourceView *srv = nullptr;

  HRESULT hr = S_OK;
  {
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    hr = d3d->device->CreateTexture2D(&desc, NULL, &tex2d);

    D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
    rtv_desc.Format = desc.Format;
    rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtv_desc.Texture2D.MipSlice = 0;
    hr = d3d->device->CreateRenderTargetView(tex2d, &rtv_desc, &render_target_view);

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format = desc.Format;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;
    srv_desc.Texture2D.MostDetailedMip = 0;
    hr = d3d->device->CreateShaderResourceView(tex2d, &srv_desc, &srv);
  }

  {
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = depth_format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    ID3D11Texture2D *depth_stencil_buffer = nullptr;
    hr = d3d->device->CreateTexture2D(&desc, NULL, &depth_stencil_buffer);
    hr = d3d->device->CreateDepthStencilView(depth_stencil_buffer, NULL, &depth_stencil_view);
  }

  Texture *texture = new Texture();
  texture->width = width;
  texture->height = height;
  texture->format = format;
  texture->view = srv;
  render_target->texture = texture;

  render_target->render_target_view = render_target_view;
  render_target->depth_stencil_view = depth_stencil_view;

  return render_target;
}

internal void clear_render_target(Render_Target *render_target, f32 r, f32 g, f32 b, f32 a) {
  R_D3D11_State *d3d = r_d3d11_state();
  f32 color[4] = {r, g, b, a};
  d3d->device_context->ClearRenderTargetView(render_target->render_target_view, color);
}
