
internal Render_Target *make_render_target(int width, int height, DXGI_FORMAT format) {
  R_D3D11_State *d3d = r_d3d11_state();
  Render_Target *render_target = new Render_Target();
  render_target->width = width;
  render_target->height = height;

  ID3D11Texture2D *texture = nullptr;
  ID3D11RenderTargetView *render_target_view = nullptr;
  ID3D11DepthStencilView *depth_stencil_view = nullptr;

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
    desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    hr = d3d->device->CreateTexture2D(&desc, NULL, &texture);

    D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
    rtv_desc.Format = desc.Format;
    rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtv_desc.Texture2D.MipSlice = 0;
    hr = d3d->device->CreateRenderTargetView(texture, &rtv_desc, &render_target_view);
  }

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
    hr = d3d->device->CreateDepthStencilView(depth_stencil_buffer, NULL, &depth_stencil_view);
  }

  render_target->texture = texture;
  render_target->render_target_view = render_target_view;
  render_target->depth_stencil_view = depth_stencil_view;

  return render_target;
}
