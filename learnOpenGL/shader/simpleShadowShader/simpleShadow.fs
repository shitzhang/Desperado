#version 330 core
// Phong related variables
uniform sampler2D texture_diffuse1;
uniform vec3 uKd;
uniform vec3 uKs;
uniform vec3 uLightPos;
uniform vec3 uCameraPos;
uniform vec3 uLightRadiance;

in vec2 vTextureCoord;
in vec3 vFragPos;
in vec3 vNormal;

out vec4 FragColor;

// Shadow map related variables

uniform sampler2D uShadowMap;

in vec4 vPositionFromLight;
#define EPS 1e-3

float unpack(vec4 rgbaDepth) {
  const vec4 bitShift = vec4(1.0, 1.0 / 256.0, 1.0 / (256.0 * 256.0),
                             1.0 / (256.0 * 256.0 * 256.0));
  return dot(rgbaDepth, bitShift);
}

float useShadowMap(sampler2D shadowMap, vec4 shadowCoord) {
  vec4 rgbaDepth = texture2D(shadowMap, shadowCoord.xy);
  float depth = unpack(rgbaDepth);
  return (shadowCoord.z > depth) ? 0.0 : 1.0;
}

vec3 blinnPhong() {
  vec3 color = texture2D(texture_diffuse1, vTextureCoord).rgb;
  color = pow(color, vec3(2.2));

  vec3 ambient = 0.05 * color;

  vec3 lightDir = normalize(uLightPos);
  vec3 normal = normalize(vNormal);
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = diff * uLightRadiance * color;

  vec3 viewDir = normalize(uCameraPos - vFragPos);
  vec3 halfDir = normalize((lightDir + viewDir));
  float spec = pow(max(dot(halfDir, normal), 0.0), 32.0);
  vec3 specular = uKs * uLightRadiance * spec;

  vec3 radiance = (ambient + diffuse + specular);
  vec3 phongColor = pow(radiance, vec3(1.0 / 2.2));
  return phongColor;
}

void main(void) {

  vec3 shadowCoord =
      (vPositionFromLight.xyz / vPositionFromLight.w) / 2.0 + 0.5;

  float visibility = 1.0;
  visibility = useShadowMap(uShadowMap, vec4(shadowCoord, 1.0));
  // visibility = PCF(uShadowMap, vec4(shadowCoord, 1.0));
  //visibility = PCSS(uShadowMap, vec4(shadowCoord, 1.0));

  //vec3 phongColor = blinnPhong();
  vec3 color = texture2D(texture_diffuse1, vTextureCoord).rgb;
  

  FragColor = vec4(color * visibility, 1.0);
}
