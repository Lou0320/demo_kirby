#version 330
out vec4 fragColor;
uniform vec4 couleur;
uniform vec4 LightColor;
uniform int sky;
uniform sampler2D tex;

in float intensite_diffus;
in vec2 tc;

void main() {
  // pour voir la coordonnée de texture : vec4(tc, 0.0, 1.0);
  if(sky == 0) {
    // Calculate the final light color with increased intensity
    vec3 finalLight = LightColor.rgb * intensite_diffus;
    
    // Brighter ambient light for all objects
    vec3 ambient = vec3(0.5, 0.5, 0.5);
    finalLight = finalLight + ambient;
    
    // Mix the texture with the colored lighting with higher light contribution
    fragColor = mix(texture(tex, tc), vec4(finalLight, 1.0) * couleur, 0.4);
  }
  else
    fragColor = texture(tex, vec2(tc.x, 1.0 - tc.y));
}