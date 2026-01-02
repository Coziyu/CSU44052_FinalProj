#version 330 core

in vec4 color;
in vec3 fragPos;
in vec3 fragNorm;


uniform vec3 lightPosition;

out vec4 finalColor;

vec3 reinhard_tone_mapper(vec3 v){
    return v / (1.0 + v);
}

vec3 gamma_correct(vec3 v, float g){
    return pow(v, vec3(1.0 / g));
}

void main()
{
    vec3 ambient = vec3(0.1, 0.1, 0.1);

    vec3 norm = normalize(fragNorm);
    vec3 lightDir = normalize(lightPosition);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

    vec3 result = ambient + diffuse;
    result = result * color.rgb;
    result = reinhard_tone_mapper(result);
    result = gamma_correct(result, 2.2);

    finalColor = vec4(result, 1.0);
}
