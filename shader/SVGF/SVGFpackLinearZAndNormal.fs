#version 460 core

layout (location = 0) out vec4  OutLinearZAndNormal;

in vec2 TexCoords;

uniform sampler2D   gLinearZ;
uniform sampler2D   gNormal;

/** Helper function to reflect the folds of the lower hemisphere
    over the diagonals in the octahedral map.
*/
vec2 oct_wrap(vec2 v)
{
    vec2 weight = vec2((v.x >= 0.f ? 1.f : -1.f) , (v.y >= 0.f ? 1.f : -1.f));
    return (vec2(1.f , 1.f) - abs(v.yx)) * weight;
}
/** Converts normalized direction to the octahedral map (non-equal area, signed normalized).
    \param[in] n Normalized direction.
    \return Position in octahedral map in [-1,1] for each component.
*/
vec2 ndir_to_oct_snorm(vec3 n)
{
    // Project the sphere onto the octahedron (|x|+|y|+|z| = 1) and then onto the xy-plane.
    vec2 p = n.xy * (1.f / (abs(n.x) + abs(n.y) + abs(n.z)));
    p = (n.z < 0.f) ? oct_wrap(p) : p;
    return p;
}

void main(){
    //vec4 fragCoord = vsOut.posH;
    //const int2 ipos = int2(fragCoord.xy);
    vec2 nPacked = ndir_to_oct_snorm(texture(gNormal,TexCoords).xyz);
    OutLinearZAndNormal = vec4(texture(gLinearZ,TexCoords).xy, nPacked.x, nPacked.y);
}