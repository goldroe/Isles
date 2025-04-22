
#define MAX_BONES 100
#define MAX_WEIGHTS 4

cbuffer Constants : register(b0) {
  matrix xform;
  float4 color;
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
  float3 normal_w : NORMAL;
  float4 color : COLOR;
  float2 uv : TEXCOORD;
};

Texture2D diffuse_texture : register(t0);
SamplerState diffuse_sampler : register(s0);

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
  // output.normal_w = mul((float3x3)world_inv_transpose, normal_l);
  // output.normal_w = normal_l;
  output.color = color;
  output.uv = input.uv;
  return output;
}

float4 ps_main(Vertex_Output input) : SV_TARGET {
  float4 diffuse_color = diffuse_texture.Sample(diffuse_sampler, input.uv);
  float4 final_color = input.color * diffuse_color;
  return final_color;
}
