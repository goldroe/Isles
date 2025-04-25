//Shadow fading rate (higher = sharper, lower = softer)
//This is the reciprocal of a regular bias (1/bias)
#define FADE 2e3
//Offset bias
#define BIAS -0.1
//Shadow map resolution
#define RES 2048.0
//Number if soft shadow samples
#define NUM 16.0

float shadow_hard(float4 proj) {
  //Project shadow map uvs (y-flipped)
  float2 uv = proj.xy / proj.w * float2(0.5, -0.5) + 0.5;
  //Difference in shadow map and current depth
  float dif = (shadow_map.Sample(shadow_sampler, uv).r - proj.z) / proj.w;
  //Map to the 0 to 1 range
  float c = clamp(dif * FADE + 2.0, 0.0, 1.0);
  return c;
}

//Interpolated shadows
float shadow_interp(float4 p, float slope) {
	//Linear sub-pixel coordinates
	float2 l = frac(p.xy / p.w * RES * 0.5);
	//Cubic interpolation
	float2 c = l*l * (3.0 - l*2.0);
	
	//Texel offsets
	float3 t = p.w / RES * float3(-0.5, +0.5, 0);
	//Offset to the nearest texel center
	float4 o = p.w / RES * float4(0.5 - l, BIAS*0.7*slope, 0);
	
	//Sample 4 nearest texels
	float s00 = shadow_hard(p + o + t.xxzz);
	float s10 = shadow_hard(p + o + t.yxzz);
	float s01 = shadow_hard(p + o + t.xyzz);
	float s11 = shadow_hard(p + o + t.yyzz);
	
	//Interpolate between samples (bi-cubic)
	return lerp(lerp(s00,s10,c.x), lerp(s01,s11,c.x), c.y);
}

//Soft, disk shadows
float shadow_soft(float4 p, float slope) {
	//Sum of shadow samples for averaging
	float sum = 0.0;
	
	//Pick a random starting direction
	// float2 dir = normalize(texture2D(u_noise, gl_FragCoord.xy/64.0).xy - 0.5);
	float2 dir = float2(1,0);
	
	//Golden angle rotation matrix
	//https://mini.gmshaders.com/i/139108917/golden-angle
	const float2x2 ang = float2x2(-0.7373688, -0.6754904, 0.6754904,  -0.7373688);
	
	//Fibonacci disk scale
  float u_radius = 4.0f;
	float scale = u_radius / RES;

	//Loop through samples in a disk (i approx. ranges from 0 to 1)
	for(float i = 0.5/NUM; i<1.0; i+=1.0/NUM)
	{
		//Rotate sample direction
		dir = mul(dir, ang);
		//Sample point radius
		float radius = scale * sqrt(i);
		
		//Add hard shadow sample
		sum += shadow_hard(radius * float4(dir, BIAS*slope, 0) + p);
	}
	return sum / NUM;
}

// float compute_shadow(float4 proj, float bias) {
//   float shadow = 1.0;
//   proj.xyz = proj.xyz / proj.w;
//   if (shadow_map.Sample(shadow_sampler, proj.xy).r < proj.z - bias) {
//     shadow = 0.0f;
//   }
//   return shadow;
// }
