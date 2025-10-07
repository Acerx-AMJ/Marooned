Marooned on an island full of dinosaurs 06/18/2025

Set in the 1700s in the Caribbean. Some one marooned you on purpose. You lost your memory? 
Trapped on an island lost to time. Find a treasure chest with a gun in it. Add a sword at some point. 
Need a way of making structures like lost temple ruins. caves. Huts. villagers? hostile locals? Go full Turok? 
Need to keep the scope small enough to finish. It's already hugely complicated. 

Level switching. Change heightmaps from a menu, destroy all objects, regenerate vegetation, and raptors. A menu screen that shows before the level loads? For development it makes sense to be able to swap heightmaps before and during runtime. 

Terrain has a hard limit to how big it can be. 4k textures scaled to 16k. sounds like a lot but everything else is scaled big. Need serpenteen paths that loop around. Need a way to block player besides water. invisible walls where thick jungle is. Create bounding boxes around trees. make areas where trees are too dense to pass. 

invisible wall at the edge of water plane. make a boundary at 30k radius from center for all maps?  

create bounding boxes around trees that collide with the player. and raptors? x

Bridges.

Treasure chest model. Use the mimic from fools path. It's fully rigged and animated. it would be good to know how to do that with raylib. 

Day/Night cycle?

foot steps x

Player Take Damage x

raptors should make more noise. when exiting idle maybe.  x

positional sound - volume attenuation only. raylib can't do 3d audio. or it can but I would have to rewrite sound manager. which I should do anyway. 

raptor agro vision should be higher, sounds heard from further away as well. 

make a handleHeightmap.h or something where we can switch between heightmaps. 

made level select menu.

made dungeon generator. need a way to clear the dungeon, when switching levels. x

dungeons are generated from a 32x32 png image. Where black pixels are walls and white pixels are floor tiles. Red is skeletons, blue is barrels, green is starting position, yellow is light source. 

Need a way to make doors work. Need some kind of barrier that can be removed. making the door arch model fit and be rotated right sounds like a nightmare. 

-Doorways are purple pixels. DoorWays spawn doors which are centered on the doorway arch. Doors can also lead to other levels. Teal pixels are exits that lead to the previous level. Orange pixels are next level doors that lead to the next level. Terrain generated levels have dungeonEnterance struct that spawns a archway with a door that leads to dungeon1. We keep a vector of entrances in level data so we can have multiple entrances to different dungeons from the same over world. 


runaway time for raptors. x 

added skeletons + pathfinding. Simple BFS with smoothPath function. Skeletons have vision by marching through the pixel map. it's not very reliable, but raylib doesn't have built in 3D raycasts. I would need to implement it myself. maybe later.
-Improved ray marching to be mostly reliable. Skeletons now have a alertNearbySkeleton() that looks for other skele's within vision and sets them to chase when they gain visibility of the player, or if they take damage. 

separated raptor AI and skeleton AI into there own functions. So they each have there own switch. Raptors are now only for outdoor use. and skeletons are for dungeons only. I tried raptors in dungeon but their run away behavior messed everything up.
So raptors follow the heightmap and skeletons follow the dungeon Image. 

Add a chest. 

game pad inputs for shoot/swing/block/jump/run/swap weapon

add simple inventory. 4 boxes in the bottom left. It's a map of strings and amounts. so you can have multiple potions in 1 inventory slot. We should make another consumable to justify this type of inventory. like food? maybe you can have multiple keys. and there's just generic keys that opens locked doors and are used up. lock picks? lock picking mini game? 

limit number of health potions you can have? nah. 

Mana potions? Maybe player can throw fireballs. 

stealth? We have working vision cones. sort of. We could make them cones. then draw the cones to show the player the enemies sight. Lean into the grid based nature of the game maybe. If we could highlight floor tiles with a shader we could highlight vision that way. I just want the ability to turn a specific floor tile red. But since it's .glb with baked in texture I can't easily do that. and I never figured out how to apply a texture to some mesh. Everything is just solid colors with shaders or baked in texture of the dungeon. Maybe just make a cube and try to apply a texture to it. a box, a breakable box?

