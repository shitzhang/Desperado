#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;
in vec2 TexCoords;

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_height1;


void main()
{             
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    vec3 refL=texture(skybox, R).rgb;
    vec3 refCol=refL*texture(texture_height1,TexCoords).rgb;
    vec3 diff=texture(texture_diffuse1,TexCoords).rgb;
    vec3 result=refCol+diff;
    FragColor = vec4(result, 1.0);  
}