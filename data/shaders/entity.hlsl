
cbuffer Constants : register(b0) {
  matrix xform;
  matrix world;
  matrix light_view_projection;
  float4 light_color;
  float3 light_direction;
  float use_override_color;
  float4 override_color;
};

struct Vertex_Input {
  float3 pos_l : POSITION;
  float3 normal : NORMAL;
  float4 color : COLOR;
  float2 uv : TEXCOORD;
};

struct Vertex_Output {
  float4 pos_h : SV_POSITION;
  float3 pos_w : POSITION;
  float3 normal : NORMAL;
  float4 color : COLOR;
  float2 uv : TEXCOORD;
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
  output.normal = mul((float3x3)world, input.normal);
  output.color = input.color;
  if (use_override_color) {
    output.color = override_color;
  }
  output.uv = input.uv;
  float4 shadow_h = mul(light_view_projection, float4(output.pos_w, 1.0));
  output.shadow_pos = shadow_h * float4(0.5, -0.5, 1.0, 1.0) + (float4(0.5, 0.5, 0.0, 0.0) * shadow_h.w);
  return output;
}

float4 ps_main(Vertex_Output input) : SV_TARGET {
  float4 diffuse_color = diffuse_texture.Sample(diffuse_sampler, input.uv);

  float light_dot = max(dot(light_direction, normalize(input.normal)), 0.0);
  float4 ambient = 0.15 * light_color;

  float d = max(dot(-light_direction, input.normal), 0.0);
  float4 diffuse = d * light_color;

  input.shadow_pos.xyz = input.shadow_pos.xyz / input.shadow_pos.w;

  float shadow_bias;
  /* float shadow_bias = max(0.005 * (1.0 - dot(input.normal, -light_direction)), 0.005); */
  shadow_bias = 0.001;

  float shadow = 0.0f;
  if (shadow_map.Sample(point_sampler, input.shadow_pos.xy).r < input.shadow_pos.z - shadow_bias) {
    shadow = 1.0f;
  }

  float4 lighting = (ambient + (1.0 - shadow) * diffuse) * diffuse_color * input.color;
  float4 final_color = float4(lighting.xyz, 1.0);
  return final_color;
}
