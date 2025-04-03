
cbuffer Constants : register(b0) {
    matrix xform;
    float4 pick_color;
};

struct Vertex_Input {
    float3 pos_l : POSITION;
};

struct Vertex_Output {
    float4 pos_h : SV_POSITION;
};

Vertex_Output vs_main(Vertex_Input input) {
    Vertex_Output output;
    output.pos_h = mul(xform, float4(input.pos_l, 1.0));
    return output;
}

float4 ps_main(Vertex_Output input) : SV_TARGET {
    float4 result = pick_color;
    return result;
}
