#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;
uniform float edgeThreshold;
uniform int colorLevels;
uniform vec2 screenSize;

// sobel kernels edge detection
const mat3 sobelX = mat3(
    -1.0, 0.0, 1.0,
    -2.0, 0.0, 2.0,
    -1.0, 0.0, 1.0
);

const mat3 sobelY = mat3(
    -1.0, -2.0, -1.0,
     0.0,  0.0,  0.0,
     1.0,  2.0,  1.0
);

float getGrayscale(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

// [ACKN] ChatGPT generated these two functions for RGB <-> HSV conversion.
vec3 rgb2hsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float sampleDepth(vec2 uv) {
    return texture(depthTexture, uv).r;
}

vec3 sampleColor(vec2 uv) {
    return texture(screenTexture, uv).rgb;
}

float detectEdge(vec2 uv) {
    vec2 texelSize = 1.0 / screenSize;
    
    float colorGx = 0.0;
    float colorGy = 0.0;
    float depthGx = 0.0;
    float depthGy = 0.0;
    
    // 3x3 sample
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            vec2 offset = vec2(float(i), float(j)) * texelSize;
            vec2 sampleUV = uv + offset;
            
            float gray = getGrayscale(sampleColor(sampleUV));
            float depth = sampleDepth(sampleUV);
            
            float weight = (i + 1) + (j + 1) * 3;
            float kx = sobelX[j + 1][i + 1];
            float ky = sobelY[j + 1][i + 1];
            
            colorGx += gray * kx;
            colorGy += gray * ky;
            depthGx += depth * kx;
            depthGy += depth * ky;
        }
    }
    
    float colorEdge = sqrt(colorGx * colorGx + colorGy * colorGy);
    float depthEdge = sqrt(depthGx * depthGx + depthGy * depthGy);
    
    return max(colorEdge, depthEdge * 100.0);
}

// cel shading in HSV for continous discontinuity LOL
vec3 quantizeColor(vec3 color, int levels) {
    vec3 hsv = rgb2hsv(color);
    
    float fLevels = float(levels - 1);
    
    hsv.y = round(hsv.y * fLevels) / fLevels;  // saturation
    hsv.z = round(hsv.z * fLevels) / fLevels;  // brightness
    
    return hsv2rgb(hsv);
}

// [ACKN] ChatGPT assisted in creating this function to increase saturation.
vec3 saturate(vec3 color, float amount) {
    float gray = getGrayscale(color);
    return mix(vec3(gray), color, 1.0 + amount);
}

void main()
{
    vec3 color = sampleColor(TexCoords);
    
    vec3 quantized = quantizeColor(color, colorLevels);
    
    // cartoonish looks with more saturation
    quantized = saturate(quantized, 0.2);
    
    float edge = detectEdge(TexCoords);
    
    float edgeFactor = smoothstep(edgeThreshold * 0.5, edgeThreshold, edge);
    vec3 edgeColor = vec3(0.0);
    
    vec3 result = mix(quantized, edgeColor, edgeFactor);
    
    FragColor = vec4(result, 1.0);
}
