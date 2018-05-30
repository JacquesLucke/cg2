// Vertex Shader
#version 330 core

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;
in vec3 position;
in vec4 color;
smooth out vec4 v_Color;

void main() {
    gl_Position = u_ViewProjection * u_Model * vec4(position, 1.0);
    v_Color = color;
}



// Fragment Shader
#version 330 core

smooth in vec4 v_Color;
layout(location = 0) out vec4 color;

void main() {
    color = v_Color;
}
