// Copyright (c) 2026 Team Ficus

#include "SettingsMenuWidget.h"
#include "Kismet/KismetSystemLibrary.h"

void USettingsMenuWidget::PopulateSettings()
{
	// Ambil instance GameUserSettings sekali aja pas widget ini dibuat
	// (GetGameUserSettings ada di class UGameUserSettings itu sendiri, bukan UGameplayStatics)
	CachedSettings = UGameUserSettings::GetGameUserSettings();
	CachedSettings->LoadSettings(false);

	// Populate supported resolutions
	if (!UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions))
	{
		Resolutions =
		{
			FIntPoint(1280, 720),
			FIntPoint(1366, 768),
			FIntPoint(1600, 900),
			FIntPoint(1920, 1080),
			FIntPoint(2560, 1440),
			FIntPoint(3840, 2160),
		};
	}
}
ETimeLoopScalabilityLevel USettingsMenuWidget::GetCurrentScalabilityLevel() const
{
	return static_cast<ETimeLoopScalabilityLevel>(CachedSettings->GetOverallScalabilityLevel());
}

void USettingsMenuWidget::SetScalabilityLevel(ETimeLoopScalabilityLevel NewScalabilityLevel)
{
	// -1: Custom
	CachedSettings->SetOverallScalabilityLevel(NewScalabilityLevel);
}

void USettingsMenuWidget::CycleScalabilityLevel(const bool bIncrement)
{
	// -1: Custom
	int32_t Scalability = CachedSettings->GetOverallScalabilityLevel();
	Scalability = FMath::Clamp(bIncrement ? Scalability + 1 : Scalability - 1, 0, static_cast<int>(Epic));
	CachedSettings->SetOverallScalabilityLevel(Scalability);
}

FString USettingsMenuWidget::GetCurrentScalabilityLevelLabel() const
{
	switch (static_cast<ETimeLoopScalabilityLevel>(CachedSettings->GetOverallScalabilityLevel()))
	{
		case Low: return TEXT("Low");
		case Medium: return TEXT("Medium");
		case High: return TEXT("High");
		case Epic: return TEXT("Epic");
		default: return TEXT("Custom");
	}
}

FIntPoint USettingsMenuWidget::GetCurrentResolution() const
{
	return CachedSettings->GetScreenResolution();
}

void USettingsMenuWidget::SetResolution(const FIntPoint NewResolution) const
{
	CachedSettings->SetScreenResolution(NewResolution);
}

FString USettingsMenuWidget::GetCurrentResolutionLabel() const
{
	const FIntPoint &Resolution = CachedSettings->GetScreenResolution();
	return FString::Printf(TEXT("%d x %d"), Resolution.X, Resolution.Y);
}

void USettingsMenuWidget::CycleResolution(const bool bIncrement)
{
	int CurrentResolutionIndex = Resolutions.Find(CachedSettings->GetScreenResolution());
	CurrentResolutionIndex = FMath::Clamp(bIncrement ? CurrentResolutionIndex + 1 : CurrentResolutionIndex - 1, 0, Resolutions.Num() - 1);
	CachedSettings->SetScreenResolution(Resolutions[CurrentResolutionIndex]);
}

EWindowMode::Type USettingsMenuWidget::GetCurrentWindowMode() const
{
	return CachedSettings->GetFullscreenMode();
}

void USettingsMenuWidget::CycleWindowMode(const bool bIncrement)
{
	int CurrentScreenMode = CachedSettings->GetFullscreenMode();
	CurrentScreenMode = FMath::Clamp(bIncrement ? CurrentScreenMode + 1 : CurrentScreenMode - 1, 0, static_cast<int32>(EWindowMode::Windowed));
	CachedSettings->SetFullscreenMode(static_cast<EWindowMode::Type>(CurrentScreenMode));
}

FString USettingsMenuWidget::GetCurrentWindowModeLabel() const
{
	switch (CachedSettings->GetFullscreenMode())
	{
		case EWindowMode::Type::Fullscreen: return TEXT("Fullscreen");
		case EWindowMode::Type::WindowedFullscreen: return TEXT("Windowed Fullscreen");
		case EWindowMode::Type::Windowed: return TEXT("Windowed");
		default: return TEXT("Unknown");
	}
}

bool USettingsMenuWidget::IsVSyncEnabled() const
{
	return CachedSettings->IsVSyncEnabled();
}

void USettingsMenuWidget::SetVSyncEnabled(const bool bEnabled) const
{
	CachedSettings->SetVSyncEnabled(bEnabled);
}

FString USettingsMenuWidget::GetCurrentVSyncLabel() const
{
	return CachedSettings->IsVSyncEnabled() ? TEXT("ON") : TEXT("OFF");
}

void USettingsMenuWidget::ApplySettings()
{
	CachedSettings->ApplySettings(false);
	CachedSettings->SaveSettings();

	if (OnSettingsApplied.IsBound())
	{
		OnSettingsApplied.Broadcast();
	}

	UE_LOG(LogTemp, Log, TEXT("SettingsMenuWidget: Settings applied and saved."));
}

void USettingsMenuWidget::RevertSettings()
{
	CachedSettings->LoadSettings(true);
}
