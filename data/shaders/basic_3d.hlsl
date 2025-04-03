
cbuffer Constants : register(b0) {
  matrix xform;
};

struct Vertex_Input {
  float3 pos_l : POSITION;
  float2 uv : TEXCOORD;
  float4 color : COLOR;
};

struct Vertex_Output {
  float4 pos_h : SV_POSITION;
  float2 uv : TEXCOORD;
  float4 color : COLOR;
};

SamplerState diffuse_sampler;
Texture2D    diffuse_texture;

Vertex_Output vs_main(Vertex_Input input) {
  Vertex_Output output;
  output.pos_h = mul(xform, float4(input.pos_l, 1.0));
  output.uv = input.uv;
  output.color = input.color;
  return output;
}

float4 ps_main(Vertex_Output input) : SV_TARGET {
  float4 diffuse_color = diffuse_texture.Sample(diffuse_sampler, input.uv);
  float4 final_color;
  final_color = diffuse_color * input.color;
  return final_color;
}
