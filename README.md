# TimeLoop
### Theme: “You Are Already Dead”

Developed with Unreal Engine 5

## Concept
The narrative tension built into a 10 to 30-second loop where self-preservation is impossible, but self-sacrifice changes someone else’s fate, creating an immediate, heavy emotional core. Combined with a PS1-era low-poly aesthetic, low-fidelity audio, and crisp UE5 lighting/fog, this could feel delightfully unsettling yet profoundly tragic.

Breakdown of core narrative thematic frameworks, level/puzzle design ideas, and mechanics tailored to UE5.

## 1. Narrative & Plot Frameworks
To make overarching story land, the loops need a connection. Why are you experiencing these memories?

- **The Archival Soul (Overarching Narrative)**:
You aren’t a ghost wandering the physical world; you are an echo being processed in a cosmic or technological “limbo.” You were a selfless person in life (or someone seeking redemption in death). You are condemned/chosen to relive the final seconds of tragedy across different eras and lives. Your goal isn’t to escape your death, but to clear the “Unresolved grief” of the people left behind.

- **The Final Twist (End Game)**:
In the final level, you relive your own actual death. All previous levels were memories of souls you interacted with or people you impacted. The person you have saved in the final 30-second loop is the one who goes on to remember you, closing the loop.

## 2. Level & Puzzles Concepts
Puzzles in a 10-30 seconds window must rely on environmental deduction across repeated loops.
- Loop 1-3: Discovery (Understanding how you die and who is in danger).
- Loop 4-6: Execution (Optimizing pathing and interacting with key objects in sequence).

### Level 1: “The Collapse” (10 seconds)
- **Setting**: A decaying 1990s apartment building during an earthquake/structural failure.
- **The Fatal Event**: At t=10s, the ceiling beam crushes you completely.
- **The Target**: A small child playing with blocks on the far side of the room.
- **The Puzzles**:
    - You cannot carry the child out in time - the door is blocked.
    - Loop 1: You run to the child, but the beam falls directly on both of you at t=10s.
    - Loop 2: You notice a heavy steel bathtub/desk nearby.
    - Solution: You sprint to the child, push them under the reinforced desk/tub, and stand in front of block falling debris. The building collapses; you die, but the child survives in the pocket of space you created

### Level 2: “The Crosshairs” (15 seconds)
- **Setting**: A rain-slicked city alleyway outside a theater.
- **The Fatal Event**: At t=15s, a sniper’s high-caliber round fires from a rooftop, killing you instantly.
- **The Target**: A musician exiting the stage door, unaware of the laser sight creeping up their chest.
- **The Puzzles**:
    - You cannot reach the sniper or warn the target through the thick glass/rain in time.
    - Solution: You must interact with the environment to alter the line of sight or redirect the bullet path. You knock over a heavy reflective billboard/metal shutter, or you deliberately step directly into the sniper’s line of sight at t=14.9s, taking the bullet intended for them.

### Level 3: “Toxins in the Glass” (30 seconds)
- **Setting**: A high-end 1920s banquet hall.
The Fatal Event: At t=30s, a poisoned wine toast takes effect. You drink from your glass and collapse.
- **The Target**: Your spouse/friend sitting across the table, whose glass is also poisoned by an assassin.
- **The Puzzles**:
    - You cannot reach the sniper or warn the target through the thick glass/rain in time.
    - Loop 1: You try to spill their drink, but the guard stops you.
    - Loop 2: You realize there is only one unpoisoned glass on the head waiter’s tray across the room.
    - Solution: You hurry to swap your target’s poisoned glass with the safe one on the tray. You have no time to swap your own. At t=30s, everyone toasts. You drink the poison and slump over, seeing your companion drink safely as the screen fades.

## 3. Core Mechanics & Low-Poly Horror Visuals
### Dynamic Loop Systems
- **Persistent Knowledge, Reset State**: The player’s position and physical items reset every loop, but unlocked codes, memorized routes, and environmental cues stay with the player’s mind.

- **The “Ghosting” Effect**: On loop resets, faint low-poly wireframes or translucent silhouettes of your previous failed attempts could briefly show your past paths, helping players optimize their route.


### UE5 Aesthetic Setup
- **Retro Shading & Lighting**: Use Unreal Engine 5’s Post-Process Materials to apply a PS1/Retro Resolution Downscaler alongside pixel jitter/affine texture mapping. Combined with modern Lumen lighting or sharp local light sources, high-contrast shadows against low-poly meshes creates an eerie atmosphere.

- **Audio Cues Countdown**: Avoid a heavy UI timer taking up the screen. Instead use ambient audio cues to signal time running out:
    - 10s remaining: Heartbeat audio begins, speeding up.
    - 3s remaining: Muffled audio, color desaturation/tunnel vision.
    - 0s: Hard jump to black cut, instantly resetting to the starting frame.
