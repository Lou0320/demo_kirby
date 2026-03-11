#version 330

out vec4 fragColor;
uniform vec4 couleur;
uniform vec4 LightColor;
uniform int sky;
uniform sampler2D tex;

in float intensite_diffus;
in vec2 tc;
in vec3 fragPos;
in vec3 normalVec;

void main() {
    if(sky == 0) {
        vec3 ambient = vec3(0.1, 0.1, 0.1);
        vec3 viewDir = normalize(-fragPos);
        vec3 reflectDir = reflect(-normalize(LightColor.xyz), normalVec);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        vec3 specular = vec3(0.5) * spec;
        vec3 finalLight = (LightColor.rgb * intensite_diffus * 1.5) + ambient + specular;
        vec4 texColor = texture(tex, tc);
        
        float glow = max(0.0, intensite_diffus - 0.5) * 2.0;
        vec3 glowColor = LightColor.rgb * glow;
        
        fragColor = vec4(mix(texColor.rgb * finalLight + glowColor, finalLight, 0.3), texColor.a);
    }
    else {
        vec4 skyColor = texture(tex, vec2(tc.x, 1.0 - tc.y));
        fragColor = skyColor * vec4(0.3, 0.3, 0.3, 1.0);  // Darken the skybox
    }
}