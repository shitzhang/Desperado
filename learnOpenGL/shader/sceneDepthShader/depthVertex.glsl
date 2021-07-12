#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

uniform mat4 uLightVP;

out vec3 vNormal;
out vec2 vTexCoord;
out vec4 FragPos;

void main() {

  vNormal = (uModelMatrix * vec4(aNormal,0.0)).xyz;
  vTexCoord = aTexCoord;
  gl_Position = uLightVP * uModelMatrix * vec4(aPos, 1.0);
  FragPos = uLightVP * uModelMatrix * vec4(aPos, 1.0);
}