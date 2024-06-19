#version 430


layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
    
    
out VertexData {
	vec3 position_world;
	vec3 normal_world;
	vec2 uv;
	
} vert;


	
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat3 normalMatrix;
uniform float u_time;
uniform float freq = 0.5;
uniform float amp = 0.3;





	/* --------------------------------------------- */
	//  we used perlin noise instead of sin and cos 
	/* --------------------------------------------- */


	// Description : Array and textureless GLSL 2D/3D/4D simplex
	//               noise functions.
	//      Author : Ian McEwan, Ashima Arts.
	//  Maintainer : ijm
	//     Lastmod : 20110822 (ijm)
	//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
	//               Distributed under the MIT License. See LICENSE file.
	//               https://github.com/ashima/webgl-noise
	//

	vec3 mod289(vec3 x) 
	{
		return x - floor(x * (1.0 / 289.0)) * 289.0;
	}
	
	vec4 mod289(vec4 x) 
	{
		return x - floor(x * (1.0 / 289.0)) * 289.0;
	}
	
	vec4 permute(vec4 x) 
	{
	     return mod289(((x*34.0)+1.0)*x);
	}
	
	vec4 taylorInvSqrt(vec4 r)
	{
		return 1.79284291400159 - 0.85373472095314 * r;
	}
	
	float noise(vec3 v)
	{
		const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
		const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

		//First corner
		vec3 i  = floor(v + dot(v, C.yyy) );
		vec3 x0 =   v - i + dot(i, C.xxx) ;

		//Other corners
		vec3 g = step(x0.yzx, x0.xyz);
		vec3 l = 1.0 - g;
		vec3 i1 = min( g.xyz, l.zxy );
		vec3 i2 = max( g.xyz, l.zxy );

		//   x0 = x0 - 0.0 + 0.0 * C.xxx;
		//   x1 = x0 - i1  + 1.0 * C.xxx;
		//   x2 = x0 - i2  + 2.0 * C.xxx;
		//   x3 = x0 - 1.0 + 3.0 * C.xxx;
		vec3 x1 = x0 - i1 + C.xxx;
		vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
		vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

		//Permutations
		i = mod289(i);
		vec4 p = permute( permute( permute(
		           i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
		         + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
		         + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

		//The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
		//Gradients: 7x7 points over a square, mapped onto an octahedron.
		float n_ = 0.142857142857; // 1.0/7.0
		vec3  ns = n_ * D.wyz - D.xzx;

		vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

		vec4 x_ = floor(j * ns.z);
		vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

		vec4 x = x_ *ns.x + ns.yyyy;
		vec4 y = y_ *ns.x + ns.yyyy;
		vec4 h = 1.0 - abs(x) - abs(y);

		vec4 b0 = vec4( x.xy, y.xy );
		vec4 b1 = vec4( x.zw, y.zw );

		//vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
		//vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
		vec4 s0 = floor(b0)*2.0 + 1.0;
		vec4 s1 = floor(b1)*2.0 + 1.0;
		vec4 sh = -step(h, vec4(0.0));

		vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
		vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

		vec3 p0 = vec3(a0.xy,h.x);
		vec3 p1 = vec3(a0.zw,h.y);
		vec3 p2 = vec3(a1.xy,h.z);
		vec3 p3 = vec3(a1.zw,h.w);

		//Normalise gradients
		vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
		p0 *= norm.x;
		p1 *= norm.y;
		p2 *= norm.z;
		p3 *= norm.w;

		//Mix final noise value
		vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
		m = m * m;
		return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
		                              dot(p2,x2), dot(p3,x3) ) );
	}

	/* --------------------------------------------- */
	//end of github code
	/* --------------------------------------------- */



	//http://lolengine.net/blog/2013/09/21/picking-orthogonal-vector-combing-coconuts 
	//This always works if the input is a non-zero vector
	vec3 orthogonal(vec3 v) {
        return normalize(abs(v.x) > abs(v.z) ? vec3(-v.y, v.x, 0.0)
        : vec3(0.0, -v.z, v.y));
    }

	//This was our original displacement function based on sine and cosine
	float displacement(vec3 position) {
		float s_contrib = 0.5f*sin(position.x*2.5f + 3.0f*u_time);
		float t_contrib = 0.3f*cos(position.y*1.1f + 2.0f*u_time);

		return s_contrib * t_contrib;
	}

	//New displacement function based on a noise function
	float displaceNoise(vec3 position)
	{
		return noise(vec3(position.x * freq, position.z * freq, u_time*0.5)) * amp;
	}

	void main()
	{
		//vec3 dispPosition = vec3(position.x, displacement(position), position.z);
		vec3 dispPosition = position + normal * displaceNoise(position);

		//calculate and displace neighbours 
		vec3 tangent = orthogonal(normal);
		vec3 bitangent = normalize(cross(normal, tangent));
		vec3 neighbour1 = position + tangent * 0.01;
		vec3 neighbour2 = position + bitangent * 0.01;
		vec3 dispN1 = vec3(neighbour1.x, displaceNoise(neighbour1), neighbour1.z);
		vec3 dispN2 = vec3(neighbour2.x, displaceNoise(neighbour2), neighbour2.z);

		//recalculate tangent and bitangent after displacement
		vec3 dispTangent = dispN1 - dispPosition;
		vec3 dispBitangent = dispN2 - dispPosition;
		vec3 displacedNormal = normalize(cross(dispTangent, dispBitangent));
		
		vert.normal_world = normalMatrix * displacedNormal ;
		vert.uv = uv;
		vec4 position_world_ = modelMatrix * vec4(dispPosition, 1.0);
		vert.position_world = position_world_.xyz;
		gl_Position = projMatrix * viewMatrix * position_world_;
    }
	