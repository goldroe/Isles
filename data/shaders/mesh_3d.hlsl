
cbuffer Constants : register(b0) {
    matrix xform;
    matrix world_matrix;
    // matrix view_matrix;
    // matrix projection_matrix;
    float3 light_dir;
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
};

Texture2D main_tex : register(t0);
SamplerState main_sampler : register(s0);

Vertex_Output vs_main(Vertex_Input input) {
    Vertex_Output output;
    output.pos_h = mul(xform, float4(input.pos_l, 1.0));
    output.pos_w = mul(world_matrix, float4(input.pos_l, 1.0)).xyz;
    output.color = input.color;
    output.uv = input.uv;
    output.normal = mul((float3x3)world_matrix, input.normal);
    return output;
}

float4 ps_main(Vertex_Output input) : SV_TARGET {
    float4 final_color;
    float4 diffuse = main_tex.Sample(main_sampler, input.uv);

    // float light_dot = max(dot(light_dir, normalize(input.normal)), 0.0);
    // float4 light_color = float4(0.8, 0.4, 0.4, 1.0);
    // float4 ambient = 0.2 * light_color;
    // final_color = diffuse * ambient;
    // final_color += saturate(light_dot * diffuse);

    final_color = diffuse * input.color;


    return final_color;
}
