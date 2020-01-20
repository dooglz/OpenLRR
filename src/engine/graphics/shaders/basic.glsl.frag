#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_OES_standard_derivatives : enable
#extension GL_ARB_separate_shader_objects : enable

#define vLIT_GLOBAL_UBO_BINDING 0
#define vLIT_MODEL_UBO_BINDING  1
#define vLIT_IMAGE_UBO_BINDING  2

layout(binding = 0, set=vLIT_GLOBAL_UBO_BINDING) uniform vLit_global_UniformBufferObject {
  mat4 view;
  mat4 proj;
  vec4 eyePosition;
  vec4 pointLight;
}
gubo;

layout(binding = 0, set=vLIT_IMAGE_UBO_BINDING) uniform sampler2D texSampler;


flat layout(location = 0) in float intensity;
layout(location = 1) in vec3 tileColour;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 barry;
layout(location = 4) in vec3 fragVert;
flat layout(location = 5) in vec3 normal;
layout(location = 6) in vec3 vertex_position;

layout(location = 0) out vec4 outColor;

// 0 == edge
float edgeFactor() {
  // return 1.0;
  vec3 d = fwidth(barry);
  vec3 a3 = smoothstep(vec3(0.0), d * 1.5, barry);
  return min(min(a3.x, a3.y), a3.z);
}

void main() {
  const vec3 fragPosition = vertex_position;
  // calculate the vector from this pixels surface to the light source
  vec3 surfacePos = fragPosition; //todo fix this for non identity model matrix
  const vec3 surfaceToLight = gubo.pointLight.xyz - fragPosition;
  const vec3 surfaceToCamera = normalize(gubo.eyePosition.xyz - surfacePos);

  // calculate the cosine of the angle of incidence
  //float brightness = dot(normal, surfaceToLight) / (length(surfaceToLight) * length(normal));


  //diffuse
  const float lightIntensity = 1.0f;
  const float materialShininess = 80.0f;
  const float lightAttenuation = 0.8f;
  const float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
  const float diffuse = diffuseCoefficient * lightIntensity;

  //specular
  vec3 view_dir = normalize(gubo.eyePosition.xyz - vertex_position);
  vec3 light_dir = normalize(gubo.pointLight.xyz - vertex_position);
  vec3 half_vector = normalize(light_dir + view_dir);
  float specular = pow(max(dot(normal, half_vector), 0), materialShininess);

  //attenuation
  float distanceToLight = length(gubo.pointLight.xyz - surfacePos);
  float attenuation = 1.0 / (1.0 + lightAttenuation * pow(distanceToLight, 2));

  float brightness = attenuation*(diffuse + specular);

  //float brightness = dot(normal, surfaceToLight);
  brightness = clamp(brightness, 0.0f, 1.0f);
  //brightness = max(brightness, 0.0f);
  outColor = mix(texture(texSampler, fragTexCoord), vec4(tileColour, 1.0f), 0.5f) * brightness;
  outColor = mix(vec4(0, 0.0, 0.0, 1.0), outColor, edgeFactor());
}
