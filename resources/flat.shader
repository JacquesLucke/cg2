// Vertex Shader
#version 330 core

in vec3 position;
uniform mat4 u_Model;
uniform mat4 u_ViewProjection;

void main() {
    gl_Position = u_ViewProjection * u_Model * vec4(position, 1.0);
}



// Fragment Shader
#version 330 core

out vec4 color;
uniform vec4 u_Color;

void main() {
    color = u_Color;
}
