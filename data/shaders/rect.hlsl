cbuffer Constants : register(b0) {
    matrix xform;
};

struct Vertex_Input {
    float2 position : POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
    float4 style : STYLE; // x=omit_tex
};

struct Vertex_Output {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
    nointerpolation float4 style : STYLE;
};

SamplerState diffuse_sampler : register(s0);
Texture2D    diffuse_texture : register(t0);

Vertex_Output vs_main(Vertex_Input input) {
    Vertex_Output output;
    output.position = mul(xform, float4(input.position, 0, 1));
    output.uv = input.uv;
    output.color = input.color;
    output.style = input.style;
    return output;
}

float4 ps_main(Vertex_Output input) : SV_TARGET {
    int omit_tex = input.style.x;

    float4 diffuse = float4(1, 1, 1, 1);
    if (!omit_tex) {
        diffuse = diffuse_texture.Sample(diffuse_sampler, input.uv);   
    }

    float4 final_color;
    final_color = diffuse * input.color;
    return final_color;    
}
