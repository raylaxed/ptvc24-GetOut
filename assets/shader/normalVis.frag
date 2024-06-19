#version 430
layout (location = 1) out vec4 brightColor;
in VertexData {
    vec3 normal_world;
    vec4 position_world;
} vert;

layout(location = 0) out vec4 color;

void main() {
    vec3 normal = normalize(vert.normal_world);
    color = vec4(normal * 0.5 + 0.5, 1.0); // Normalen von [-1, 1] auf [0, 1] skalieren
	float brightness = dot(color.rgb,   vec3(0.92,0.42,0.14));
    if(brightness > 1.0)
	{
        brightColor = vec4(color.rgb, 1.0);
	}else{
		brightColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}
