#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;

    vec2 iResolution;

    float time;
    float audioLevel;

    float pad;
    float yBase;
    float gap;
    float centerDip;
    float curveTight;

    float normalOffset;
    float fragThickness;
    float fragGlow;
    float brightness;
    vec4 shaderColor1;
    vec4 shaderColor2;
    vec4 shaderColor3;
};

const int SAMPLES = 40;

float saturate(float x)
{
    return clamp(x, 0.0, 1.0);
}

float smooth01(float a, float b, float x)
{
    x = saturate((x - a) / (b - a));
    return x * x * (3.0 - 2.0 * x);
}

float gaussian(float x, float sigma)
{
    sigma = max(0.0001, sigma);
    return exp(-(x * x) / (2.0 * sigma * sigma));
}

vec2 cubicPoint(float t, vec2 p0, vec2 p1, vec2 p2, vec2 p3)
{
    float it = 1.0 - t;
    return it * it * it * p0
         + 3.0 * it * it * t * p1
         + 3.0 * it * t * t * p2
         + t * t * t * p3;
}

vec2 cubicTangent(float t, vec2 p0, vec2 p1, vec2 p2, vec2 p3)
{
    float it = 1.0 - t;
    return 3.0 * it * it * (p1 - p0)
         + 6.0 * it * t * (p2 - p1)
         + 3.0 * t * t * (p3 - p2);
}

void getCurveControls(bool isLeft, out vec2 p0, out vec2 p1, out vec2 p2, out vec2 p3)
{
    float w = iResolution.x;
    float cx = w * 0.5;

    if (isLeft) {
        float leftEndX = cx - gap * 0.5;
        p0 = vec2(pad, yBase);
        p1 = vec2(pad + (leftEndX - pad) * 1.0, yBase);
        p2 = vec2(pad + (leftEndX - pad) * 0.92, yBase + centerDip * curveTight);
        p3 = vec2(leftEndX, yBase + centerDip);
    } else {
        float rightStartX = cx + gap * 0.5;
        p0 = vec2(rightStartX, yBase + centerDip);
        p1 = vec2(rightStartX + (w - pad - rightStartX) * 0.08, yBase + centerDip * curveTight);
        p2 = vec2(rightStartX + (w - pad - rightStartX) * 0.0, yBase);
        p3 = vec2(w - pad, yBase);
    }
}

void nearestOnCurve(
    vec2 fragPx,
    bool isLeft,
    out vec2 nearestP,
    out vec2 nearestTan,
    out float nearestT,
    out float nearestDist
) {
    vec2 p0, p1, p2, p3;
    getCurveControls(isLeft, p0, p1, p2, p3);

    nearestDist = 1e20;
    nearestT = 0.0;
    nearestP = p0;
    nearestTan = vec2(1.0, 0.0);

    for (int i = 0; i <= SAMPLES; ++i) {
        float t = float(i) / float(SAMPLES);
        vec2 p = cubicPoint(t, p0, p1, p2, p3);
        float d = distance(fragPx, p);
        if (d < nearestDist) {
            nearestDist = d;
            nearestT = t;
            nearestP = p;
            nearestTan = cubicTangent(t, p0, p1, p2, p3);
        }
    }
}

void main()
{
    vec2 fragPx = qt_TexCoord0 * iResolution;
    float w = iResolution.x;
    float cx = w * 0.5;

    bool isLeft = fragPx.x < cx;
    float reactive = saturate(audioLevel);

    vec2 p;
    vec2 tanVec;
    float t;
    float curveDist;
    nearestOnCurve(fragPx, isLeft, p, tanVec, t, curveDist);

    float tanLen = max(length(tanVec), 0.0001);
    vec2 tanDir = tanVec / tanLen;

    vec2 n1 = vec2(-tanDir.y, tanDir.x);
    vec2 n2 = -n1;
    vec2 normalDir = n1.y > n2.y ? n1 : n2;

    float signedDown = dot(fragPx - p, normalDir);

    if (signedDown < 0.0) {
        fragColor = vec4(0.0);
        return;
    }

    float distFromCenter = abs(fragPx.x - cx);
    float halfGap = gap * 0.5;

    if (distFromCenter < halfGap) {
        fragColor = vec4(0.0);
        return;
    }

    float outerDist = cx - pad;
    float smoothAlong = saturate((distFromCenter - halfGap) / max(0.001, outerDist - halfGap));

    float endIn  = smooth01(0.0, 0.015, smoothAlong);
    float endOut = 1.0 - smooth01(0.60, 1.0, smoothAlong);
    float alongMask = endIn * endOut;

    float endTaper = pow(alongMask, 1.20);
    float shapeMask = alongMask;

    float centerThick = reactive * exp(-smoothAlong * 5.0);
    float baseSigma = max(0.20, (fragThickness + centerThick * 1.5 + reactive * 0.2) * (0.05 + 0.95 * endTaper));
    float softSigma = max(1.00, baseSigma * 3.4);
    float glowSigma = max(1.50, (fragGlow + centerThick * 3.0 + reactive * 0.5) * (0.10 + 0.90 * endTaper));

    float depth = signedDown - normalOffset;

    float baseCore = gaussian(depth, baseSigma) * shapeMask;
    float baseSoft = gaussian(depth, softSigma) * shapeMask;
    float baseGlow = gaussian(depth, glowSigma) * shapeMask;

    float pulseSpeed = 1.5;
    float phase = fract(time * pulseSpeed - smoothAlong * 2.0);

    float pulse = exp(-pow((phase - 0.2) / 0.15, 2.0));
    pulse += exp(-pow((phase - 0.7) / 0.12, 2.0)) * 0.6;

    float centerFlash = exp(-smoothAlong * 6.0) * reactive * 3.0;
    float waveFlash = pulse * reactive * 2.5;

    float core = baseCore * (1.0 + pulse * 0.3 + centerFlash + waveFlash);
    float soft = baseSoft * (1.0 + pulse * 0.2 + centerFlash * 0.6 + waveFlash * 0.6);
    float glow = baseGlow * (1.0 + pulse * 0.2 + centerFlash * 0.4 + waveFlash * 0.5);


    vec3 c1 = shaderColor1.rgb;
    vec3 c2 = shaderColor2.rgb;
    vec3 c3 = shaderColor3.rgb;

    float colorPhase = 0.5 + 0.5 * sin((smoothAlong * 1.5 - time * 0.4) * 6.28318);

    vec3 multiColor;
    if (colorPhase < 0.5) {
        multiColor = mix(c1, c2, colorPhase * 2.0);
    } else {
        multiColor = mix(c2, c3, (colorPhase - 0.5) * 2.0);
    }


    float sparkle = gaussian(depth, max(0.40, baseSigma * 0.5)) * shapeMask * (pulse * 0.1 + centerFlash * 0.5 + waveFlash * 0.4);

    vec3 finalColor =
        multiColor * (glow * 0.34 + soft * 0.78 + core * 0.55) +
        vec3(1.0) * (sparkle * 0.10);

    finalColor *= brightness * (1.0 + reactive * 0.08);

    float alpha =
        glow * 0.11 +
        soft * 0.18 +
        core * 0.12 +
        sparkle * 0.025;

    alpha = clamp(alpha, 0.0, 1.0);

    fragColor = vec4(min(finalColor, vec3(1.0)), alpha * qt_Opacity);
}
