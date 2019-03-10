#version 330 core

// Output data
out vec3 color;

// Interpolated values from the vertex shaders
in vec3 Normal_cameraspace;
in vec3 LightDirection_cameraspace;

void main() {
    vec3 n = normalize(Normal_cameraspace);  // Normal of the computed fragment, in camera space
    vec3 l = normalize(LightDirection_cameraspace); // Direction of the light (from the fragment to the light)
    float cosTheta = clamp(dot(n,l), 0, 1);         // Cosine of the angle between the normal and the light direction, 

    color = vec3(1,0,0)*(0.1 +           //  ambient lighting
                         1.3*cosTheta);  //  diffuse lighting
}

