#version 330 core
layout(location=0) in vec3 aVertexPosition;
layout(location=1) in vec3 aNormalPosition;
layout(location=2) in vec2 aTextureCoord;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform mat4 uLightVP;

out vec2 vTextureCoord;
out vec3 vFragPos;
out vec3 vNormal;
out vec4 vPositionFromLight;

void main() {

  vFragPos = (uModelMatrix * vec4(aVertexPosition, 1.0)).xyz;
  vNormal = (uModelMatrix * vec4(aNormalPosition, 0.0)).xyz;

  gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aVertexPosition, 1.0);

  vTextureCoord = aTextureCoord;
  vPositionFromLight = uLightVP * uModelMatrix * vec4(aVertexPosition, 1.0);

  //gl_Position = vPositionFromLight;
}