We have multiple models of the barrel. One of which is the barrel smashed. We could give the barrels health and when you hit them with a sword we swap in the broken model. barrels should be like 30 percent bigger. they don't line up with the walls. I want barrels in the corners of the dungeon but since the wall is in the middle of the tile there a gap. could push walls in somehow. 

Doors have a member variable isLocked...implemented locked doors. Locked doors are Aqua colored pixels. The doorway gets set to locked, then the door sets isLocked to dw.isLocked. 

Implemented keys and updated inventory. Keys are consumables. They open locked doors. You can carry keys between levels. We can do some puzzle type stuff with this later. Keys are billboard just like health pot. need to increase the size, just for keys. 
if (key.type == CollectableType::Key) scale = 100

teleporters, fast travel, Ride a big boat to a new over world map. 

Maybe you need to find things to repair your damaged boat. Wood planks, cloth for a sail, rudder, supplies
There is a model of a ship wreck right off the coast of the starting island. swim out to it and collect stuff to build a raft. The goal is to get off the island. We could show the outline of a raft model that slowly gets filled in, as you find more things. 

dungeon 1-1 would be middle islands first dungeon, middle island could have 2 more dungeons. dungeon 1-2, and 1-3, after 1-3 you have collected all the supplies for the raft and sail to the next overworld where you enter dungeon 2-1. islands could have different flora and fauna. 

Take a stab at UI art. Pirate themed, wood with gold trim. Steam punk health bar? Health and stam should be on the bottom, as to not blend in with sky. Could make a console type interface. 

pirates should stay tinted red on death

fill out the rest of map 7.  Big room with just 3 pirates. L shaped room around starting corner. A door on either end. both lead to a locked door. There is a maze between the 2 doors where you find the key. So you might end up on the other side of the map or vise versa depending on where you start. Make the path of least resistance the longest. 

Dungeon 3 should exit back up to surface. on the little island in the back. make another entrance you can get to dungeon 3 by boat or swimming to the other island. 

make dungeon 4 and 5 and put an entrance on one of the side islands. left 4 5 6, right 7 8 9. center 1 2 3 then switch to a different island chain. second island chain should be fat perlin noise. 

would the left island dungeons exit out on the right side? Maybe we could make small island exit and right exit locked from the outside. 

Gold. buys magic scrolls. Lightning, fireball, ice, barrels can contain gold, like diablo. 

what to do about webs? We need to draw webs as a flat quad, they can't be a billboard. but everything else is a billboard and needs to be sorted to not occlude each other. we can't put the flat quad into a vector of billboards. could we make a billboard that always faces a certain direction. 

Made a struct called drawRequest. and a vector of drawRequests. drawRequests contain all the data needed to draw an animated billboard, or flatquad, or decal, everything 2d in the game. Then we sort the vector of drawRequests based on distance to camera, and draw in that order. We have functions called GatherEnemies, GatherDecals ect. that copy the data from the character class to the drawReqest. In the final draw function we have a switch case that draws either flat quads or billboards depending on drawRequestType, an enum class in transparentDraw.h this and transparentDraw.cpp contains all the code to make this happen. we just call gatherAllbillboards, then draw the requests. -this solves all the occlusion problems. 
-doors are not included in this. but it doesn't seem to matter. they don't occlude anything. Should probably add draw flat door to the render pipeline anyway. 

I never explained tint based lighting. Since the dungeons are constructed out of floor tiles and wall segments, we can light each one individually by changing the tint property of the model. We can iterate all the walls and measure the distance to a light source or sources, and change the tint brighter or darker the further away. This makes a really low res lighting system. We can even do dynamic lights by just attaching a light source to a moving object like bullet, and then check for it each frame.


Added line of sight to tint based lighting. each wall segment checks distance to light source AND does a line of sight check. That way light doesn't bleed through walls. Could add double walls to stop light bleeding to other side of lit wall. Player has simpler light source without raycasts. It looks better that way. -Started using worldLOS check instead of dungeon map. 3d raycasts better spread the light if I give it an epsilon. where the ray goes further than it should so it actualy hits the walls. Needed to do this for floor ceiling doors and walls. The ray was stopping at the colliders i guess. not making it to wall.position. 

