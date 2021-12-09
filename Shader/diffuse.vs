#version 330 core
layout(location = 0) in vec3 aPos;   // 位置变量的属性位置值为 0 
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;
out vec2 TexCoords;
out vec3 FragPos;
//out vec4 nimasile;

void main()
{
    gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
}