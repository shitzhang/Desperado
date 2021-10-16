#version 330 core
out vec4 FragColor;

in vec2 TexCoords;


uniform sampler2D diffuse_map1;

void main()
{
    vec4 color = texture2D(diffuse_map1, TexCoords);
    //FragColor = texture2D(diffuse_map1, TexCoords); 
    if(color.a < 0.1)
        discard;
    FragColor = color; 
    
}