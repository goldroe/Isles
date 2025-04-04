
cbuffer Constants : register(b0) {
  matrix projection;
};

struct Vertex_Input {
  float2 position : POSITION;
  float2 uv : TEXCOORD;
  uint argb : ARGB;
};

struct Vertex_Output {
  float4 position : SV_POSITION;
  float4 color : COLOR;
  float2 uv : TEXCOORD;
};

SamplerState diffuse_sampler : register(s0);
Texture2D diffuse_texture : register(t0);

float4 argb_to_color(uint argb) {
  float4 result;
  result.a = ((argb >> 0) & 0xFF) / 255.0f;
  result.r = ((argb >> 8) & 0xFF) / 255.0f;
  result.g = ((argb >> 16) & 0xFF) / 255.0f;
  result.b = ((argb >> 24) & 0xFF) / 255.0f;
  return result;
}

Vertex_Output vs_main(Vertex_Input input) {
  Vertex_Output output;
  output.position = mul(projection, float4(input.position, 0.0, 1.0));
  output.color = argb_to_color(input.argb);
  output.uv = input.uv;
  return output;
}

float4 ps_main(Vertex_Output input) : SV_TARGET {
  float4 diffuse_color = diffuse_texture.Sample(diffuse_sampler, input.uv);
  float4 final_color = input.color * diffuse_color;
  return final_color;
}

