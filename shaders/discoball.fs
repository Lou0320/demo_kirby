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
    vec3 viewDir = normalize(-fragPos);
    vec3 normal = normalize(normalVec);
    
    // Amélioration du calcul des facettes
    float facetSize = 0.08; // Facettes plus petites
    vec3 spherePos = normalize(fragPos);
    float facetVariation = sin(floor(spherePos.x / facetSize) * 3.0) * 
                          cos(floor(spherePos.y / facetSize) * 3.0) * 
                          sin(floor(spherePos.z / facetSize) * 3.0);
    
    // Calcul des normales des facettes avec plus de détails
    vec3 facetedNormal = normalize(normal + 
        (sin(spherePos.x * 80.0) * cos(spherePos.y * 80.0) + facetVariation) * vec3(0.35) +
        (cos(spherePos.z * 80.0) * sin(spherePos.x * 80.0) + facetVariation) * vec3(0.35));
    
    // Calcul spéculaire amélioré
    vec3 reflectDir = reflect(-viewDir, facetedNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 24.0) * (1.0 + facetVariation);
    
    // Amélioration de la couverture sphérique
    float sphereCoverage = max(dot(normal, viewDir), 0.3);

    vec3 baseColor = vec3(0.8 + facetVariation * 0.3) * sphereCoverage;
    vec3 rainbowTint = vec3(
        sin(fragPos.x * 15.0 + fragPos.y * 15.0) * 0.5 + 0.9,
        cos(fragPos.y * 15.0 + fragPos.z * 15.0) * 0.5 + 0.9,
        sin(fragPos.z * 15.0 + fragPos.x * 15.0) * 0.5 + 0.9
    );

    float minBrightness = 0.8;
    vec3 reflectedLight = max(LightColor.rgb * (spec * 8.0 + minBrightness), vec3(minBrightness));

    vec3 finalColor = baseColor * reflectedLight * rainbowTint * 2.0;
    finalColor += vec3(0.5) * spec;
    
    fragColor = vec4(finalColor, 1.0);
}