#version 430 core
layout (location = 0) out vec4 color;
layout (location = 1) out vec4 brightColor;

in VertexData {
	vec3 position_world;
	vec3 normal_world;
	vec2 uv;
} vert;


uniform vec3 lightColor;

void main()
{           
    color = vec4(lightColor, 1.0);
	float brightness = dot(color.rgb,  vec3(0.92,0.42,0.14));
    if(brightness > 1.0)
	{
        brightColor = vec4(color.rgb, 1.0);
	}else{
		brightColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}