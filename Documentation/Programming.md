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

