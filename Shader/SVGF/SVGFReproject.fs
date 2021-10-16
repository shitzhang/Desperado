#version 460 core

layout (location = 0) out vec4  OutIllumination;
layout (location = 1) out vec2  OutMoments;
layout (location = 2) out float OutHistoryLength;

in vec2 TexCoords;

uniform sampler2D   gMotion;
uniform sampler2D   gPositionNormalFwidth;
uniform sampler2D   gColor;
uniform sampler2D   gAlbedo;
uniform sampler2D   gEmission;
uniform sampler2D   gPrevIllum;
uniform sampler2D   gPrevMoments;
uniform sampler2D   gLinearZAndNormal;
uniform sampler2D   gPrevLinearZAndNormal;
uniform sampler2D   gPrevHistoryLength;

uniform float       gAlpha;
uniform float       gMomentsAlpha;

vec2 oct_wrap(vec2 v)
{
    vec2 weight = vec2((v.x >= 0.f ? 1.f : -1.f) , (v.y >= 0.f ? 1.f : -1.f));
    return (vec2(1.f , 1.f) - abs(v.yx)) * weight;
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

bool isReprjValid(vec2 coord, float Z, float Zprev, float fwidthZ, vec3 normal, vec3 normalPrev, float fwidthNormal)
{
    //const int2 imageDim = getTextureDims(gColor, 0);
    ivec2 imageDim = textureSize(gColor,0);
    float texelSizeX = 1.0 / float(imageDim.x);
    float texelSizeY = 1.0 / float(imageDim.y);
    vec2 texelSize = vec2(texelSizeX,texelSizeY);

    // check whether reprojected pixel is inside of the screen
    if (any(bvec2(coord.x < 0.0 , coord.y < 0.0)) || any(bvec2(coord.x > 1.0 , coord.y > 1.0))) return false;

    // check if deviation of depths is acceptable
    if (abs(Zprev - Z) / (fwidthZ + 1e-2f) > 10.f) return false;

    // check normals for compatibility
    if (distance(normal, normalPrev) / (fwidthNormal + 1e-2) > 16.0) return false;

    return true;
}

bool loadPrevData(vec2 texCoords, out vec4 prevIllum, out vec2 prevMoments, out float historyLength)
{
    //const int2 ipos = posH;
    ivec2 imageDim = textureSize(gColor,0);
    float texelSizeX = 1.0 / float(imageDim.x);
    float texelSizeY = 1.0 / float(imageDim.y);
    vec2  texelSize = vec2(texelSizeX,texelSizeY);

    vec2 motion = texture(gMotion,texCoords).xy;
    float normalFwidth = texture(gPositionNormalFwidth,texCoords).y;

    vec2 texCoordsPrev = texCoords + motion.xy;

    vec2 depth = texture(gLinearZAndNormal,texCoords).xy;
    vec3 normal = oct_to_ndir_snorm(texture(gLinearZAndNormal,texCoords).zw);

    prevIllum   = vec4(0,0,0,0);
    prevMoments = vec2(0,0);

    bool v[4];
    //const vec2 posPrev = floor(posH.xy) + motion.xy * imageDim;
    vec2 offset[4] = vec2[]( 
        vec2(0.0, 0.0), 
        vec2(texelSizeX, 0.0), 
        vec2(0, texelSizeY), 
        vec2(texelSizeX, texelSizeY) 
    );

    // check for all 4 taps of the bilinear filter for validity
    bool valid = false;
    for (int sampleIdx = 0; sampleIdx < 4; sampleIdx++)
    {
        vec2 loc = texCoordsPrev + offset[sampleIdx];
        vec2 depthPrev =texture(gPrevLinearZAndNormal,loc).xy;
        vec3 normalPrev = oct_to_ndir_snorm(texture(gPrevLinearZAndNormal,loc).zw);

        v[sampleIdx] = isReprjValid(texCoordsPrev, depth.x, depthPrev.x, depth.y, normal, normalPrev, normalFwidth);

        valid = valid || v[sampleIdx];
    }

    if (valid)
    {
        float sumw = 0;
        float x = fract(imageDim.x * texCoordsPrev.x);       
        float y = fract(imageDim.y * texCoordsPrev.y);

        // bilinear weights
        float w[4] = float[]( (1 - x) * (1 - y),
                                   x  * (1 - y),
                              (1 - x) *      y,
                                   x  *      y);

        // perform the actual bilinear interpolation
        for (int sampleIdx = 0; sampleIdx < 4; sampleIdx++)
        {
            vec2 loc = texCoordsPrev + offset[sampleIdx];
            if (v[sampleIdx])
            {
                prevIllum   += w[sampleIdx] * texture(gPrevIllum,loc);
                prevMoments += w[sampleIdx] * texture(gPrevMoments,loc).xy;
                sumw        += w[sampleIdx];
            }
        }

        // redistribute weights in case not all taps were used
        valid = (sumw >= 0.01);
        prevIllum   = valid ? prevIllum / sumw   : vec4(0, 0, 0, 0);
        prevMoments = valid ? prevMoments / sumw : vec2(0, 0);
    }

    if (!valid) // perform cross-bilateral filter in the hope to find some suitable samples somewhere
    {
        float nValid = 0.0;

        // this code performs a binary descision for each tap of the cross-bilateral filter
        const int radius = 1;
        for (int yy = -radius; yy <= radius; yy++)
        {
            for (int xx = -radius; xx <= radius; xx++)
            {
                vec2 p = texCoordsPrev + vec2(xx * texelSizeX, yy * texelSizeY);
                vec2 depthFilter = texture(gPrevLinearZAndNormal,p).xy;
                vec3 normalFilter = oct_to_ndir_snorm(texture(gPrevLinearZAndNormal,p).zw);

                if (isReprjValid(texCoordsPrev, depth.x, depthFilter.x, depth.y, normal, normalFilter, normalFwidth))
                {
                    prevIllum += texture(gPrevIllum,p);
                    prevMoments += texture(gPrevMoments,p).xy;
                    nValid += 1.0;
                }
            }
        }
        if (nValid > 0)
        {
            valid = true;
            prevIllum   /= nValid;
            prevMoments /= nValid;
        }
    }

    if (valid)
    {
        // crude, fixme
        historyLength = texture(gPrevHistoryLength,texCoordsPrev).x;
    }
    else
    {
        prevIllum   = vec4(0,0,0,0);
        prevMoments = vec2(0,0);
        historyLength = 0;
    }

    return valid;
}

// not used currently
float computeVarianceScale(float numSamples, float loopLength, float alpha)
{
    float aa = (1.0 - alpha) * (1.0 - alpha);
    return (1.0 - pow(aa, min(loopLength, numSamples))) / (1.0 - aa);
}

void main()
{             
    //const float4 posH = vsOut.posH;
    //const int2 ipos = posH.xy;

    vec3 illumination = demodulate(texture(gColor, TexCoords).rgb - texture(gEmission, TexCoords).rgb, texture(gAlbedo, TexCoords).rgb);
    // Workaround path tracer bugs. TODO: remove this when we can.
    if (isnan(illumination.x) || isnan(illumination.y) || isnan(illumination.z))
    {
        illumination = vec3(0, 0, 0);
    }

    float historyLength;
    vec4 prevIllumination;
    vec2 prevMoments;
    bool success = loadPrevData(TexCoords, prevIllumination, prevMoments, historyLength);
    historyLength = min(32.0f, success ? historyLength + 1.0f : 1.0f);


    // this adjusts the alpha for the case where insufficient history is available.
    // It boosts the temporal accumulation to give the samples equal weights in
    // the beginning.
    float alpha        = success ? max(gAlpha,        1.0 / historyLength) : 1.0;
    float alphaMoments = success ? max(gMomentsAlpha, 1.0 / historyLength) : 1.0;

    // compute first two moments of luminance
    vec2 moments;
    moments.r = luminance(illumination);
    moments.g = moments.r * moments.r;

    vec2 pm = moments;

    // temporal integration of the moments
    moments = mix(prevMoments, moments, alphaMoments);

    float variance = max(0.f, moments.g - moments.r * moments.r);

    //variance *= computeVarianceScale(16, 16, alpha);

    // temporal integration of illumination
    OutIllumination = mix(prevIllumination,   vec4(illumination,   0), alpha);
    //OutIllumination = vec4(illumination, 1.0);
    // variance is propagated through the alpha channel
    OutIllumination.a = variance;
    OutMoments = moments;

    OutHistoryLength = historyLength;
}