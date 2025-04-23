
cbuffer Constants : register(b0) {
  float3 camera_position;
  matrix to_view_projection;
};

struct Vertex_Input {
  float3 position_w : POSITION;
  float4 color : COLOR;
  float2 scale : SCALE;
};

struct Vertex_Output {
  float3 position_w : POSITION;
  float4 color : COLOR;
  float2 scale : SCALE;
};

struct Geometry_Output {
  float4 position_h : SV_POSITION;
  float3 position_w : POSITION;
  float4 color : COLOR;
  float2 uv : TEXCOORD;
};

Texture2D diffuse_texture;
SamplerState diffuse_sampler;

Vertex_Output vs_main(Vertex_Input input) {
  Vertex_Output output;
  output.position_w = input.position_w;
  output.color = input.color;
  output.scale = input.scale;
  return output;
}

[maxvertexcount(4)]
void gs_main(point Vertex_Output input[1], inout TriangleStream<Geometry_Output> output_stream) {
  float3 up = float3(0, 1, 0);
  float3 look = input[0].position_w - camera_position;
  look.y = 0;
  look = normalize(look);
  float3 right = cross(up, look);

  float2 half_size = 0.5 * input[0].scale;

  float3 center = input[0].position_w;
  float4 v[4];
  v[0] = float4(center - half_size.x * right - half_size.y * up, 1.0);
  v[1] = float4(center - half_size.x * right + half_size.y * up, 1.0);
  v[2] = float4(center + half_size.x * right - half_size.y * up, 1.0);
  v[3] = float4(center + half_size.x * right + half_size.y * up, 1.0);

  float2 tex_coords[4] = {
    float2(0.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 1.0f),
    float2(1.0f, 0.0f)
  };

  Geometry_Output output;
  [unroll]
  for (int i = 0; i < 4; i++) {
    output.position_h = mul(to_view_projection, v[i]);
    output.position_w = v[i].xyz;
    output.color = input[0].color;
    output.uv = tex_coords[i];
    output_stream.Append(output);
  }
}

float4 ps_main(Geometry_Output input) : SV_TARGET {
  float4 diffuse_color = diffuse_texture.Sample(diffuse_sampler, input.uv);
  float4 final_color = input.color * diffuse_color;
  return final_color;
}
