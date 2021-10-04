#version 330 core

layout (location = 0) out vec4  OutFilteredIllumination;

in vec2 TexCoords;

uniform sampler2D   gAlbedo;
uniform sampler2D   gHistoryLength;
uniform sampler2D   gIllumination;
uniform sampler2D   gLinearZAndNormal;

uniform int         gStepSize
uniform float       gPhiColor;
uniform float       gPhiNormal;

/** Helper function to reflect the folds of the lower hemisphere
    over the diagonals in the octahedral map.
*/
vec2 oct_wrap(vec2 v)
{
    return (1.f - abs(v.yx)) * (v.xy >= 0.f ? 1.f : -1.f);
}
/** Converts normalized direction to the octahedral map (non-equal area, signed normalized).
    \param[in] n Normalized direction.
    \return Position in octahedral map in [-1,1] for each component.
*/
vec2 ndir_to_oct_snorm(float3 n)
{
    // Project the sphere onto the octahedron (|x|+|y|+|z| = 1) and then onto the xy-plane.
    vec2 p = n.xy * (1.f / (abs(n.x) + abs(n.y) + abs(n.z)));
    p = (n.z < 0.f) ? oct_wrap(p) : p;
    return p;
}
/** Converts point in the octahedral map to normalized direction (non-equal area, signed normalized).
    \param[in] p Position in octahedral map in [-1,1] for each component.
    \return Normalized direction.
*/
vec3 oct_to_ndir_snorm(vec2 p)
{
    vec3 n = vec3(p.xy, 1.0 - abs(p.x) - abs(p.y));
    n.xy = (n.z < 0.0) ? oct_wrap(n.xy) : n.xy;
    return normalize(n);
}
/** Returns a relative luminance of an input linear RGB color in the ITU-R BT.709 color space
    \param RGBColor linear HDR RGB color in the ITU-R BT.709 color space
*/
float luminance(vec3 rgb)
{
    return dot(rgb, vec3(0.2126f, 0.7152f, 0.0722f));
}

vec3 demodulate(vec3 c, vec3 albedo)
{
    return c / max(albedo, vec3(0.001, 0.001, 0.001));
}

float computeWeight(
    float depthCenter, float depthP, float phiDepth,
    vec3 normalCenter, vec3 normalP, float phiNormal,
    float luminanceIllumCenter, float luminanceIllumP, float phiIllum)
{
    const float weightNormal  = pow(clamp(dot(normalCenter, normalP), 0.0, 1.0), phiNormal);
    const float weightZ       = (phiDepth == 0) ? 0.0f : abs(depthCenter - depthP) / phiDepth;
    const float weightLillum  = abs(luminanceIllumCenter - luminanceIllumP) / phiIllum;

    const float weightIllum   = exp(0.0 - max(weightLillum, 0.0) - max(weightZ, 0.0)) * weightNormal;

    return weightIllum;
}

// computes a 3x3 gaussian blur of the variance, centered around
// the current pixel
float computeVarianceCenter(vec2 texCoords)
{
    ivec2 imageDim = textureSize(gIllumination,0);
    float texelSizeX = 1.0 / float(imageDim.x);
    float texelSizeY = 1.0 / float(imageDim.y);

    float sum = 0.f;

    const float kernel[2][2] = {
        { 1.0 / 4.0, 1.0 / 8.0  },
        { 1.0 / 8.0, 1.0 / 16.0 }
    };

    const int radius = 1;
    for (int yy = -radius; yy <= radius; yy++)
    {
        for (int xx = -radius; xx <= radius; xx++)
        {
            const vec2 p = texCoords + vec2(xx * texelSizeX, yy * texelSizeY);
            const float k = kernel[abs(xx)][abs(yy)];
            sum += texture(gIllumination, p).a * k;
        }
    }

    return sum;
}

void main(){
    ivec2 imageDim = textureSize(gAlbedo,0);
    float texelSizeX = 1.0 / float(imageDim.x);
    float texelSizeY = 1.0 / float(imageDim.y);
    vec2  texelSize = vec2(texelSizeX,texelSizeY);

    const float epsVariance      = 1e-10;
    const float kernelWeights[3] = { 1.0, 2.0 / 3.0, 1.0 / 6.0 };

    // constant samplers to prevent the compiler from generating code which
    // fetches the sampler descriptor from memory for each texture access
    // what the fuck?
    const vec4 illuminationCenter = texture(gIllumination,TexCoords);
    const float lIlluminationCenter = luminance(illuminationCenter.rgb);

    // variance, filtered using 3x3 gaussin blur
    const float var = computeVarianceCenter(TexCoords);

    // number of temporally integrated pixels
    const float historyLength = texture(gHistoryLength,TexCoords).x;

    const vec2 zCenter = texture(gLinearZAndNormal,TexCoords).xy;
    if (zCenter.x < 0)
    {
        // not a valid depth => must be envmap => do not filter
        return illuminationCenter;
    }
    const vec3 nCenter = oct_to_ndir_snorm(texture(gLinearZAndNormal,TexCoords).zw);

    const float phiLIllumination   = gPhiColor * sqrt(max(0.0, epsVariance + var.r));
    const float phiDepth     = max(zCenter.y, 1e-8) * gStepSize;

    // explicitly store/accumulate center pixel with weight 1 to prevent issues
    // with the edge-stopping functions
    float sumWIllumination   = 1.0;
    vec4  sumIllumination  = illuminationCenter;

    for (int yy = -2; yy <= 2; yy++)
    {
        for (int xx = -2; xx <= 2; xx++)
        {
            const vec2 p      = TexCoords + vec2(xx, yy) * gStepSize * texelSize;
            const bool inside = all(p >= vec2(0.0,0.0)) && all(p < vec2(1.0,1.0));

            const float kernel = kernelWeights[abs(xx)] * kernelWeights[abs(yy)];

            if (inside && (xx != 0 || yy != 0)) // skip center pixel, it is already accumulated
            {
                const vec4 illuminationP = texture(gIllumination,p);
                const float lIlluminationP = luminance(illuminationP.rgb);
                const float zP = texture(gLinearZAndNormal,p).x;
                const vec3 nP = oct_to_ndir_snorm(texture(gLinearZAndNormal,p).zw);

                // compute the edge-stopping functions
                const vec2 w = computeWeight(
                    zCenter.x, zP, phiDepth * length(float2(xx, yy)),
                    nCenter, nP, gPhiNormal,
                    lIlluminationCenter, lIlluminationP, phiLIllumination);

                const float wIllumination = w.x * kernel;

                // alpha channel contains the variance, therefore the weights need to be squared, see paper for the formula
                sumWIllumination  += wIllumination;
                sumIllumination   += vec4(wIllumination.xxx, wIllumination * wIllumination) * illuminationP;
            }
        }
    }

    // renormalization is different for variance, check paper for the formula
    vec4 filteredIllumination = vec4(sumIllumination / vec4(sumWIllumination.xxx, sumWIllumination * sumWIllumination));

    OutFilteredIllumination = filteredIllumination;
}