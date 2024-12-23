#version 450

layout(binding = 0) uniform sampler2D inputTexture; // 첫 번째 서브패스의 출력 텍스처

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

const float gamma = 2.2;

void main() {
    vec3 color = texture(inputTexture, fragTexCoord).rgb;
    color = pow(color, vec3(1.0 / gamma)); // 감마 보정 적용
    outColor = vec4(color, 1.0);
}
