#version 330 core
uniform vec3 uLightPos;
uniform vec3 uCameraPos;
uniform sampler2D uSampler;

in vec3 vNormal;
in vec2 vTexCoord;
in vec4 FragPos;

vec4 pack (float depth) {
    // 使用rgba 4字节共32位来存储z值,1个字节精度为1/256
    const vec4 bitShift = vec4(1.0, 256.0, 256.0 * 256.0, 256.0 * 256.0 * 256.0);
    const vec4 bitMask = vec4(1.0/256.0, 1.0/256.0, 1.0/256.0, 0.0);
    // gl_FragCoord:片元的坐标,fract():返回数值的小数部分
    vec4 rgbaDepth = fract(depth * bitShift); //计算每个点的z值
    rgbaDepth -= rgbaDepth.gbaa * bitMask; // Cut off the value which do not fit in 8 bits
    return rgbaDepth;
}

void main(){
  //vec3 color = texture2D(uSampler, vTextureCoord).rgb;
  //gl_FragColor = vec4( gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);
  gl_FragColor = pack(gl_FragCoord.z);
}