Changed pirates vision to be just a worldLOS check, instead of both dungeon map and world ray cast. They can now see through open doors better and notice the player when he's around a corner but still in eye sight. Probably can do this for skeleton as well. 

Make a fireball type in bullet. 

bullets get occluded by billboards x fixed. bullets are 3d, draw them before transparent 2d stuff. 

started on a revolver, but Now i'm not too sure I want one. finding a good model was impossible, and it wouldn't fit the pirate era theme. Then what else? Another slow firing black powder weapon is too similar to the blunderbuss. A magic staff? that could shoot magic projectiles. maybe you have a book in your left hand and a staff in your right. You do some dark magic rituals to cast spells. 

Added magic staff, and a bullet parameter fireball. fireballs are just big yellow spheres with a light source attached. Need to apply a shader with a fire texture. maybe work on 3d particle system. Maybe fireball bullets have gravity and fly in an arc until they hit the ground and explode. like a grenade launcher! grenade.h? no it would be a bullet. 
-added arcing to fireballs. still need to apply a shader with a moving texture. 
-firballs need to explode when they hit the ground. Explosions would require particles no? we could do a 3d model of an explosion quake 2 style. both particles and model would be ideal. 

3d particle system. It was easy enough in 2d. Try it in 3d for blood and smoke. Your limited with just raylib. particles is one thing that you can do. so you should do it. 

Added fireballs and particles. Fireball is a model I made in blender with a fire texture attached. it slowly rotates when fired. when it hits the ground or an enemy it spawns a animated explosion decal. While the fireball is in the air it emits dark grey smoke particles and when it explodes it emits orange spark particles. I also add blood particles that get emitted from bullets when they hit a non skeleton enemy. -possibly do half orange half yellow particles on explosion, might look like confetti. 

Add blood particles to sword hits. Would need to add emitter to enemy. spawn blood on take damage. Removes need for spawn blood in bullet. -Moved blood emitter to character. Emit blood on take damage if your not a skeleton. 

make staff not available until you pick it up? Where do you pick it up? Level 4? make it a color on the dungeon map and spawn the model in world at that position. make it a collectable? It's lame if you have it from the beginning. for now just make a hasStaff member variable. then you can only switch to it if you hasStaff. 

-Made an std::vector of weaponType, we can than % vector.size() to scroll through weapons. No matter how many weapons you may have unlocked. We simply add the weaponType to the vector when you pick up the weapon. 

         player.currentWeaponIndex = (player.currentWeaponIndex + 1) % player.collectedWeapons.size();
        activeWeapon = player.collectedWeapons[player.currentWeaponIndex];

sword is bloody when starting in dungeon for mysterious reasons. 

Use dinoBite for spider attack sound. current one is weird sounding. 

make another release with the staff sitting on little island. Maybe we could take a picture of the staff and show a billboard of the staff instead of instancing the 3d model. that seems like more work than just draw model. We would need to worry about sorting if it were a billboard. The model should be scaled correctly to look good at a distance. It would have to be a struct though. we can't just have some one off model floating around, it needs  to be a collectable...it can't be a collectable. It needs to be a one off thing. What if you needed to pick up the blunderbuss or sword. We should have in world pickupable weapons. Start with nothing. you already have the vector of weapon types. What would you call this class? CollectableWeapon.
-made CollectableWeapon class. Uses existing WeaponType enum. instance and add to worldWeapons vector. update and draw the 3d models hovering and spinning. For now start with blunderbuss and sword. find the staff on level 4. 

Staff isn't too overpowered. Should maybe slow the fire rate. Self damage seems good, you can accidently hurt yourself with fireballs if too close, but it's not that much damage, 50 max that falls off with distance. Should the staff have different projectiles? How could we do that? FireFireball(), FireIceball(), FireLightning(). Freezing enemies and them hitting them with the sword shaders them. erase the sprite and spawn ice particles, lots of them with high gravity. Ice ball would emit blue particles when in the air instead of smoke. Ice explosion on hit. Could reuse existing explosion just add an ice flag and change the color. 

