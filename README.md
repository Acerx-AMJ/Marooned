# Marooned

**Marooned** is a peaceful and atmospheric 3D island exploration game built with C++ and Raylib. 

![Gameplay Screenshot](assets/Screenshot1.png)
---

## 🌴 Features

- 🏝️ **Procedural Terrain**  
  Generate vast island landscapes from 4K perlin noise heightmaps.

- 🌤️ **Dynamic Skybox**  
  A seamless, procedurally rendered sky with soft, animated clouds.

- 🌊 **Animated Water Shader**  
  Realistic water distortion with distance-based gradients for added depth.

- 🌲 **Vegetation System**  
  Procedurally placed palm trees with randomized scale, rotation, and offsets.  
  Tree placement is filtered by terrain height and spacing rules.

- 🌫️ **Ambient Occlusion Shader**  
  Screen-space AO gives a subtle cell-shaded look to island edges.

- 🧭 **Free Camera Navigation**  
  Navigate freely over the landscape or plan for a 3rd-person boat cam.

---

## 🚧 Planned Features

- 🚣 First-person boat controller  
- 🌿 Additional vegetation types like bushes or grass (billboards)  
- 🧍 First-person on-foot controller  
- 🗺️ Item collection, crafting, and progression  
- 🔮 Mysterious story elements and environmental clues

---

## 🛠 Built With

- [Raylib](https://www.raylib.com/) - Simple and powerful C/C++ game framework  
- C++17 - All game logic and rendering code written from scratch  
- GLSL - Custom shaders for water, lighting, sky, and post-processing  

---

## 🎮 Running the Game

To build and run:

```bash
make
./game.exe
