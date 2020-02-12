#version 330 core
layout (location = 0) out vec4 gAlbedoSpec;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gPosition;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main()
{    
    // Store the fragment position vector in the first gbuffer texture
    //gPosition = FragPos;
    gPosition = vec3(0.1, 0.2, 0.3);
    // Also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);
    gNormal = vec3(0.4, 0.5, 0.6);
    // And the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;
    // Store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(texture_specular1, TexCoords).r;
    gAlbedoSpec = vec4(0.7, 0.8, 0.9, 1.0);
}