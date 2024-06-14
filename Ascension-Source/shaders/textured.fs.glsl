#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform int flash;
uniform int initialHit;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	vec4 originalColor = texture(sampler0, vec2(texcoord.x, texcoord.y));

	if(flash == 1 && originalColor.z != 0.0){
		color = originalColor - vec4(0.2, 0.2, 0.2, 0.3);
	}else{
		color = originalColor;
	}

	if(initialHit == 1 && originalColor.z != 0.0){
		color = vec4(1.0, 1.0, 1.0, 1.0);
	}

	color = vec4(fcolor, 1.0) * color;
}
