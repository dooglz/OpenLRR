#version 450
#extension GL_ARB_separate_shader_objects : enable

flat layout(location = 0) in float intensity;
flat layout(location = 1) in vec3 tileColour;
layout(location = 2) in vec2 fragTexCoord;
layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
  // vec2 fragTexCoord = vec2(0.1,0.1);
  // outColor = vec4(fragColor, 1.0);
  outColor = mix(texture(texSampler, fragTexCoord), vec4(tileColour, 1.0f), 0.5f) * intensity;
}
