// Vertex Shader
#version 330 core

layout(location = 0) in vec4 position;
uniform mat4 u_MVP;

void main() {
    gl_Position = u_MVP * position;
}



// Fragment Shader
#version 330 core

uniform vec4 u_Color;
out vec4 color;

void main() {
    color = u_Color;
}