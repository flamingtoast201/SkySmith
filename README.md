# Blacksmithing Simulator (Working Title)

# Copyright © 2026 Spicer Games - All Rights Reserved.

### **Project Status & Living Documentation**
This project is a work in progress simulation game built in Unreal Engine. The systems described below reflect the active, custom C++ and Blueprint architecture of the game. 

For inquiries or collaboration, contact:
* **Phone:** (620) 804-3083
* **Email:** alexander.spicer@hotmail.com | alexander.spicer201@gmail.com

---

## **Theme / Setting / Genre**
* **Genre:** Immersive Blacksmithing Simulation & Crafting Management
* **Setting:** A serene, high-altitude sanctuary set among a breathtaking **Sea of Clouds**. 
* **Tone:** Tactical, rewarding, and deeply atmospheric. Adventurers traversing this vertical world rely on light, high-mobility gear instead of heavy plate armor, creating a unique economic demand for light weaponry (daggers, longswords) and custom specialized tools.

---

## **Core Game Systems & Architecture**

### Core Game Loop
-> Purchase
-> Smelt
-> Craft
-> Sell
### Purchase
*You purchase supplies from a Dock Master stationed at your Sky Island which will be delivered over X amount of time.*
### Smelt
*You smelt supplies and raw materials into Ingots or Crafted Items.*
### Craft
*You can craft items into useful tools for selling or crafting.*
### Sell
*The game will have a shop which allows you to place items for sale that adventurers will pickup and then purchase from you.*

### The Interactive Anvil Minigame
A rhythmic, precision-based striking system that turns forging into an active skill:
* **Radial Spin Loop:** A timing-based minigame featuring a dynamic status bar and a high-reward "Sweet Spot".
* **The 4-State Execution Architecture:** Driven entirely by explicit variable references (no hardcoded numbers) to evaluate game states:
    * `SUCCESS`: Item successfully created and baked into inventory.
    * `PROGRESS`: Well-timed hits compound the status bar using blueprint class-default multipliers.
    * `MISSTRIKE`: Errant hits apply specific penalties pulled directly from class defaults.
    * `TIMEOUT`: Reaching the time limit triggers a "Poof" routine, cleanly consuming resources from the forge slots.

### Order-Independent Recipe Scouting
A highly decoupled workbench inventory pipeline:
* Players can throw raw resources into any of the 4 forge slots in any order.
* An asynchronous recipe evaluation loop scans the slots, matches ingredient totals, and seamlessly maps them to Blueprint Actor Class Defaults—safely bridging background data structures to visual inventory grids.

### High-Performance Stylized Graphics
* Designed around strict optimization guidelines to maintain a highly efficient **17%–22% GPU rendering profile**.
* Features an artistic low-poly aesthetic with lightweight depth layering, flat fog horizons, and diegetic, resolution-independent World Space UI (such as "Pop-Out" damage and progress floating combat text tailored for colorblind accessibility).

---

## **Technical Implementation Highlights**
* **Decoupled UI Passivity Tree:** Fully decoupled `WBP_Inventory` and slot interaction systems matching back-end data arrays via distinct active-border highlight overrides.
* **Custom Save File System:** Implements low-overhead bitshift operator serialization overloading via standard `FArchive` binaries, keeping player data processing highly performant and secure.