Implement mana. Player member var. Mana bar. maxMana, currentMana. animate the mana going down. Mana potions are spawn from barrels, and can be spawned in world with a dark blue pixel maybe. Shooting a fireball only costs like 10 mana. shoot 10 times before you need more mana. May need to make fireballs more powerful if they are going to be limited. 
-implemented mana. 10 mana to cast fireball maybe to low. Barrels can drop mana potions 10 percent of the time. 


Make the light range super far but the intensity low. it blends things out wayyyy better. Well way better for fireball anyway. 
I got it a little bit smoother. 

the gun should bob up and down while you walk. the sword should swing up and down. the staff should do something as well when moving. -added weapon bob to all weapons. bobbing every 2 steps looks most like a how a video game should be. x

make staff shoot slower. - slowed from 0.5 to 1.0

chest open animation seems way too slow. x

bullets now kill skeles in one hit. they seems to be penetrating them. how is that possible? we are early returning? -fixed but I dont remember what was wrong. 

Improved bullet light code.

Character has rotationY var that gets set when moving. We never use this. Why would we need rotationY on character? bullet gets direction from player.position - position. billboards always face camera. Could we use it for something in the future? 

make skeletons attach slightly faster so they actually hit each swing not every other. -feels much better should have fixed this ages ago. They attack slightly faster but way more consistent. Attack anim was 4 frames, 0.2 frame time = 0.8 attack cool down. 


added magic icon for fire and ice. T switches between magic spells when holding the magic staff. 

Refactored Resources.h/cpp to ResourceManager.h/cpp. We no longer have a global variable for every resource. We just look up the resource in a map, by string key when we need it. This allows us to unload all resources automatically, and makes adding a new resource one line of code instead of like 3 in 3 different places. It replaces about 100 global vars. 

Refactored main.cpp. Moved everything out of main.cpp into world. Things in world.cpp control the overWorld map, and some global functions that need to get called. DungeonGeneration.h handles all things dungeon. 

Consider moving your render pipeline out of main and into it's own file. x

Fireball traps. Iceball traps

make a controls menu page. 

Moved all the rendering code to render_pipline.cpp. It's getting harder to remember where I put things. 

Made camera_system.h/cpp. changed camera to a singelton camera = CameraSystem::Get().Active(), we could have a cinematic camera in addition to player/freecam. Imagine cut scenes where we animate the camera flying over the terrain and focusing on an objective or something. 

NPC: friendly Character. An old hermit again? A friendly skeleton. Friendly pirate. Talking animal, like a parrot, or monkey. 

dont generate trees around starting area. x

Go through each function and erase parameters we no longer need to pass. 


sharks in the water. 

enemy ghost. x -Uses skeletonAI. maybe it should shoot projectiles. or at least some kind of animation when it attacks, a decal? 
-maybe it could siphon health to regain it's own health, it slowly turn more and more invisible when it's moving and not attacking. On take damage it would become fully opaque and when attacking it would be fully visible when not doing these things it would fade out alpha. 

Added trapezoid UI bars for health, mana, and stamina. We lerp the values to smoothly animate them. Health bar color lerps toward red when player is hit. -another idea was pill shaped health bars. Maybe that would fit the theme better, it would be less code than constructing trapezoids. 

Made many changes to lighting. Shortened lightsource range for floors and lengthened the wall range. looks better. added a "saturation" variable. Fixed light.intensity to actually reflect changes to the struct. It was stuck at 1.0f when I thought I was changing it. 

Enemy stamina? 

Added fireball launcher model and got it spawning on vermilion colored pixels. Mark down vermilion color on legend. How will it work? Make a new map. One with lots firball traps. Long hallways with launchers set to different timings between launches. Already have a LauncherTrap struct. Needs timing vars on creation. How would we set timings on creation? 
void GenerateLauncherTraps, takes level data? Because what if we want multiple firetrap levels, it would have to be level data. 

