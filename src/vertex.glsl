#version 330 core

// Input vertex data, different for all executions of this shader
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;

// Output data; will be interpolated for each fragment
out vec3 Normal_cameraspace;
out vec3 LightDirection_cameraspace;

// Values that stay constant for the entire mesh
uniform vec3 LightPosition_worldspace;

void main() {
    gl_Position = vec4(vertexPosition_modelspace, 1);              // Output position of the vertex, in clip space
    Normal_cameraspace = vertexNormal_modelspace;                  // Normal of the the vertex, in camera space
    LightDirection_cameraspace = LightPosition_worldspace - vertexPosition_modelspace;
}

