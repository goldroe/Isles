#define MAX_BONES 100
#define MAX_WEIGHTS 4

cbuffer Constants : register(b0) {
  matrix world;
  matrix light_view_projection;
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
};

Vertex_Output vs_main(Vertex_Input input) {
  Vertex_Output output;

  float4 position_l = float4(0, 0, 0, 0);
  for (int i = 0; i < MAX_WEIGHTS; i++) {
    if (input.bone_indices[i] == -1) continue;
    if (input.bone_indices[i] >= MAX_BONES) {
      position_l = float4(input.position_l, 1.0);
      break;
    }

    position_l += input.weights[i] * mul(bone_matrices[input.bone_indices[i]], float4(input.position_l, 1.0));
  }

  output.position_h = mul(mul(light_view_projection, world), position_l);
  return output;
}

void ps_main(Vertex_Output input) {
}
