# Programming Documentation
This is a programming documentation for Time Loop Game Development in Unreal Engine 5. This contains types of Blueprints, Enums and How they connected to eachother.

# Blueprint - Interface
`BPI_Resetable` Interface for all the actor classes that can be resetted.
`BPI_Interactable` Interface for all the actor classes that can be interact with player.

# Blueprint - Class
`BP_LoopGameMode` Base GameMode for the Active Level. This contains the Game state logic.
```C++
// Functions:
StartLoop() // Initialize the Loop State and Timer
UpdateTimer() // Update the timer
TriggerLoopFailure() // Event for Failure
TriggerLoopSuccess() // Event for Success
ResetLoop() // Reset loop and trigger the ResetableObjects
RegisterResetables(Resetable Object) // Register Resetable Objects
```

`BP_LoopCharacter` Inherited from `BPI_Resetable`<br>
Main Player Character
```C++
// Events
BeginPlay() // Register itself to the GameMode using the RegisterResetables function
```

`BP_TargetNPC` Player target to interact

---
# C++ - The Collapse game flow (Prologue -> Earthquake -> Blackout)

| Class | Role |
|---|---|
| `ACollapseDirector` | Level-placed. Runs the prologue, quake, blackout and emergency phases. Place **one** per level. |
| `ACollapseTrigger` | Box trigger. Sends `EventId` (e.g. `Quake`, `SecondaryTremor`) to the director, optionally requiring an inventory item. |
| `UApartmentLightController` | Added at runtime to every apartment light. Modes: `Normal`, `Buzz`, `Flicker`, `Stutter`, `Off`. Spawns Niagara sparks. |
| `UFlashlightEffects` | Flashlight toggle, inventory gating, `FlickerFlashlight()`. |
| `ATimeLoopGameState` | `bInPrologue`, `StartPrologue()`, `EndPrologueAndInitiateLoop()`, `OnPrologueStateChanged`. |

Phases (`ECollapsePhase`): `Prologue` -> `InitialTremor` (3s) -> `StructuralRupture` (3s) -> blackout -> `Emergency`.

Actor tags used by the director (all configurable):

| Tag | On | Effect |
|---|---|---|
| `ApartmentLight` | any light actor | Controlled by the director (BP_ArpartmentLight is found by class automatically) |
| `BuzzLamp` | light actor | Buzzes during the prologue (Maya's room) |
| `StutterLamp` | light actor | Keeps stuttering after the blackout. If none is tagged, the lamp closest to the player is used |
| `QuakeProp` | physics actor | Jitters during the quake |
| `QuakeDust` | any actor | Extra dust spawn point |

Debug: in PIE open the console and run `ke * TriggerEarthquakeSequence`.

---
# Enums
`EEndReason` Types of End Reason
| Name | Description |
|---|---|
| **TimeExpired**| Player running out of time|
| **None** | Default state |

`ELoopState` Types of current gameplay state inside the `BP_LoopGameMode`
| Name | Description |
|---|---|
| **Inactive**   | Not Running/Reset |
| **Running**    | In Gameplay Update |
| **Failed**     | Failed - Need to Respan/Reset |
| **Succeedeed** | Succeedeed - Transition to the next Level|

