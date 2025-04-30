
cbuffer Per_Frame : register(b0) {
  matrix xform;
};

struct Vertex_Input {
  float3 position : POSITION;
};

struct Vertex_Output {
  float4 position_h : SV_POSITION;
  float3 position : POSITION;
};

SamplerState skybox_sampler;
TextureCube skybox_texture;

Vertex_Output vs_main(Vertex_Input input) {
  Vertex_Output output;
  output.position_h = mul(xform, float4(input.position, 1.0)).xyww;
  output.position = input.position;
  return output;
}

float4 ps_main(Vertex_Output input) : SV_TARGET {
  float4 texture_color = skybox_texture.Sample(skybox_sampler, input.position);
  float4 final_color = texture_color;
  return final_color;
}
  
