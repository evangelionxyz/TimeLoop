// Copyright (c) 2026 Team Ficus

using UnrealBuildTool;
using System.Collections.Generic;

public class TimeLoopTarget : TargetRules
{
	public TimeLoopTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;

		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.AddRange( new string[] { "TimeLoop" } );
	}
}
