
cbuffer Constants : register(b0) {
  matrix projection;
  float2 center;
  float radius;
};

struct Vertex_Input {
  float2 position_l : POSITION;
};

struct Vertex_Output {
  float2 position : POSITION;
  float4 position_h : SV_POSITION;
};

float3 color_wheel(float2 NDC) {
  const float invPI = 0.31830988618;
  const int N = 6;
  float3 c[N] = { 
    float3(0,1,1), float3(0,1,0), float3(1,1,0),
    float3(1,0,0), float3(1,0,1), float3(0,0,1)};

  float a = (atan2(NDC.y, NDC.x) * invPI + 1.0) * float(N) * 0.5;
  int i = int(a);
  return lerp(c[i], c[(i+1)%N], frac(a));
  // return lerp(float3(1,1,1), lerp(c[i], c[(i+1)%N], frac(a)), length(NDC));
}

Vertex_Output vs_main(Vertex_Input input) {
  Vertex_Output output;
  
  matrix scale = {
    radius, 0, 0, 0,
    0, radius, 0, 0,
    0, 0, radius, 0,
    0, 0, 0, 1
  };
  matrix translate = {
    1, 0, 0, center.x,
    0, 1, 0, center.y,
    0, 0, 1, 0,
    0, 0, 0, 1
  };
  matrix transform = mul(projection, mul(translate, scale));

  output.position = mul(translate, mul(scale, float4(input.position_l, 0, 1)));
  output.position_h = mul(transform, float4(input.position_l, 0, 1));
  return output;
}

float4 ps_main(Vertex_Output input) : SV_TARGET {
  float2 uv = (input.position.xy - center) / radius;

  float4 final_color = lerp(float4(0, 0, 0, 0), float4(color_wheel(uv), 1), smoothstep(0, 0.002, 0.5 - length(uv)));

  return final_color;
}
