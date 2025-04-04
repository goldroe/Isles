
cbuffer Constants : register(b0) {
  matrix xform;
  matrix world;
  matrix light_xform;
};

struct Vertex_Input {
  float3 pos_l : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD;
  float4 color : COLOR;
};

struct Vertex_Output {
  float4 pos_h : SV_POSITION;
  float3 pos_w : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD;
  float4 color : COLOR;
  float4 shadow_pos : TEXCOORD1;
};

SamplerState diffuse_sampler : register(s0);
SamplerState point_sampler   : register(s1);

Texture2D diffuse_texture : register(t0);
Texture2D shadow_map      : register(t1);

Vertex_Output vs_main(Vertex_Input input) {
  Vertex_Output output;
  output.pos_h = mul(xform, float4(input.pos_l, 1.0));
  output.pos_w = mul(world, float4(input.pos_l, 1.0)).xyz;
  output.color = input.color;
  output.uv = input.uv;
  output.normal = mul((float3x3)world, input.normal);
  float4 shadow_h = mul(light_xform, float4(output.pos_w, 1.0));
  output.shadow_pos = shadow_h * float4(0.5, -0.5, 1.0, 1.0) + (float4(0.5, 0.5, 0.0, 0.0) * shadow_h.w);
  return output;
}

float4 ps_main(Vertex_Output input) : SV_TARGET {
  float4 ambient = float4(0.1, 0.1, 0.1, 1.0);
  // float light_dot = max(dot(light_dir, normalize(input.normal)), 0.0);
  // float4 light_color = float4(0.8, 0.4, 0.4, 1.0);
  // float4 ambient = 0.2 * light_color;
  // final_color = diffuse * ambient;
  // final_color += saturate(light_dot * diffuse);

  float4 final_color;
  float4 diffuse = diffuse_texture.Sample(diffuse_sampler, input.uv);

  input.shadow_pos.xyz = input.shadow_pos.xyz / input.shadow_pos.w;

  float shadow_bias = 0.005;

  if (shadow_map.Sample(point_sampler, input.shadow_pos.xy).r < input.shadow_pos.z - shadow_bias) {
    final_color = ambient * diffuse;
  } else {
    final_color = diffuse;
  }

  return final_color;
}
