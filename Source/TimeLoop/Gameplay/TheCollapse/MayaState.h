// Copyright (c) 2026 Team Ficus
#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EMayaState : uint8
{
	Inactive             UMETA(DisplayName = "Inactive"),

	// Prologue Phase
	MovingToDoor        UMETA(DisplayName = "Moving To Door"),
	InteractingWithDoor UMETA(DisplayName = "Interacting With Door"),
	MovingToDrawing     UMETA(DisplayName = "Moving To Drawing"),
	Drawing             UMETA(DisplayName = "Drawing"),

	// Disaster / Quake Reaction
	Startled            UMETA(DisplayName = "Startled (Initial Tremor)"),
	Panicked            UMETA(DisplayName = "Panicked (Crying on Floor)"),

	// Puzzle / Loop Resolution
	SeekingShelter      UMETA(DisplayName = "Seeking Shelter (Running)"),
	Sheltered           UMETA(DisplayName = "Sheltered (Under Desk)"),

	// Terminal States
	Crushed             UMETA(DisplayName = "Crushed"),
	Saved               UMETA(DisplayName = "Saved"),
};