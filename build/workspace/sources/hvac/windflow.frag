#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;

    vec4 windColor;
    vec2 iResolution;

    float time;
    float flow;
    float strandCount;
    float spread;
    float ampMain;
    float ampDetail;
    float thickness;
    float glowScale;
    float brightness;
};

const float PI = 3.14159265358979323846;
const int MAX_STRANDS = 9;

float saturate(float x)
{
    return clamp(x, 0.0, 1.0);
}

float smooth01(float a, float b, float x)
{
    x = saturate((x - a) / (b - a));
    return x * x * (3.0 - 2.0 * x);
}

float wrapDistance(float a, float b)
{
    float d = abs(a - b);
    return min(d, 1.0 - d);
}

float gaussian(float x, float sigma)
{
    sigma = max(0.0001, sigma);
    return exp(-(x * x) / (2.0 * sigma * sigma));
}

float laneY(float t, float lane, float count)
{
    float mid = max(0.5, (count - 1.0) * 0.5);
    float laneNorm = count > 1.0 ? (lane - mid) / mid : 0.0;

    float laneOffset =
            laneNorm * spread
            * (0.92 + 0.16 * sin(lane * 1.7 + 0.8));

    float gather = 1.0 - smooth01(0.56, 1.0, t);
    gather = 0.06 + 0.94 * pow(gather, 1.75);

    float body = pow(max(0.0, sin(t * PI)), 0.72);

    float mainWave =
            sin(t * PI * 2.0
                + time * 0.42
                + lane * 0.34) * ampMain;

    float detailWave =
            sin(t * PI * 7.9
                - time * 1.8
                + lane * 0.92) * ampDetail
            * (0.45 + 0.55 * body);

    float microWave =
            sin(t * PI * 14.0
                + time * 2.7
                + lane * 1.28) * 0.9 * body;

    float drift = sin(t * PI * 1.1 - time * 0.18) * 2.8;
    float centerY = iResolution.y * 0.54 + drift;

    return centerY
            + laneOffset * gather
            + mainWave * (0.72 + 0.28 * body)
            + detailWave
            + microWave;
}

void main()
{
    vec2 fragPx = qt_TexCoord0 * iResolution;
    float t = qt_TexCoord0.x;
    float count = max(1.0, strandCount);

    float glow = 0.0;
    float core = 0.0;
    float whiteCore = 0.0;
    float sparkle = 0.0;

    for (int i = 0; i < MAX_STRANDS; ++i) {
        if (float(i) >= count)
            break;

        float lane = float(i);
        float mid = max(0.5, (count - 1.0) * 0.5);
        float laneNormAbs = count > 1.0 ? abs((lane - mid) / mid) : 0.0;
        float laneWeight = 1.0 - laneNormAbs * 0.16;

        float y = laneY(t, lane, count);

        float body = pow(max(0.0, sin(t * PI)), 0.4);

        float headFade = 1.0 - smooth01(0.76, 1.0, t);
        headFade = 0.04 + 0.96 * pow(headFade, 1.2);

        float shine1 =
                exp(-pow(wrapDistance(t, flow), 2.0) / 0.0011);
        float shine2 =
                exp(-pow(wrapDistance(t, mod(flow + 0.16, 1.0)), 2.0) / 0.0048);
        float shine3 =
                exp(-pow(wrapDistance(t, mod(flow + 0.34, 1.0)), 2.0) / 0.0120);

        float shine = min(1.6, shine1 + shine2 * 0.85 + shine3 * 0.35);

        float alphaBase = body * headFade * laneWeight;

        float laneThickness =
                thickness
                * (0.95 + body * 1.10)
                * (1.0 - laneNormAbs * 0.08);

        float distPx = abs(fragPx.y - y);

        glow += gaussian(
                    distPx,
                    max(1.0, laneThickness * glowScale * (1.2 + 0.2 * shine))
                ) * alphaBase * (0.50 + shine * 0);

        core += gaussian(
                    distPx,
                    max(0.7, laneThickness * (0.95 + 0.10 * shine))
                ) * alphaBase * (0.65 + shine * 0);

        whiteCore += gaussian(
                        distPx,
                        max(0.45, laneThickness * 0.33 * (1.0 + 0.20 * shine))
                     ) * alphaBase * (0.20 + shine * 0);

        sparkle += gaussian(
                        distPx,
                        max(0.30, laneThickness * 0.16)
                   ) * alphaBase * shine1 * 0;
    }

    vec3 color =
            windColor.rgb * (glow * 0.34 + core * 0.92)
            + vec3(1.0) * (whiteCore * 0.95 + sparkle * 0.75);

    color *= brightness;

    float alpha =
            glow * 0.10
            + core * 0.34
            + whiteCore * 0.48
            + sparkle * 0.32;

    alpha = clamp(alpha, 0.0, 1.0);

    fragColor = vec4(min(color, vec3(1.0)), alpha * qt_Opacity);
}
