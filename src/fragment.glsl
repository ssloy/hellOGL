#version 330 core

// Output data
out vec3 color;

// Interpolated values from the vertex shaders
in vec3 Normal_cameraspace;
in vec3 LightDirection_cameraspace;
in vec3 tangent_cameraspace;
in vec3 bitangent_cameraspace;
in vec2 UV;

// Values that stay constant for the entire mesh
uniform sampler2D tangentnm;
uniform sampler2D diffuse;

void main() {
    vec3 n = normalize(Normal_cameraspace);  // Normal of the computed fragment, in camera space

    mat3 D = mat3(normalize(tangent_cameraspace), normalize(bitangent_cameraspace), n); // Darboux frame for tangent space normal mapping
    n = normalize((D*normalize(texture(tangentnm, UV).rgb * 2 - 1)));                   // tangent space normal mapping

    vec3 l = normalize(LightDirection_cameraspace); // Direction of the light (from the fragment to the light)
    float cosTheta = clamp(dot(n,l), 0, 1);         // Cosine of the angle between the normal and the light direction, 

    color = texture(diffuse, UV).xyz*(0.1 +           //  ambient lighting
                                      1.3*cosTheta);  //  diffuse lighting
}

