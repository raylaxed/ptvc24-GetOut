#version 430

in VertexData {
    vec3 position_world;
    vec3 normal_world;
    vec2 uv;
} vert;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 brightColor;

// Uniforms for lighting
uniform vec3 lightPosition = vec3(10.0, 10.0, 10.0);
uniform vec3 lightColor = vec3(1.0, 1.0, 1.0);
uniform vec3 ambientColor = vec3(0.2, 0.2, 0.2);

// Material properties
uniform vec3 diffuseColor = vec3(0.8, 0.8, 0.8);
uniform vec3 specularColor = vec3(1.0, 1.0, 1.0);
uniform float shininess = 32.0;

void main() {
    // Normalize the interpolated normal
    vec3 normal = normalize(vert.normal_world);
    
    // Compute the vector from the fragment to the light source
    vec3 lightDir = normalize(lightPosition - vert.position_world);
    
    // Compute the ambient component
    vec3 ambient = ambientColor * diffuseColor;
    
    // Compute the diffuse component
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * diffuseColor;
    
    // Compute the view direction (assuming the camera is at the origin)
    vec3 viewDir = normalize(-vert.position_world);
    
    // Compute the reflect direction for specular component
    vec3 reflectDir = reflect(-lightDir, normal);
    
    // Compute the specular component
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * lightColor * specularColor;
    
    // Combine all components
    vec3 color = ambient + diffuse + specular;
    
    // Output the final color
    color = vec4(color, 1.0);	

	if (mode) {
        color = vec4(color , 1.0);
    } else {
        color = vec4(normal, 1.0);
    }

	float brightness = dot(color.rgb, vec3(0.92, 0.42, 0.14));
    if (brightness > 1.0) {
        brightColor = vec4(color.rgb, 1.0);
    } else {
        brightColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
