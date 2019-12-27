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
layout(location = 3) in vec3 inBarry;

flat layout(location = 0) out float intensity;
layout(location = 1) out vec3 tileColour;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 outBarry;

void main() {
  gl_Position = ubo.mvp * vec4(inPosition, 1.0);
    outBarry = inBarry;
  // vec3 lightDir = normalize(vec3(0,-10.f,-10.f));
  // vec3 lightDir = normalize(vec3(0,0,1.f));
  intensity = dot(normalize(inNormal), ubo.lightDir);
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
  //intensity = 1.0;
  fragTexCoord = vec2(inPosition.x, inPosition.y);
}
