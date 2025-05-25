#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D sceneTexture;
uniform vec2 resolution;

void main()
{
    vec2 texel = 1.0 / resolution;
    vec3 center = texture(sceneTexture, fragTexCoord).rgb;
    //center = floor(center * 4.0) / 4.0;

    float ao = 0.0;

    // Sample 4 diagonal neighbors
    ao += length(center - texture(sceneTexture, fragTexCoord + texel * vec2(-1, -1)).rgb);
    ao += length(center - texture(sceneTexture, fragTexCoord + texel * vec2( 1, -1)).rgb);
    ao += length(center - texture(sceneTexture, fragTexCoord + texel * vec2(-1,  1)).rgb);
    ao += length(center - texture(sceneTexture, fragTexCoord + texel * vec2( 1,  1)).rgb);

    ao = clamp(ao * 0.226, 0.0, 1.0); // tweak strength

    vec3 final = center - ao * 0.2; // darken based on AO


    finalColor = vec4(final, 1.0);
    

}


// #version 330

// in vec2 fragTexCoord;
// out vec4 finalColor;

// uniform sampler2D texture0;
// uniform vec2 screenSize;

// void main()
// {
//     vec4 texColor = texture(texture0, fragTexCoord);

//     // Simulate distance using Y coordinate (retro fog trick)
//     float fogFactor = clamp((fragTexCoord.y - 0.4) * 0.0, 0.0, 1.0);

//     vec3 fogColor = vec3(0.7, 0.8, 1.0); // fog blend
//     vec3 color = mix(texColor.rgb, fogColor, fogFactor);

//     finalColor = vec4(color, 1.0);
// }
