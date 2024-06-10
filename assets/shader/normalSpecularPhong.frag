#version 430 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D normalMap;
uniform sampler2D specularMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform vec4 materialCoefficients;

uniform struct PointLight {
	vec3 color;
	vec3 position;
	vec3 attenuation;
} pointL;

#define nrOfPointLights 5
uniform PointL pointLights[nrOfPointLights];

// Computes the reflection direction for an incident vector I about normal N,
// and clamps the reflection to a maximum of 180, i.e. the reflection vector
// will always lie within the hemisphere around normal N.
// Aside from clamping, this function produces the same result as GLSL's reflect function.
vec3 clampedReflect(vec3 I, vec3 N)
{
	return I - 2.0 * min(dot(N, I), 0.0) * N;
}

vec3 fresnelSchlick (float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow (1.0 - cosTheta, 5.0);
}

vec3 phong(vec3 n, vec3 l, vec3 v, vec3 diffuseC, float diffuseF, vec3 specularC, float specularF, float alpha, bool attenuate, vec3 attenuation)
{
	float d = length(l);
	l = normalize(l);
	float att = 1.0;
	if(attenuate) att = 1.0f / (attenuation.x + d * attenuation.y + d * d * attenuation.z);
	vec3 r = reflect(-l, n);
    vec3 rougness = vec3(1.0) - texture(specularMap, fs_in.TexCoords).rgb;
    //vec3 specular = spec * rougness;
	return (diffuseF * diffuseC * max(0, dot(n, l)) + specularF * specularC * pow(max(0, dot(r, v)), alpha)) * att;
}

void main()
{           
    vec3 n = normalize(texture(normalMap, fs_in.TexCoords)).rgb;
    vec3 v = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 R = normalize(clampedReflect(v, n));
    vec3 F0 = vec3(0.1);
	vec3 reflectivity = fresnelSchlick(dot(n, -v), F0);
   
    vec3 diffuseColor = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 color = diffuseColor * materialCoefficients.x;

    vec3 roughness = vec3(1.0) - texture(specularMap, fs_in.TexCoords).rgb;
    float adjustedAlpha = materialCoefficients.w * (1.0 - roughness);

    for(int i = 0; i < pointLights.length(); i++) {
        // add point light contribution
	    color += phong(n, pointL.position - fs_in.TangentFragPos, -v, pointL.color * diffuseColor, materialCoefficients.y, pointL.color, materialCoefficients.z, materialCoefficients.w, true, pointL.attenuation);
    }

    FragColor = vec4(color, 1.0);
}