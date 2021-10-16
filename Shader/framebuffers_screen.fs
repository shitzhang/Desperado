#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    //float col = texture(screenTexture, TexCoords).r;
    //vec2 col = texture(screenTexture, TexCoords).rg;
    //vec3 col = texture(screenTexture, TexCoords).rgb;
    vec4 col = texture(screenTexture, TexCoords).rgba;
    //FragColor = vec4(col.r/32.0, col.r/32.0, col.r/32.0, 1.0);
    //FragColor = vec4(col.r, col.g, 0.0, 1.0);
    //FragColor = vec4(col.r, col.g, col.b, 1.0);
    FragColor = vec4(col.r, col.g, col.b, col.a);
}