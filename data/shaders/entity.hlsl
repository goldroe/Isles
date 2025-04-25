// Specular exponent for sharpness
#define SPEC_EXP 16.0
// Specular strength
#define SPEC_AMOUNT 0.5

cbuffer Constants : register(b0) {
  matrix xform;
  matrix world;
  matrix light_view_projection;
  float4 light_color;
  float3 light_direction;
  float use_override_color;
  float4 override_color;
  float3 eye;
};

struct Vertex_Input {
  float3 pos_l : POSITION;
  float3 normal : NORMAL;
  float4 color : COLOR;
  float2 uv : TEXCOORD;
};

struct Vertex_Output {
  float4 pos_h : SV_POSITION;
  float3 pos_w : POSITION;
  float3 normal : NORMAL;
  float4 color : COLOR;
  float2 uv : TEXCOORD;
  float4 shadow_coords : TEXCOORD1;
};

SamplerState diffuse_sampler : register(s0);
SamplerState shadow_sampler   : register(s1);

Texture2D diffuse_texture : register(t0);
Texture2D shadow_map      : register(t1);

// #include "shading.hlsl"

Vertex_Output vs_main(Vertex_Input input) {
  Vertex_Output output;
  output.pos_h = mul(xform, float4(input.pos_l, 1.0));
  output.pos_w = mul(world, float4(input.pos_l, 1.0)).xyz;
  output.normal = mul((float3x3)world, input.normal);
  output.color = input.color;
  if (use_override_color) {
    output.color = override_color;
  }
  output.uv = input.uv;
  float4 light_view_position = mul(light_view_projection, float4(output.pos_w, 1.0));
  output.shadow_coords = light_view_position * float4(0.5, -0.5, 1.0, 1.0) + (float4(0.5, 0.5, 0.0, 0.0) * light_view_position.w);
  // output.shadow_coords = mul(light_view_projection, float4(output.pos_w, 1.0));
  return output;
}

float4 ps_main(Vertex_Output input) : SV_TARGET {
  float4 diffuse_color = float4(1, 1, 1, 1);
  float4 texture_color = diffuse_texture.Sample(diffuse_sampler, input.uv);

  float4 ambient = 0.15 * light_color;
  float2 suv = input.shadow_coords.xy;
  float depth = input.shadow_coords.z;

  float bias = 0.005;
  float2 poisson_disk[16] = {
    float2( -0.94201624, -0.39906216 ), 
    float2( 0.94558609, -0.76890725 ), 
    float2( -0.094184101, -0.92938870 ), 
    float2( 0.34495938, 0.29387760 ), 
    float2( -0.91588581, 0.45771432 ), 
    float2( -0.81544232, -0.87912464 ), 
    float2( -0.38277543, 0.27676845 ), 
    float2( 0.97484398, 0.75648379 ), 
    float2( 0.44323325, -0.97511554 ), 
    float2( 0.53742981, -0.47373420 ), 
    float2( -0.26496911, -0.41893023 ), 
    float2( 0.79197514, 0.19090188 ), 
    float2( -0.24188840, 0.99706507 ), 
    float2( -0.81409955, 0.91437590 ), 
    float2( 0.19984126, 0.78641367 ), 
    float2( 0.14383161, -0.14100790 ) 
  };

  float d = max(dot(-light_direction, input.normal), 0.0);

  float shadow = 0.8;
  for (int i = 0; i < 4; i++) {
    int p = i;
    float shadow_depth = shadow_map.Sample(shadow_sampler, suv + poisson_disk[p] * 0.001).r;
    if (shadow_depth < depth - bias) {
      shadow -= 0.2;
    }
  }

  float4 color = ambient + shadow * diffuse_color * d;
  color = saturate(color);
  color = color * texture_color * input.color;
  // color = float4(shadow, shadow, shadow, 1.0);

  return color;
}


// void main() {
// 	//Discard below the alpha threshold
// 	vec4 col = texture2D(gm_BaseTexture, v_coord);
// 	if (col.a<0.5) discard;
	
// 	//Factor in vertex color
// 	col *= v_color;
// 	//Convert to linear RGB
// 	col.rgb = pow(col.rgb, vec3(GAMMA));
	
// 	//Compute shadow-projection-space coordinates
// 	vec4 proj = u_sha_proj * v_shadow;
	
// 	//Normalize to the -1 to +1 range (accounting for perspective)
// 	vec2 suv = proj.xy/proj.w;
// 	//Edge vignette from shadow uvs
// 	vec2 edge = max(1.0 - suv*suv, 0.0);
// 	//Shade anything outside of the shadow map
// 	float shadow = edge.x * edge.y * float(proj.z>0.0);
// 	//Normalize shadow-space normals
// 	vec3 norm = normalize(v_normal);
	
// 	//Compute slope with safe limits
// 	float slope = 1.0 / max(-norm.z, 0.1);
// 	//Only do shadow mapping inside the shadow map
// 	if (shadow>0.01) shadow *= shadow_soft(proj, slope);
// 	//Try alternative shadow functions here: shadow_hard(proj), shadow_interp(proj, slope)
	
// 	//Soft lighting
// 	float lig = max(-norm.z, 0.5-0.5*norm.z);
// 	//Blend with shadows and some ambient light
// 	lig *= lig * (shadow*0.95 + 0.05);
	
// 	//Specular reflection
// 	vec3 eye = normalize(v_eye);
// 	float ref = max(reflect(eye, norm).z, 0.0);
// 	float spec = pow(ref, SPEC_EXP) * SPEC_AMOUNT;
// 	//Screen blend specular highlights with 
// 	col.rgb = 1.0 - (1.0 - col.rgb*lig) * (1.0 - spec);
	
// 	//Convert back to sRGB
// 	col.rgb = pow(col.rgb, 1.0/vec3(GAMMA));
// 	//col.rgb = sin(eye/.1)*.5+.5;
	
// 	//Colorless test
// 	//col.rgb = vec3(shadow);
//     gl_FragColor = col;
// }

