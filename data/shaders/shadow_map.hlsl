
cbuffer Constants : register(b0) {
  matrix world;
  matrix light_view_projection;
};

struct Vertex_Input {
  float3 pos_l : POSITION;
};

struct Vertex_Output {
  float4 pos_h : SV_POSITION;
};

Vertex_Output vs_main(Vertex_Input input) {
  Vertex_Output output;
  output.pos_h = mul(mul(light_view_projection, world), float4(input.pos_l, 1.0));
  return output;
}

void ps_main(Vertex_Output input) {}
