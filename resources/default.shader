// Vertex Shader
#version 330 core

in vec3 position;

uniform mat4 u_MVP;

void main() {
    gl_Position = u_MVP * vec4(position, 1.0);
}



// Fragment Shader
#version 330 core

uniform vec4 u_Color;
out vec4 color;

void main() {
    color = u_Color;
}
