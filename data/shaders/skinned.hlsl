
#define MAX_BONES 100
#define MAX_WEIGHTS 4

cbuffer Constants : register(b0) {
  matrix xform;
  matrix world;
  matrix light_view_projection;
  float4 color;
  float4 light_color;
  float3 light_direction;
  matrix bone_matrices[MAX_BONES];
};

struct Vertex_Input {
  float3 position_l : POSITION;
  float3 normal_l : NORMAL;
  float2 uv : TEXCOORD;
  float4 weights : WEIGHTS;
  int4 bone_indices : BONEIDS;
};

struct Vertex_Output {
  float4 position_h : SV_POSITION;
  float3 position_w : POSITION;
  float3 normal : NORMAL;
  float4 color : COLOR;
  float2 uv : TEXCOORD;
  float4 shadow_pos : TEXCOORD1;
};

SamplerState diffuse_sampler : register(s0);
SamplerState shadow_sampler : register(s1);

Texture2D diffuse_texture : register(t0);
Texture2D shadow_map : register(t1);

#include "shading.hlsl"

Vertex_Output vs_main(Vertex_Input input) {
  Vertex_Output output;

  float4 position_l = float4(0, 0, 0, 0);
  float3 normal_l = float3(0, 0, 0);
  for (int i = 0; i < MAX_WEIGHTS; i++) {
    if (input.bone_indices[i] == -1) continue;
    if (input.bone_indices[i] >= MAX_BONES) {
      position_l = float4(input.position_l, 1.0);
      normal_l = input.normal_l;
      break;
    }

    normal_l   += input.weights[i] * mul((float3x3)bone_matrices[input.bone_indices[i]], input.normal_l);
    position_l += input.weights[i] * mul(bone_matrices[input.bone_indices[i]], float4(input.position_l, 1.0));
  }

  output.position_h = mul(xform, position_l);
  output.position_w = mul(world, position_l).xyz;
  float4 shadow_h = mul(light_view_projection, float4(output.position_w, 1.0));
  output.shadow_pos = shadow_h;
  // output.normal = mul((float3x3)world_inv_transpose, normal_l);
  output.normal = normal_l;
  output.color = color;
  output.uv = input.uv;
  return output;
}

float4 ps_main(Vertex_Output input) : SV_TARGET {
  float4 diffuse_color = diffuse_texture.Sample(diffuse_sampler, input.uv);

  // Shadow
  float4 proj = input.shadow_pos;
  // Normalize to the -1 to +1 range (accounting for perspective)
  float2 suv = proj.xy/proj.w;
  // Edge vignette from shadow uvs
  float2 edge = max(1.0 - suv*suv, 0.0);
  // Shade anything outside of the shadow map
  float shadow = edge.x * edge.y * float(proj.z>0.0);
  float3 norm = normalize(input.normal);
  // Compute slope with safe limits
  float slope = 1.0 / max(-norm.z, 0.1);
  //Only do shadow mapping inside the shadow map
  if (shadow>0.01) shadow *= shadow_soft(proj, slope);

  float4 final_color = diffuse_color * input.color;

	//Soft lighting
	float L = max(-norm.z, 0.5-0.5*norm.z);
	//Blend with shadows and some ambient light
	L *= L * (shadow*0.95 + 0.1);
  final_color.rgb = 1.0 - (1.0 - final_color.rgb * L);

  // final_color.rgb = float3(shadow, shadow, shadow);
  return final_color;
}
