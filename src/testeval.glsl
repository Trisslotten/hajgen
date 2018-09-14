#version 440 core

layout(quads, equal_spacing, cw) in;

in vec2 tcPatchPos[];

out vec2 tetex;
out vec3 teposition;

out vec3 tenormal;

uniform sampler2D heightmap;
uniform vec3 size;
uniform mat4 proj_view;

uniform sampler2D texRockDisp;

uniform float patchSize;
uniform vec2 hmPos;


float CubicHermite (float A, float B, float C, float D, float t)
{
	float t2 = t*t;
    float t3 = t*t*t;
    float a = -A/2.0 + (3.0*B)/2.0 - (3.0*C)/2.0 + D/2.0;
    float b = A - (5.0*B)/2.0 + 2.0*C - D / 2.0;
    float c = -A/2.0 + C/2.0;
   	float d = B;
    
    return a*t3 + b*t2 + c*t + d;
}
float BicubicHermite(sampler2D sampler, vec2 P)
{

	float c_textureSize = textureSize(heightmap, 0).x;
    vec2 pixel = P * c_textureSize + 0.5;

	float c_onePixel = 1.0/c_textureSize;
	float c_twoPixels = 2.0*c_onePixel;
    
    vec2 frac = fract(pixel);
    pixel = floor(pixel) / c_textureSize - vec2(c_onePixel/2.0);
    
    float C00 = texture(sampler, pixel + vec2(-c_onePixel ,-c_onePixel)).r;
    float C10 = texture(sampler, pixel + vec2( 0.0        ,-c_onePixel)).r;
    float C20 = texture(sampler, pixel + vec2( c_onePixel ,-c_onePixel)).r;
    float C30 = texture(sampler, pixel + vec2( c_twoPixels,-c_onePixel)).r;
    
    float C01 = texture(sampler, pixel + vec2(-c_onePixel , 0.0)).r;
    float C11 = texture(sampler, pixel + vec2( 0.0        , 0.0)).r;
    float C21 = texture(sampler, pixel + vec2( c_onePixel , 0.0)).r;
    float C31 = texture(sampler, pixel + vec2( c_twoPixels, 0.0)).r;    
    
    float C02 = texture(sampler, pixel + vec2(-c_onePixel , c_onePixel)).r;
    float C12 = texture(sampler, pixel + vec2( 0.0        , c_onePixel)).r;
    float C22 = texture(sampler, pixel + vec2( c_onePixel , c_onePixel)).r;
    float C32 = texture(sampler, pixel + vec2( c_twoPixels, c_onePixel)).r;    
    
    float C03 = texture(sampler, pixel + vec2(-c_onePixel , c_twoPixels)).r;
    float C13 = texture(sampler, pixel + vec2( 0.0        , c_twoPixels)).r;
    float C23 = texture(sampler, pixel + vec2( c_onePixel , c_twoPixels)).r;
    float C33 = texture(sampler, pixel + vec2( c_twoPixels, c_twoPixels)).r;    
    
    float CP0X = CubicHermite(C00, C10, C20, C30, frac.x);
    float CP1X = CubicHermite(C01, C11, C21, C31, frac.x);
    float CP2X = CubicHermite(C02, C12, C22, C32, frac.x);
    float CP3X = CubicHermite(C03, C13, C23, C33, frac.x);
    
    return CubicHermite(CP0X, CP1X, CP2X, CP3X, frac.y);
}





vec3 sampleNormal(vec2 uv) {
	vec2 heightmapSize = vec2(textureSize(heightmap, 0));
	float t = 1.0/heightmapSize.x;

	vec4 h;
	/*
	h[0] = texture(heightmap, uv + t * vec2(0,-1)).r;
	h[1] = texture(heightmap, uv + t * vec2(-1,0)).r;
	h[2] = texture(heightmap, uv + t * vec2(1, 0)).r;
	h[3] = texture(heightmap, uv + t * vec2(0, 1)).r;
	*/

	h[0] = BicubicHermite(heightmap, uv + t * vec2(0,-1));
	h[1] = BicubicHermite(heightmap, uv + t * vec2(-1,0));
	h[2] = BicubicHermite(heightmap, uv + t * vec2(1, 0));
	h[3] = BicubicHermite(heightmap, uv + t * vec2(0, 1));



	float ratioX = t*size.x/(size.y);
	float ratioZ = t*size.z/(size.y);
	vec3 n;
	n.x = ratioX * (h[1] - h[2]);
	n.z = ratioZ * (h[0] - h[3]);
	n.y = 2 * ratioX*ratioZ;
	return normalize(n);
}

void main() {
	vec2 pos = tcPatchPos[0] + patchSize * gl_TessCoord.xy;
	
	vec2 uv = pos / size.xz;
	float t = 1.5/textureSize(heightmap, 0).x;
	uv = (1.0 - 2.0*t)*uv + t;

	tetex = uv;

	teposition.xz = pos.xy;
	//teposition.y = size.y * BicubicHermite(heightmap, uv);
	teposition.y = size.y * texture(heightmap, uv).r;
	teposition.xz += size.xz * hmPos;

	tenormal = sampleNormal(uv);

	
	vec2 uvMedium = teposition.xz/100.0;
	float rockDisp = 2.0*texture(texRockDisp, uvMedium).r - 1.0;
	float threshold = 0.9;
	float hinterval = 0.1;
	float weight = smoothstep( threshold + hinterval, threshold - hinterval, tenormal.y);
	//teposition += 3.0 * tenormal * rockDisp * weight;

	gl_Position = proj_view * vec4(teposition, 1.0);
}
