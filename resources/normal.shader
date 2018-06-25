// Vertex Shader
#version 330 core

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;
in vec3 position;
in vec3 normal;
out vec3 v_Position;
smooth out vec3 v_Normal;

void main() {
    gl_Position = u_ViewProjection * u_Model * vec4(position, 1.0);
    v_Position = vec3(u_Model * vec4(position, 1.0));
    v_Normal = vec3(u_Model * vec4(normal, 0.0));
}



// Fragment Shader
#version 330 core

uniform float u_Brightness;
in vec3 v_Position;
smooth in vec3 v_Normal;
layout(location = 0) out vec4 color;

void main() {
    color = vec4((v_Normal / 2.0 + 0.5) * u_Brightness, 1.0);
}
