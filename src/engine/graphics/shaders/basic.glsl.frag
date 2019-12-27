#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_OES_standard_derivatives : enable

flat layout(location = 0) in float intensity;
 layout(location = 1) in vec3 tileColour;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 barry;
layout(binding = 1) uniform sampler2D texSampler;
layout(location = 0) out vec4 outColor;


//0 == edge
float edgeFactor(){
//return 1.0;
vec3 d = fwidth(barry);
    vec3 a3 = smoothstep(vec3(0.0), d*1.5, barry);
    return min(min(a3.x, a3.y), a3.z);
}

void main() {
  // vec2 fragTexCoord = vec2(0.1,0.1);
  //outColor = vec4(tileColour, 1.0);
  outColor = mix(texture(texSampler, fragTexCoord), vec4(tileColour, 1.0f), 0.5f) * intensity;
 // outColor = vec4(tileColour,1.0);
  outColor = mix( vec4(0,0.0,0.0,1.0),outColor, edgeFactor());
   //outColor =  mix( vec4(0,1.0,1.0,1.0),vec4(barry,1), edgeFactor());

  //outColor = vec4(1.0,1.0,1.0, 1.0);
}
