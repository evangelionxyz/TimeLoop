// Copyright (c) 2026 Team Ficus

using UnrealBuildTool;
using System.Collections.Generic;

public class TimeLoopEditorTarget : TargetRules
{
	public TimeLoopEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;

		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.AddRange( new string[] { "TimeLoop" } );
	}
}
