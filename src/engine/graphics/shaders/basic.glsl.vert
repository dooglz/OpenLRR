#version 450
#extension GL_ARB_separate_shader_objects : enable
#define vLIT_GLOBAL_UBO_BINDING 0
#define vLIT_MODEL_UBO_BINDING 1

layout(binding = 0, set = vLIT_GLOBAL_UBO_BINDING) uniform vLit_global_UniformBufferObject {
  mat4 view;
  mat4 proj;
  vec4 lightDir;
  vec4 pointLight;
}
gubo;

layout(binding = 0, set = vLIT_MODEL_UBO_BINDING) uniform vLit_object_UniformBufferObject { mat4 model; }
mubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inBarry;

flat layout(location = 0) out float intensity;
layout(location = 1) out vec3 tileColour;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 outBarry;
layout(location = 4) out vec3 fragVert;
flat layout(location = 5) out vec3 outNormal;

void main() {
  fragVert = inPosition;
  outNormal = inNormal;
  const mat4 mvp = gubo.proj * gubo.view * mubo.model;
  gl_Position = mvp * vec4(inPosition, 1.0);

  outBarry = inBarry;
  // vec3 lightDir = normalize(vec3(0,-10.f,-10.f));
  // vec3 lightDir = normalize(vec3(0,0,1.f));
  // intensity = dot(normalize(inNormal), gubo.lightDir);
  intensity = 1;
  /*
  int tri = gl_VertexIndex % 3;
  switch(tri){
  case 0:
  tileColour = vec3(1,0,0);
  break;
  case 1:
  tileColour = vec3(0,1,0);
  break;
  case 2:
  tileColour = vec3(0,0,1);
  break;

  }*/
  tileColour = inColor;
  //  tileColour = inNormal;
  // intensity = 1.0;
  fragTexCoord = vec2(inPosition.x, inPosition.y);
}
