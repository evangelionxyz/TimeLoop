// Copyright (c) 2026 Team Ficus
#pragma once

#include "CoreMinimal.h"
#include "MayaState.generated.h"

/// ====== PROLOGUE ======
/// 1. Ethan walked up to Maya's room (Room 101)
/// 2. Maya open the door
/// 3. Maya talk to Ethan
/// 4. Ethan give Maya the Birthday Gift
/// 5. Maya moving to drawing

/// ====== DISASTER BEGIN ======
/// 1. Ethan back to his room (Room 201)
/// 2. Trying to relax to sofa
/// 3. Then earthquake is coming
/// 4. Maya starts screaming/crying
/// 4. Panicking
/// 5. Blackout - Maya still screaming/crying
/// 6. Find flashlight
/// 7. Trying to approach Maya's room
/// 8. Rooftop falling block Ethan's way (Stairs)

/// ====== PUZZLES BEGIN ======
/// NOTE: (60 seconds before Maya cannot be saved)
/// 1. Ethan need to find other ways to go to save Maya
/// 2. Find emergency stairs but the room is locked
/// 3. Find the key
/// 4. Got the key
/// 5. The Apartment hall also got blocked by other blockers
/// 6. Need to move all the blocker
/// 7. Finally arrived at Maya's room
/// 8. But Maya's room's rooftop starts to falling
/// 9. Maya scared
/// 10. Maya seeking for a shelter
/// 11. Her shelter broken after rooftop fallen on top of it
/// 12. She is trying to seeking for other shelter but too far
/// 13. Ethan need to bring the shelter
/// 14. Maya will be Saved or Crushed

UENUM(BlueprintType)
enum class EMayaState : uint8
{
	Inactive            UMETA(DisplayName = "Inactive"),

	// Prologue Phase
	MovingToEthan       UMETA(DisplayName = "Moving To Ethan"),
	TalkToEthan         UMETA(DisplayName  = "Talk To Ethan"),
	MovingToDrawing     UMETA(DisplayName = "Moving To Drawing"),
	Drawing             UMETA(DisplayName = "Drawing"),

	// Disaster / Quake Reaction
	Startled            UMETA(DisplayName = "Startled (Initial Tremor)"),
	Panicked            UMETA(DisplayName = "Panicked (Running on To Find Safe Place)"),
	Crying              UMETA(DisplayName = "Crying (Crying on Floor)"),

	// Puzzle / Loop Resolution
	SeekingShelter      UMETA(DisplayName = "Seeking Shelter (Running)"),
	Sheltered           UMETA(DisplayName = "Sheltered (Under Desk)"),

	// Terminal States
	Crushed             UMETA(DisplayName = "Crushed"),
	Saved               UMETA(DisplayName = "Saved"),
};

