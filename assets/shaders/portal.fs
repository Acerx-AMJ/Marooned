#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

// --------- Tunable uniforms ----------
uniform float u_time;            // seconds
uniform float u_speed;           // 0.0..5.0   (master animation rate)
uniform float u_swirlStrength;   // 0.0..3.0   (radians of swirl)
uniform float u_swirlScale;      // 2.0..30.0  (how tight the swirl bands are)
uniform vec3  u_colorA;          // base color (e.g. teal)
uniform vec3  u_colorB;          // base color (e.g. magenta)
uniform float u_edgeFeather;     // 0.01..0.2  (softness at circle edge)
uniform float u_rings;           // 0.0..1.0   (how visible the moving rings are)
uniform float u_glowBoost;       // 0.0..2.0   (brighten center for bloom)
// -------------------------------------

// Cheap hash noise (value noise style)
float hash(vec2 p){
    // offset to avoid symmetry at (0,0)
    p = fract(p*vec2(123.34, 345.45));
    p += dot(p, p + 34.345);
    return fract(p.x * p.y);
}

// Smooth Hertz color palette (cosine palette)
vec3 palette(float t, vec3 a, vec3 b) {
    // t in [0,1]
    // Base: mix a/b via cosine for richer movement
    vec3 c = 0.5 + 0.5*cos(6.28318*(vec3(0.0, 0.33, 0.67) + t));
    return mix(a, b, c);
}

void main() {
    // Map UV to [-1,1] with (0,0) at center
    vec2 uv = fragTexCoord;
    vec2 p  = uv*2.0 - 1.0;

    // Keep portal strictly round regardless of quad aspect
    // If your door quad isn’t square, you can scale p.x or p.y here to compensate.
    float r = length(p);
    float ang = atan(p.y, p.x);

    // Swirl: angle offset based on radius
    float swirl = u_swirlStrength * sin(u_time*u_speed + r * u_swirlScale);
    ang += swirl;

    // Reproject to a secondary coordinate system for patterning
    vec2 q = vec2(cos(ang), sin(ang)) * r;

    // Animated ring pattern traveling inward/outward
    float rings = sin( (r*24.0) - (u_time*u_speed*2.0) );
    float ringMask = mix(0.0, 0.5 + 0.5*rings, u_rings);

    // Add subtle noisy drift (prevents banding/robotic motion)
    float n = hash(q*8.0 + u_time*0.2);

    // Core color field: cosine palette driven by swirl + noise
    float t = 0.5 + 0.5*sin( (q.x+q.y)*4.0 + u_time*u_speed + n*2.0 );
    vec3 baseCol = palette(t, u_colorA, u_colorB);

    // Combine with ring accents
    baseCol += ringMask * 0.25;

    // Glow boost towards center (for bloom). 1 - smoothstep near center
    float centerGlow = 1.0 - smoothstep(0.0, 0.6, r);
    baseCol += centerGlow * u_glowBoost;

    // Clamp to sane HDR-ish range (ACES tonemap will tame it if you have it)
    baseCol = min(baseCol, vec3(4.0));

    // Soft circular edge so it sits inside the arch
    float edge = 1.0 - smoothstep(1.0 - u_edgeFeather, 1.0, r);

    // Final
    vec3 col = baseCol * edge;

    finalColor = vec4(col, 1.0) * fragColor; // respect any global tint if you pass one
}
