#version 330

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

uniform mat4 proj, mod, view;
uniform vec4 Lp;
uniform float nb_repeat;
uniform vec2 decal;

out float intensite_diffus;
out vec2 tc;

void main() {
  // Get model space position
  vec4 worldPos = mod * vec4(pos, 1.0);
  
  // Try different rotation: keep x, flip y
  vec2 rotatedTexCoord = vec2(texCoord.x, -texCoord.y);
  tc = rotatedTexCoord * nb_repeat + decal;
  
  vec4 p = view * worldPos;
  vec4 n = transpose(inverse(view * mod)) * vec4(normal, 0.0);
  vec3 Ld = normalize(p.xyz - Lp.xyz);
  intensite_diffus = clamp(dot(normalize(n.xyz), -Ld), 0.0, 1.0);
  gl_Position = proj * p;
}