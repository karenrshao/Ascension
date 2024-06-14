#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform vec3 fcolor;

in vec2 texcoord;

layout(location = 0) out vec4 color;

vec2 distort(vec2 uv)	
{
	float new_y = uv.y;

	float new_x = sin((time+(uv.y*2*3.1415926))*0.6)*(0.08)*(1.0-uv.y) + uv.x;

	if(new_y < 0.0){
		new_y = 0.0;
	}

	if(new_y > 1.0){
		new_y = 1.0;
	}

	if(new_x < 0.0){
		new_x = 0.0;
	}

	if(new_x > 1.0){
		new_x = 1.0;
	}

	return vec2(new_x, new_y);
}

void main()
{
	vec2 coord = distort(texcoord);

    color = vec4(fcolor, 1.0) * texture(screen_texture, coord);
}