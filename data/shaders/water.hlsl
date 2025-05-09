
cbuffer Constants : register(b0) {
  matrix view_proj;
  matrix world;
  float3 eye_position;
  float tiling;
  float wave_strength;
  float move_factor;
  float near = 0.01;
  float far = 1000.0;
};

struct Vertex_Input {
  float3 position : POSITION;
  float3 normal : NORMAL;
};

struct Vertex_Output {
  float4 position_h : POSITION;
  float4 screen_space : SV_POSITION;
  float3 normal : NORMAL;
  float2 dudv : TEXCOORD;
  float3 to_eye : EYEDIR;
};

Texture2D reflection_texture : register(t0);
Texture2D refraction_texture : register(t1);
Texture2D dudv_map : register(t2);
Texture2D normal_map : register(t3);
Texture2D depth_map : register(t4);

SamplerState main_sampler : register(s0);

Vertex_Output vs_main(Vertex_Input input) {
  Vertex_Output output;
  float4 position_w = mul(world, float4(input.position, 1.0));
  output.position_h = mul(view_proj, position_w);
  output.screen_space = output.position_h;
  output.normal = input.normal;
  output.dudv = float2(input.position.x * 0.5 + 0.5, input.position.z * 0.5 + 0.5) * tiling;
  output.to_eye = eye_position - position_w.xyz;
  return output;
}

float linearize_depth(float d, float near, float far) {
  float z_n = 2.0 * d - 1.0;
  return 2.0 * near * far / (far + near - z_n * (far - near));
}

float4 ps_main(Vertex_Output input) : SV_TARGET {
  float2 ndc = (input.position_h.xy / input.position_h.w);
  float2 uv = float2(ndc.x * 0.5 + 0.5, ndc.y * 0.5 - 0.5);

  float2 reflect_uv = float2(uv.x, uv.y);
  float2 refract_uv = float2(uv.x, -uv.y);

  float depth = depth_map.Sample(main_sampler, refract_uv).r;
  float floor_distance = linearize_depth(depth, near, far);
  float water_distance = linearize_depth(input.position_h.z, near, far);

  float water_depth = (floor_distance - water_distance);

  float2 distort_uv = dudv_map.Sample(main_sampler, float2(input.dudv.x + move_factor, input.dudv.y)).xy * 0.1;
  distort_uv = input.dudv + float2(distort_uv.x, distort_uv.y + move_factor);
  float2 distortion = (dudv_map.Sample(main_sampler, distort_uv).xy * 2.0 - 1.0) * wave_strength * clamp(water_depth / 20, 0.0, 1.0);

  refract_uv += distortion;
  refract_uv = clamp(refract_uv, 0.001, 0.999);

  reflect_uv += distortion;
  reflect_uv.x = clamp(reflect_uv.x, 0.001, 0.999);
  reflect_uv.y = clamp(reflect_uv.y, -0.999, -0.001);

  float4 normal_color = normal_map.Sample(main_sampler, distort_uv);
  float3 normal = float3(normal_color.r * 2.0 - 1.0, normal_color.b * 3.0, normal_color.g * 2.0 - 1.0);
  normal = normalize(normal);

  float3 view_vector = normalize(input.to_eye);
  float refract_factor = dot(view_vector, normal);
  refract_factor = pow(refract_factor, 6.0);
  refract_factor = clamp(refract_factor, 0.0, 0.9);

  float4 murky = float4(0.0, 0.1, 0.04, 0.4);

  float4 reflect_color = reflection_texture.Sample(main_sampler, reflect_uv);
  float4 refract_color = refraction_texture.Sample(main_sampler, refract_uv);
  refract_color = lerp(refract_color, murky, clamp(water_depth * 2.0f, 0.0, 1.0));

  float4 final_color = lerp(reflect_color, refract_color, refract_factor);
  final_color.a = clamp(water_depth * 10.0f, 0.0, 1.0);
  return final_color;
}
