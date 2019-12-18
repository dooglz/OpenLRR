#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
  mat4 model;
  mat4 view;
  mat4 proj;
  mat4 mvp;
  vec3 lightDir;
}
ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

flat layout(location = 0) out vec3 fragColor;

void main() {
  gl_Position = ubo.mvp * vec4(inPosition, 1.0);

  // vec3 lightDir = normalize(vec3(0,-10.f,-10.f));
  // vec3 lightDir = normalize(vec3(0,0,1.f));
  float intensity = dot(normalize(inNormal), ubo.lightDir);

  // fragColor = vec3(1.f,inPosition.z,inPosition.z);
  fragColor = normalize(inNormal);
  fragColor = inColor * intensity;
}
