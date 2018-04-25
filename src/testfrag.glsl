#version 440 core

out vec4 out_color;

in vec2 getex;
in vec3 geposition;
in vec3 genormal;

uniform sampler2D heightmap;

uniform vec3 size;

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

float rand(float n) { 
	return fract(sin(n*4.1414) * 43758.5453);
}
float rand2(float n) { 
	return 2.0*fract(sin(n*13.5135) * 28968.2317) - 1.0;
}


float ambientOcclusion(vec2 uv, vec3 normal) {
	
	vec3 tangent = normalize(cross(normal, vec3(1,0,0)));
	vec3 bitangent = cross(normal, tangent);

	float height = texture(heightmap, uv).r;

	float occlusion = 0.f;

	float seed = 0;// 10*rand(dot(uv, uv));

	const float samples = 100;
	for(float i = 0; i < samples; i++) {
		vec3 hemi;
		hemi += rand2(i + seed) * tangent;
		hemi += rand2(i+10 + seed) * bitangent;
		hemi += rand(i + seed) * normal;
		hemi = 30 * rand(i + 100+ seed) * normalize(hemi);


		vec2 coord = uv + hemi.xz / size.xz;

		float currHeight = texture(heightmap, coord).r;

		if(height + hemi.y/size.y < currHeight) {
			occlusion += 1.0/samples;
		}
	}

	return sqrt(occlusion);
}


vec3 rgb(float r, float g, float b) {
	return vec3(r, g, b) / 255.0;
}
const vec3 colorSnow = vec3(1);
const vec3 colorGrass = rgb(151, 169, 79);
const vec3 colorDarkGrass = 0.6*rgb(151, 169, 79);
const vec3 colorSand = rgb(194, 178, 128);
const vec3 colorRock = rgb(81,79,86);
const vec3 colorWater = rgb(30, 90, 190);

vec3 chooseMat(vec3 pos, vec3 normal) {
	
	vec3 result = colorGrass;
	float leaning = dot(normal, vec3(0,1,0));

	float snowHeight = size.y*0.8;

	// leaning grass
	float threshold = 0.9;
	float hinterval = 0.005;
	result = mix(result, colorDarkGrass, smoothstep( threshold + hinterval, threshold - hinterval, leaning));

	// snow
	if(pos.y > snowHeight) {
		result = colorSnow;
	}
	
	// rock wall
	threshold = 0.5;
	hinterval = 0.005;
	result = mix(result, colorRock, smoothstep(threshold + hinterval, threshold - hinterval, leaning));


	return result;
}

void main() {
	vec3 normal = sampleNormal(getex);
	

	float ao = 1;//(1.0 - ambientOcclusion(getex, normal));


	float diffuse = 0.8*max(dot(normal, normalize(vec3(-1,1,1))), 0) * ao;
	float ambient = 0.2 * ao;

	vec3 material = chooseMat(geposition, normal);
	
	vec3 color;
	color += material * diffuse;
	color += material * ambient;


	out_color = vec4(color, 1.0);
}