// Vertex Shader
#version 330 core

attribute vec3 position;
attribute vec3 normal;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;

out vec3 v_Normal;
out vec3 v_Position;

void main() {
     v_Normal = normal;
     v_Position = position;
     gl_Position = u_ViewProjection * u_Model * vec4(position, 1.0);
}

// Fragment Shader
#version 330 core


uniform float magnitude;

uniform vec3 specularColor;
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
     
uniform vec3 lightPosition;

uniform vec3 cameraPosition;

const float blinnPhongExp = 16.0;

in vec3 v_Normal;
in vec3 v_Position;

out vec4 color;

void main() {

    vec3 normal = normalize(v_Normal);
    vec3 lightDir = lightPosition - v_Position;
    float distance = length(lightDir);

    lightDir = normalize(lightDir);

    float lambert = dot(lightDir, normal);
    float specular = 0.0;

    if(lambert > 0.0)
    {
        vec3 viewDir = normalize(cameraPosition - v_Position);
	vec3 halfVec = normalize(lightDir + viewDir);
	specular = pow(max(dot(halfVec, normal), 0.0), blinnPhongExp);
    }

    color = vec4(ambientColor +
    		 diffuseColor * magnitude * lambert / distance +
		 specularColor * magnitude * specular / distance, 1.0);
}
