# Survival Horror Project

A demo-length survival horror game built in Unreal Engine 5 with C++, developed as a
Computer Science capstone project and delivered in eight weeks by a three-person team.
Players navigate a zombie-infested mansion during a thunderstorm, solving nine
environmental puzzles to collect key items and escape. Combat features directional hit
reactions with per-limb damage tracking, locational blood masking, and ragdoll physics.

As project lead, I scoped and implemented the core game framework, all combat and AI
systems, cinematic sequences for narrative transitions, and the reusable interaction
and inventory components, while coordinating task delegation across the team.

<p align="center">
  <a href="https://downloads.andrewcreekmore.dev/MansionEscape98_v1.6.7z">Download (Windows)</a>
  &nbsp;&nbsp;|&nbsp;&nbsp;
  <a href="https://andrewcreekmore.dev/projects/game/survival-horror-project">Portfolio</a>
</p>

<div align="center">
  <a href="https://github.com/andrewcreekmore/SurvivalHorrorProject/assets/44483269/7dc4b376-669d-4ba1-91a9-2ddf7807e834"><img src="https://github.com/andrewcreekmore/SurvivalHorrorProject/assets/44483269/7dc4b376-669d-4ba1-91a9-2ddf7807e834" width="24%" alt="Exploration" /></a>
  <a href="https://github.com/andrewcreekmore/SurvivalHorrorProject/assets/44483269/317cb87c-848a-4cd6-8610-3d93d8eb86c4"><img src="https://github.com/andrewcreekmore/SurvivalHorrorProject/assets/44483269/317cb87c-848a-4cd6-8610-3d93d8eb86c4" width="24%" alt="Combat" /></a>
  <a href="https://github.com/andrewcreekmore/SurvivalHorrorProject/assets/44483269/52b18dfd-d4e0-4d0c-bc6d-39a24851f9e1"><img src="https://github.com/andrewcreekmore/SurvivalHorrorProject/assets/44483269/52b18dfd-d4e0-4d0c-bc6d-39a24851f9e1" width="24%" alt="Inventory" /></a>
  <a href="https://andrewcreekmore.dev/projects/survival-horror-project/screenshot-7.webp"><img src="https://andrewcreekmore.dev/projects/survival-horror-project/screenshot-7.webp" width="24%" alt="Map system" /></a>
</div>

## Details

*Players explore a storm-battered mansion, solving environmental puzzles across multiple
floors, managing limited ammunition and healing items, and fighting through zombies to
collect key items and escape.*

- Nine environmental puzzles with container-based item validation gating progression
  through the mansion
- Directional hit reactions with per-limb damage tracking, locational blood masking on
  enemy models, and ragdoll death physics
- Incapacitation system: sufficient leg damage transitions enemies from standing to
  crawling with modified movement speed, attack patterns, and ground-level alignment
- Hitscan weapon system with bone-specific damage modifiers, crosshair spread affected
  by movement, and curve-driven recoil
- Four zombie base types with randomized clothing and hair meshes generating visual
  variety across encounters
- Component-based interaction with raycast detection, timed hold-to-interact, and
  delegate-driven events reused across doors, containers, pickups, and puzzle elements
- Progression-updated map system revealing newly explored areas and floors as puzzles
  are solved

## Technology

- **Engine:** Unreal Engine 5
- **Languages:** C++
- **Visual Scripting:** Blueprint
- **Version Control:** Perforce
- **Project Management:** Jira

<p align="center"><a href="#survival-horror-project">↑ Back to top</a></p>
