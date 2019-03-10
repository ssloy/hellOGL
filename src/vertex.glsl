#version 330 core

// Input vertex data, different for all executions of this shader
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 vertexUV;

// Output data; will be interpolated for each fragment
out vec2 UV;
out vec3 Normal_cameraspace;
out vec3 LightDirection_cameraspace;
out vec3 tangent_cameraspace;
out vec3 bitangent_cameraspace;
out vec3 EyeDirection_cameraspace;

// Values that stay constant for the entire mesh
uniform vec3 LightPosition_worldspace;
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;

void main() {
    gl_Position = MVP * vec4(vertexPosition_modelspace, 1);              // Output position of the vertex, in clip space : MVP * position
    UV = vertexUV;  // UV of the vertex. No special space for this one.

    Normal_cameraspace = (transpose(inverse(V*M)) * vec4(vertexNormal_modelspace, 0)).xyz;  // Normal of the the vertex, in camera space
    EyeDirection_cameraspace = vec3(0,0,1);  // Vector that goes from the vertex to the camera, in camera space.

    vec3 vertexPosition_cameraspace = (V*M*vec4(vertexPosition_modelspace,1)).xyz;
    vec3 LightPosition_cameraspace  = (V*  vec4(LightPosition_worldspace, 1)).xyz;   // M is ommited because it's identity.
    LightDirection_cameraspace = LightPosition_cameraspace - vertexPosition_cameraspace;

    tangent_cameraspace   = (V*vec4(  tangent, 0)).xyz;
    bitangent_cameraspace = (V*vec4(bitangent, 0)).xyz;
}

