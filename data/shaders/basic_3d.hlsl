
cbuffer Constants : register(b0) {
  matrix xform;
};

struct Vertex_Input {
  float3 pos_l : POSITION;
  float4 color : COLOR;
};

struct Vertex_Output {
  float4 pos_h : SV_POSITION;
  float4 color : COLOR;
};

Vertex_Output vs_main(Vertex_Input input) {
  Vertex_Output output;
  output.pos_h = mul(xform, float4(input.pos_l, 1.0));
  output.color = input.color;
  return output;
}

float4 ps_main(Vertex_Output input) : SV_TARGET {
  float4 final_color;
  final_color = input.color;
  return final_color;
}
