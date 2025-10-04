# Dungeon Lighting System – Design Doc

For 5 months I've been messing with the lighting system for the game. First it was tint based lighting where you just change the tint property of the model depending on distance to a light source. I finaly hit on making a light map plus a shader to read it pixel by pixel. I had AI give me an overview of what we came up with after lots of trial and error. Incase some one wants to reproduce it, or for my own reference. -Joe 10/03/2025

## Overview
The lighting system combines **pre-baked static lights** (with occlusion) and **dynamic lights** (without occlusion) into a unified **lightmap + shader pipeline**.  
- **Static lights**: calculated once at dungeon generation.  
- **Dynamic lights**: updated every frame (e.g., fireballs, traps).  
- The shader samples the lightmap per-pixel and applies lighting to dungeon wall and floor geometry.  

This approach balances **performance** and **visual accuracy**.

---

## Lightmap
- A 2D texture generated at **4× the resolution of the dungeon PNG**.  
  - Dungeon PNG = 32×32 pixels → Lightmap = 128×128 pixels.  
- Each lightmap pixel stores:  
  - **RGB** = accumulated light color/intensity.  
  - **Alpha** = special effects (lava glow, tutorial highlights, etc.).  
- Used by shaders to apply per-pixel lighting across geometry (walls, floors, ceilings).

---

## Static Lighting
- **Light Sources**: defined in dungeon PNG via colored pixels (e.g., yellow = torch).  
- **Bake Pass** (runs once on dungeon load):  
  1. For each light source, cast rays to all nearby wall segments.  
  2. Perform **LOS (line-of-sight)** checks to determine occlusion.  
  3. Stamp light contribution into the lightmap (falloff over distance, color blending).  
- Result is stored in a **StaticBase lightmap buffer**.  
- Each frame, StaticBase is copied into the dynamic buffer before dynamic lights are added.

---

## Dynamic Lighting
- Used for moving/projectile-based lights (e.g., fireballs, traps, lava glow).  
- Updated every frame:  
  1. Copy StaticBase into the **DynamicLightmap buffer**.  
  2. For each dynamic light, stamp its contribution (range, color, falloff).  
  3. No occlusion checks for performance reasons.  
- Shader then samples the **DynamicLightmap**.

---

## Shader Responsibilities
- Reads from the active **DynamicLightmap**.  
- Applies per-pixel lighting to dungeon models:  
  - Walls (vertical faces).  
  - Floors/Ceilings (horizontal tiles).  
  - Special tiles (lava, decals).  
- Blends baked + dynamic contributions seamlessly.  
- Supports bloom/tonemapping for glow effects.  

---

## Special Features
- **Alpha Channel Usage**:  
  - Glow effects (lava, telegraphs for boss attacks, tutorial path highlights).  
- **Player Light**: a small dynamic sample taken at the player’s position.  
- **Sub-Tile Lighting**: Static lights near walls can affect partial floor tiles for smoother gradients.  

---

## Limitations
- Dynamic occlusion not supported (too expensive for multiple projectiles).  
- Lightmap is limited to dungeon grid resolution (though oversampled ×4).  
- Outdoor levels use a different lighting system (not covered here).  

---

## Future Considerations
- Explore hybrid occlusion for *single* dynamic sources (e.g., big boss attack).  
- Investigate GPU-side light stamping for higher performance.  
- Add secondary wall-only lightmap for more accurate vertical lighting.  
