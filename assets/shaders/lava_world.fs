// lava_world.fs
#version 330

in vec3 vWorldPos;
in vec4 vColor;

out vec4 finalColor;

uniform sampler2D texture0;     // raylib binds your lava texture here
uniform float uTime;
uniform vec2  uScrollDir;       // e.g. (0.07, 0.0)
uniform float uSpeed;           // e.g. 1.0
uniform vec2  uWorldOffset;     // usually your dungeon origin (x,z)
uniform float uUVScale;         // repeats per world unit (see below)
uniform float uDistortFreq;     // e.g. 6.0
uniform float uDistortAmp;      // e.g. 0.02
uniform vec3  uEmissive;        // e.g. (1.0, 0.4, 0.08)    
uniform float uEmissiveGain;    // e.g. 1.8

void main() {
    // World-projected UVs (continuous across tiles)
    vec2 uv = (vWorldPos.xz - uWorldOffset) * uUVScale;

    // Scroll & ripple
    uv += uScrollDir * (uTime * uSpeed);
    float sx = sin((uv.y + uTime*0.35) * uDistortFreq);
    float cy = cos((uv.x - uTime*0.27) * (uDistortFreq*0.8));
    uv += vec2(sx, cy) * uDistortAmp;

    vec3 base = texture(texture0, uv).rgb * vColor.rgb;
    //if (vWorldPos.y > 500.0) discard; //fix the giant lava texture in the sky by discarding anything above 500.


    // Emissive lava (self-lit)
    vec3 color = base + uEmissive * uEmissiveGain;
    finalColor = vec4(color, 1.0);
    //finalColor = vec4(1.0, 0.0, 1.0, 1.0);//Magenta test
}