Me and chat GPT came up with a ingenious encoding method for fireball traps. There is a vermilion colored pixel which is the trap itself. A yellow direction pixel that is an orthogonal neighbor to the trap that marks the direction the trap shoots, and a darker yellow timing pixel that can be one of 3 different shades of yellow. darkest yellow = 3 second delay, then 2 second and brightest is 1 second delay. The timing pixel is always on the opposite side of the direction pixel, that way it is easier to look up and it looks like a 3x1 line of red and yellow pixels that look like little launchers on the map. it took 2 attempts to make it work but LauncherTrap struct now contains all the info needed to launch fireballs. - Consider a blueish pixel for ice traps. use the same direction and timing colors. 

spider death animation not running. x - we never implemented a case for spiders. 

only enter free cam if in debug mode. ~ x -debugInfo must be true for tab button to work. 

Character patrol not working -(if !isWalkable) continue; was set to if (isWalkable) continue. So if it found a path it just skipped it. This was effect all characters. Wonder how that ! got erased. 

I tried desaturating the wall model texture and floor model texture. Making it neutral gray and trying to color it with just the tint based lighting. Color would only be applied to models within the light range so unlit tiles would be a flat grey that looked bad. I tried giving the wall textures a blue gray tint, but in the end it looked best with the default wall texture which is slightly reddish purple, almost maroon....marooned. I changed the "warm tint" for floor tiles to be even more yellow which mixes with the default blueish color of the floor tile to make a pretty neutral gray that works and is a slightly different color than the walls so it's not just one uniform color like before with the gray dungeon. I though about making my own wall models. 

Apparently we can do per pixel lighting with a projected light map and a shader. Say a texture twice the size of the the 32x32 pixel image. Would have lighting twice the resolution we have currently. why not a texture 10x as big? The baked lighting pass would "fill up" this texture map with light contribution values. I guess according to the larger light map we would light each pixel based on the light contribution values we saved while baking. Light contribution values come from the 3d raycast from light sources. Would we keep per model lighting for dynamic lights? because raycasting to every tile then calculating the contribution every frame. It would only need to be 400 pixels distance. Wait... we ray cast to each model. This gives 1 value. maybe we wouldn't rely on the raycast. all the info would be baked into the lightmap image. Need to ask some clarifying questions when the internet comes back. Maybe a transparent to white gradiant starting from the center of the yellow pixel. How would you do LOS lighting. actually paint in the lighting pixel by pixel. At a higher resolution to get sub model size lighting.
Need an example of a projected light map. Need internet...

internet was backup for a minute there. Apparently we generate the light map with code. but it would only work for the XZ plane. we could make a second lightmap for walls ZY plane by doing some math on the first lightmap to generate the second i guess. Lightmap doesn't need to be a texture, just an array that is filled with light values. say a 128x128 array of vec3? would be 4 times better resolution than per model. That means 1 tile would be lit down to 1/4 of the tile. perfect for the mini floor tiles. We would light the pixels of a 200x200 tile. We might need to do something fancy to light the mini tiles. 

Could we make wall tiles occlude the light for floor tiles? iterate the light map and skip anything beyond the range of the light. then iterate the 2nd lightmap for walls. skip tiles without raymarched line of sight? 

Implemented floor and ceiling lighting based on 128x128 generated light map. A shader reads the lightmap and colors the models per pixel. 4x the resolution we had before. Still working on getting dynamic fireball lights to look good. Stil over saturates the tiles and turns them green. Next we can try doing the XY for walls instead of XZ

Tried to make dynamic shader lights. made another light map for dynamic lights that gets checked everyframe. and updates the shader. If we try to combine baked light with dynamic light it fucks itself. 

-Try having only dynamic light map. just add static lights to it then we'd be reading off of 1 map not trying to combine. We were only baking lights because we were doing world LOS checks but we aren't doing that for floors right now anyway. 

-Tried and succeeded so far. For now we use a single dynamic lightmap, we load both dynamic lights and static lights and update it every frame. We aren't doing LOS checks on floors so it's fine and runs fast. LLM was saying we could do dynamic occlusion stamping or some such. We would iterate every light source and do a LOS check for walls and only stamp the light on places that aren't occluded. 

