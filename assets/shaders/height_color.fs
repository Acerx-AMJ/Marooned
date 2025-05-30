#version 330

in vec3 fragNormal;
in vec3 fragPosition;

out vec4 finalColor;

uniform vec3 cameraPos;

void main()
{
    float height = fragPosition.y;

    // Color zones
    //vec3 waterColor = vec3(0.0, 0.4, 0.8);
    //vec3 grassColor = vec3(0.1, 0.7, 0.1);
    vec3 rockColor  = vec3(0.4, 0.3, 0.2);
    vec3 snowColor  = vec3(1.0, 1.0, 1.0);

    vec3 waterColor = vec3(0.1, .6, 1.0);//vec3(0.01, 0.3, 1.0); //vec3(0.0, 0.3, 1.0);
    vec3 sandColor  = vec3(0.9, 0.9, 0.01); //vec3(0.9, 0.9, 0.2);
    vec3 grassColor = vec3(0.2, 0.7, 0.2); 

    float brightness = 0.60;

    // Blend based on height
    vec3 color = waterColor;

    if (height > 60.0) {
        float t = smoothstep(30.0, 80.0, height);
        color = mix(waterColor, sandColor, t);

        if (height > 120.0) {
            float s = smoothstep(100.0, 130.0, height);
            color = mix(sandColor, grassColor, s);
        }
    }

    // Apply brightness
    color *= brightness;

    // Distance-based desaturation
    float dist = length(fragPosition - cameraPos);
    //float desatFactor = clamp((dist - 100.0) / 500.0, 0.0, 1.0); // fades out over distance
    float desatFactor = clamp((dist - 5000.0) / 20000.0, 0.0, 1.0);


    vec3 fogColor = vec3(0.6, 0.7, 0.9);
    float grayscale = dot(color, vec3(0.299, 0.587, 0.114)); // Luminance formula
    
    color = mix(color, vec3(grayscale), desatFactor); // blend toward grayscale

    finalColor = vec4(color, 1.0);


}


// #version 330

// in vec3 fragNormal;
// in vec3 fragPosition;

// out vec4 finalColor;

// uniform vec3 cameraPos;

// void main()
// {
//     float height = fragPosition.y;

//     // Colors
//     vec3 waterColor = vec3(0.9, 0.9, 0.01);//vec3(0.01, 0.3, 1.0); //vec3(0.0, 0.3, 1.0);
//     vec3 sandColor  = vec3(0.9, 0.9, 0.01); //vec3(0.9, 0.9, 0.2);
//     vec3 grassColor = vec3(0.01, 0.4, 0.01); 

//     // Thresholds
//     float waterLevel = 20.0;
//     float sandLevel  = 30.0;
//     float grassLevel = 150.0;

//     vec3 color;

//     if (height < waterLevel) {
//         color = waterColor;
//     } else if (height < sandLevel) {
//         float t = (height - waterLevel) / (sandLevel - waterLevel);
//         color = mix(waterColor, sandColor, t);
//     } else if (height < grassLevel) {
//         float t = (height - sandLevel) / (grassLevel - sandLevel);
//         color = mix(sandColor, grassColor, t);
//     } else {
//         color = grassColor;
//     }

//     float brightness = 0.35;
//     vec3 corrected = pow(color * brightness, vec3(1.0 / 2.2));
//     finalColor = vec4(corrected, 1.0);
// }
