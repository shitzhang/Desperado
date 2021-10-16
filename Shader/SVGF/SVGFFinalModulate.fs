#version 460 core

layout (location = 0) out vec4  OutPixelColor;

in vec2 TexCoords;

uniform sampler2D   gAlbedo;
uniform sampler2D   gEmission;
uniform sampler2D   gIllumination;


void main(){
    vec4 albedo = vec4(texture(gAlbedo,TexCoords).rgb, 1.0);
    vec4 illumination = texture(gIllumination,TexCoords);
    vec4 emission = vec4(texture(gEmission,TexCoords).rgb, 1.0);
    OutPixelColor = albedo * illumination + emission;
}