-we now run the lighting with occlusion check once for static lights. We save those values and copy them to the dynamic light map every frame, no need to recalculate every frame for static lights. Dynamic fireball lights just don't have occlusion because it would be too slow. Maybe it would be slow you should try it and see. it might give cool effects. it took 3 days and we tried like 3 different approaches. What worked was first just adding worldLOS checks to the stamplight function. It was super slow, so then we tried to optimize. but it didn't get much better. Then I realized how we could bake the lighting and still use the dynamic map. We run the LOS checks once and save the light values to a seprate GStaticBase vector of colors i think. and then when we run the dynamic lighting we copy gstaticbase to gDynamic so it has all the static lights before stamping the dynamic ones. I also made a lightsample at the players position so we can have shader light on the player. Looks way better than the tint based player light. 
-Consider sub tile lighting for static lights close to walls. 


When traveling to the next level by door, The next level's lighting is completely off, like dark. When you go to the menu and load the level from there it works. -made a hack solution where we quicly switch to menu when fading out. Then if in menu, if switchFromMenu = true we call init level and it loads correctly. My guess is that when we switch to menu, no other code runs but the menu code allowing us to cleaning switch light maps. I made another state called loading level. and switch to that instead of menu just to make things more clear and it doesn't work. The lights are broke, even though when loadingLevel no other code runs, just like menu state. 

-what is different about being in menu state that allows the light map to switch correctly? -we are drawing the menu, we are updating the menu, we are setting level index, It's not fadeout...we may never know. 

chests are broken. opening one chest means all other chests are open from then on. replaced all chests with keys until we can fix it somehow.

lava tiles with their own custom shader and light source attachment. 

implemented lava tiles. made a new floor tile in blender out of a cube matched to the size of the floor tile but 1/4 has high. Attached lava texture to model. sample lava texture somehow with shader and use world UV coords to makes the texture applied across all tile seamlessly. Had a bug where we were drawing a giant lava texture in the sky the tiled infinitely or at least to clipping plane. Just added a pixel discard to shader if world.y is over 500. Caused by world UVs and the world being very large. Still need to add lights to lava somehow. bloom doesn't pick it up. 

Spent two days trying to make the tiles above lava tiles glow red. Was successful in the end, but the effect is meh. Maybe the tiles around the lava should also glow red. But if it's gonna be meh, I don't want to try it. Anyway the lava glow can be used for more than just lava. We can now color an arbitrary tile any color we want. Our light map uses RGB for light color and occlusion, and uses A, the alpha channel to add glow to tiles above lava, or for example light a path along the floor. Maybe for tutorial reasons, or maybe a boss advertises his attacks and the floor lights up beneath you before an attack. It would take more work to do it dynamically. 

I think we need a better way to get floor tiles, we get what tiles are lava by making a lava map. Shouldn't there be an easier way? We already have a map. 

It's always sort of looked like the enemies were floating above the dungeon floor. I gave them a shadow model with a shader and it improved it a little, I needed to move all the characters down like 20 units so it looked like they were on the floor, but instead I just moved the dungeon floor up to 100, instead of 80. Luckily everything is tied to that one parameter floorHeight. So now the enemies look like they are properly grounded and with shadows to sell it more. It also made the player height shorter. Which I think is better because pirates were too short, this makes player more at eye level with them.

Implemented lava pits. Before we were faking it by dropping the camera when you stepped in lava making it looked like you took one step down. I lowered the lava tile height to -200. So there is a 1 tile drop down into the pit. This made a black void in between the floor tiles and the lower lava tiles. So using the lava mask we make a "skirt" by spawning walls on the edge of lava tiles. the walls are dropped by 400 so they cover the void. The next problem was adding collision to the subterranean walls. We ended up using the old makeWallBoundingBox along with a lot more rigamarole to get them positioned correctly. but Now everything lines up perfect and it looks great. There is an extra 50 units you can stand on near the edge of the pit where the skirts are but the bounding check makes the player stay at the right height perfectly. Once you fall into the pit you need to jump to get back out. Which is what I was going for. 
-I spoke too soon. We don't use MakeWallBoundingBox any more. It was 90 degrees of for differently shaped pits. We now generate the bounding boxes from the wall skirt once we place them. Had to flip the x and z when generating for some reason but it now all lines up. 

