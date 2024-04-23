#version 430 core

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 brightColor;


#define PI 3.14159265

in VertexData {
	vec3 position_world;
	vec3 normal_world;
	vec2 uv;
} vert;


uniform vec3 camera_world;

uniform vec3 materialCoefficients; // x = ambient, y = diffuse, z = specular 
uniform float specularAlpha;
uniform sampler2D diffuseTexture;



//F0 is the fresnel factor we set it at 0.8 as our default value 
uniform float roughness = 0.8;
uniform float metallic = 0.9;
uniform float ao = 0.05;



uniform struct PointLight {
	vec3 color;
	vec3 position;
	vec3 attenuation;
} pointL;

#define NR_POINT_LIGHTS 5
uniform PointLight pointLights[NR_POINT_LIGHTS];




vec3 fresnelSchlick(float cosT, vec3 F0New){


	return F0New + (1.0 - F0New) * pow(clamp(1.0 - cosT, 0.0, 1.0), 5.0);

}


float beckmannDist(float roughness, float NdotH){

	float rough2 = roughness * roughness;
	float rough4 = rough2* rough2;
	//float r1 = 1.0 / (4.0 * rough2 * pow(NdotH, 4.0));
	//float r2 = (NdotH * NdotH - 1.0) / (rough2 * NdotH * NdotH);
	//other distributíon
	float NdotH2 = NdotH * NdotH;
	float num = rough4;
	float den = NdotH2 * (rough4-1.0) + 1.0;
	return num/(PI*den*den);

	//return r1 * exp(r2);


}



float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float rough2 = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - rough2) + rough2;
	
    return num / denom;
}



void main()
{
	vec3 texColor = texture(diffuseTexture, vert.uv).rgb;
	

	vec3 albedo = texColor;
	vec3 N = normalize(vert.normal_world);
	vec3 viewDir = normalize(camera_world-vert.position_world);

	vec3 F0New = vec3(0.04f);
	//albedo may be texColor
	F0New = mix(F0New,albedo,metallic);

	vec3 Lo = vec3(0.0);
	for(int i = 0; i < NR_POINT_LIGHTS; i++)
	{
		vec3 lightDir = normalize(pointLights[i].position - vert.position_world);
		vec3 halfVector = normalize(viewDir + lightDir);
		float dist = length(pointLights[i].position - vert.position_world);
		float att = 1.0/(pointLights[i].attenuation.x + dist * pointLights[i].attenuation.y + dist*dist*pointLights[i].attenuation.z );
		vec3 radiance = pointLights[i].color * att;

		float NdotH = max(0, dot(vert.normal_world, halfVector));
		float NdotV = max(0, dot(vert.normal_world, viewDir));
		float NdotL = max(0, dot(vert.normal_world, lightDir));
		float HdotV = max(0, dot(halfVector, viewDir));

		float ggx1 = GeometrySchlickGGX(NdotV,roughness); 
		float ggx2 = GeometrySchlickGGX(NdotL,roughness);

		float NDF = beckmannDist(roughness,NdotH);
		float G = ggx1*ggx2;
		vec3 F = fresnelSchlick(HdotV,F0New);
		
		vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	

		vec3 num = NDF * G * F;
		float denum = 4.0 * NdotV * NdotL + 0.0001;
		vec3 spec = num/denum;

		Lo += (kD * albedo / PI + spec) * radiance * NdotL;
	
	}

	vec3 ambient = vec3(0.03) * albedo * ao;


	vec3 vcolor = ambient + Lo;

	//vcolor = vcolor/ (vcolor + vec3(1.0));
	//vcolor = pow(vcolor, vec3(1.0/2.2));
	color = vec4(vcolor,1.0);

	float brightness = dot(color.rgb,   vec3(0.92,0.42,0.14));
    if(brightness > 1.0)
	{
        brightColor = vec4(color.rgb, 1.0);
	}else{
		brightColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}

