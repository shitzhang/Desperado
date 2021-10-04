#version 330 core

layout (location = 0) out vec4  OutIllumination;

in vec2 TexCoords;

uniform sampler2D   gIllumination;
uniform sampler2D   gMoments;
uniform sampler2D   gHistoryLength;
uniform sampler2D   gLinearZAndNormal;

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

void main(){
    //float4 posH = vsOut.posH;
    //int2 ipos = int2(posH.xy);
    ivec2 imageDim = textureSize(gIllumination,0);
    float texelSizeX = 1.0 / float(imageDim.x);
    float texelSizeY = 1.0 / float(imageDim.y);
    vec2  texelSize = vec2(texelSizeX,texelSizeY);

    float h = texture(gHistoryLength,TexCoords).x;
    ivec2 screenSize = textureSize(gHistoryLength, 0);

    if (h < 4.0) // not enough temporal history available
    {
        float sumWIllumination   = 0.0;
        vec3  sumIllumination   = vec3(0.0, 0.0, 0.0);
        vec2  sumMoments  = vec2(0.0, 0.0);

        const vec4  illuminationCenter = texture(gIllumination,TexCoords);
        const float lIlluminationCenter = luminance(illuminationCenter.rgb);

        const vec2 zCenter = texture(gLinearZAndNormal,TexCoords).xy;
        if (zCenter.x < 0)
        {
            // current pixel does not a valid depth => must be envmap => do nothing
            OutIllumination = illuminationCenter;
        }
        const vec3 nCenter = oct_to_ndir_snorm(texture(gLinearZAndNormal,TexCoords).zw);
        const float phiLIllumination   = gPhiColor;
        const float phiDepth     = max(zCenter.y, 1e-8) * 3.0;

        // compute first and second moment spatially. This code also applies cross-bilateral
        // filtering on the input illumination.
        const int radius = 3;

        for (int yy = -radius; yy <= radius; yy++)
        {
            for (int xx = -radius; xx <= radius; xx++)
            {
                const vec2 p     = TexCoords + vec2(xx * texelSizeX, yy * texelSizeY);
                const bool inside = all(p >= vec2(0.0,0.0)) && all(p < vec2(1.0,1.0));
                const bool samePixel = (xx ==0 && yy == 0);
                const float kernel = 1.0;

                if (inside)
                {
                    const vec3  illuminationP = texture(gIllumination,p).rgb;
                    const vec2  momentsP      = texture(gMoments,p).xy;
                    const float lIlluminationP = luminance(illuminationP.rgb);
                    const float zP = texture(gLinearZAndNormal,p).x;
                    const vec3  nP = oct_to_ndir_snorm(texture(gLinearZAndNormal,p).zw);

                    const float w = computeWeight(
                        zCenter.x, zP, phiDepth * length(float2(xx, yy)),
                        nCenter, nP, gPhiNormal,
                        lIlluminationCenter, lIlluminationP, phiLIllumination);

                    sumWIllumination += w;
                    sumIllumination  += illuminationP * w;
                    sumMoments += momentsP * w;
                }
            }
        }

        // Clamp sum to >0 to avoid NaNs.
        sumWIllumination = max(sumWIllumination, 1e-6f);

        sumIllumination   /= sumWIllumination;
        sumMoments  /= sumWIllumination;

        // compute variance using the first and second moments
        float variance = sumMoments.g - sumMoments.r * sumMoments.r;

        // give the variance a boost for the first frames
        variance *= 4.0 / h;

        OutIllumination = vec4(sumIllumination, variance.r);
    }
    else
    {
        // do nothing, pass data unmodified
        OutIllumination = texture(gIllumination,TexCoords);
    }
}