lava still doesn't glow. Edit the post process shader to make bloom pickup the lava. maybe a separate Glow shader. 

Consider making deeper "death" pits. 

-clean up dungeonGeneration.cpp, it's loaded with static inline funcs and vars defined right about the funcs that use them. very sloppy. Limit dungeonGeneration.cpp to just the generation of dungeons. We have 2 main places that control the game. DG.cpp and world.cpp. World was supposed to control the over world and dungeonGeneration was supposed to control the dungeon. Oh and main.cpp which "updates context", consider moving all these update functions to world, updateContext func that just calls all the other funcs. 

A sky box for dungeons? Night sky, stars, moon, maybe add on to current skybox shader, could we do a star field with just a shader? -implemented skybox shader night time branch. if is dungeon generate noise and only show the very tops of it. makes it look like lots of points of light. 

-added tone map to bloom shader. turned contrast down to 0. we now add contrast via ACES tone map plus 1.0 exposure. 

make fireball emitter a sphere not a point for fireball trail. x

bullets should hit lava not stop at floor above it. x

make UI bars centered for all resolutions. 

When exiting dungeon to over world the dungeon entrances are colored black. 

other island entrances now start locked and unlock after dungeon 3. 

If your going to make a demo, you need a way to communicate the controls to the player. How would you do this? Do it like you did the last few times. text pop ups before each new thing. Start with "WASD TO MOVE", than once they move "Left click to attack" "Q to switch weapons" Then do a controls page in the menu later. Ok how would we do popup text? 

inside UI.h make Class Popup.it has a position on screen, a width and height, a std::string "message". Maybe a popup could have some kind of state. Like showing first message. Maybe a table of messages.
-we have a vector of messages inside a class hintManager.h/cpp We instance hints in UI, setup tutorial text, then iterate the vector of std::strings depending on the players action. We update the messages inside hintManager because all we need is the player or input keys. Should probably be in UI.cpp. It's called UpdateTutorial and could be put anywhere I'm just afraid it's going to be hard to find later. 

Do we need any more popups? The staff. The interact popup should popup when you are close to the dungeon entrance. 

Test other styles of font, don't just take the first one. 

38 days left. What is it missing? 

How hard would it be to shrink the world? Very. We sized the tiles to 200 on a whim. and the floor tiles are x700 before that. Could I make like a global size factor. 

make a new island. make a portal out the the archway with a shader that animates a multicolored door. The portal could be at the end of the last dungeon and your teleported to a new island. Make a boss. 

09/25/2025

Fixed tree shadows. We now stamp the shadow onto the terrain with a shader. So shadow quads are no longer floating in the air sometimes.

More sounds, sound of bullets hitting wall. jump sound effect. ghost sound effects. enemy hurt sound effect should be positional. 

River level was crashing because there were no entrances. over world level must have entrances. or account for this. 

Can we just apply the lighting shader to the launcher model and it will just work? Yup it just works. That means barrels and chests can be lit by the shader as well we don't need to tint them. -need to ad more dungeon props if we can light things so easily. 

maybe we could tint the gun and sword model with the same shader? it would probably make it too dark in dark areas. My guess is using our current tint method would work better. 

Take another stab at adding a texture to the grass some how. Can we apply a texture to pixels that are over a certian height on the heightmap? you can sample a texture with a shader surely. You tried it before and it wouldn't work. Maybe it was something to do with the vertex shaders UVs or something. 

-This time it worked. I think because of all the experience with lighting shader. 

Added fog for distant terrain. Then added treeShader which is applied to trees, bushes, entrances, and doors. So it should probably be called fog shader or something. It just blends the colors toward background sky color depending on distance. 

make swimming work. -added swim sound effect, works the same as foosteps but for water. 

Make ghost slowly turn invisible when not attacking. It should start and remain visible until player enters vision. then slowly fade out, intantly fade back in if attacking, then slowly fade out. Upon taking damage instantly fade back in. 

Commision an artist to redraw all the enemy sprites. 

Commision a musician for an ambient music track. 

























