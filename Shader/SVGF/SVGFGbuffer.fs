#version 460 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec2 gLinearZ;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec2 gPositionNormalFwidth;
layout (location = 4) out vec4 gAlbedo;
layout (location = 5) out vec2 gMotion;
//layout (location = 6) out uint gMeshId;
layout (location = 7) out vec3 gEmission;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform mat4 pre_view;
uniform mat4 pre_projection;

uniform mat4 view;
uniform mat4 projection;

uniform sampler2D diffuse_map1;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * 100.0 * 5000.0) / (5000.0 + 100.0 - z * (5000.0 - 100.0));    
}

/** Calculate screen-space motion vector.
    \param[in] pixelCrd Sample in current frame expressed in pixel coordinates with origin in the top-left corner.
    \param[in] prevPosH Sample in previous frame expressed in homogeneous clip space coordinates. Note that the definition differs between D3D12 and Vulkan.
    \param[in] renderTargetDim Render target dimension in pixels.
    \return Motion vector pointing from current to previous position expressed in sceen space [0,1] with origin in the top-left corner.
*/
// vec2 calcMotionVector(vec2 pixelCrd, vec4 prevPosH, vec2 renderTargetDim)
// {
//     vec2 prevCrd = prevPosH.xy / prevPosH.w;
// #ifdef FALCOR_VK
//     prevCrd *= vec2(0.5, 0.5);
// #else
//     prevCrd *= vec2(0.5, -0.5);
// #endif
//     prevCrd += 0.5f;
//     vec2 normalizedCrd = pixelCrd / renderTargetDim;
//     return prevCrd - normalizedCrd;
// }

vec2 calcMotionVector()
{
    //may be need to optimize
    vec4 screen_pos = projection * view * vec4(FragPos, 1.0);
    screen_pos /= screen_pos.w;
    vec4 pre_screen_pos = pre_projection * pre_view * vec4(FragPos, 1.0);
    pre_screen_pos /= pre_screen_pos.w;

    vec2 uv_pos = (screen_pos.xy + vec2(1.0, 1.0)) * 0.5;
    vec2 uv_pre_pos = (pre_screen_pos.xy + vec2(1.0, 1.0)) * 0.5;
   
    vec2 uv_delta = uv_pre_pos - uv_pos;
    //vec2 uv_delta = uv_pos - uv_pre_pos;
    return uv_delta;
}

void main()
{   
    //discard transparent object
    vec4 color = texture(diffuse_map1, TexCoords);
    if(color.a < 0.1)
        discard;
    gPosition = vec4(FragPos, 1.0);
    gLinearZ = vec2(gl_FragCoord.z , fwidth(gl_FragCoord.z));
    gNormal = vec4(normalize(Normal), 1.0);
    gPositionNormalFwidth = vec2(length(fwidth(FragPos)), length(fwidth(Normal)));
    gAlbedo = vec4(color.rgb, 1.0);
    gMotion = calcMotionVector();
    gEmission = vec3(0.0);
}  