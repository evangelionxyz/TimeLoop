// Copyright (c) 2026 Team Ficus

#include "SettingsMenuWidget.h"
#include "Kismet/KismetSystemLibrary.h"

void USettingsMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Ambil instance GameUserSettings sekali aja pas widget ini dibuat
	// (GetGameUserSettings ada di class UGameUserSettings itu sendiri, bukan UGameplayStatics)
	CachedSettings = UGameUserSettings::GetGameUserSettings();
}

TArray<FIntPoint> USettingsMenuWidget::GetSupportedResolutions() const
{
	TArray<FIntPoint> Resolutions;

	// Ambil daftar resolusi fullscreen yang didukung monitor
	// (fungsi ini ada di UKismetSystemLibrary, bukan UGameUserSettings)
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);

	// Fallback kalau ternyata kosong (misal driver monitor lagi aneh), kasih beberapa resolusi umum
	if (Resolutions.IsEmpty())
	{
		Resolutions = {
			FIntPoint(1280, 720),
			FIntPoint(1366, 768),
			FIntPoint(1600, 900),
			FIntPoint(1920, 1080),
			FIntPoint(2560, 1440),
		};
	}

	return Resolutions;
}

FIntPoint USettingsMenuWidget::GetCurrentResolution() const
{
	if (CachedSettings)
	{
		return CachedSettings->GetScreenResolution();
	}
	return FIntPoint(1280, 720);
}

void USettingsMenuWidget::SetResolution(FIntPoint NewResolution)
{
	if (CachedSettings)
	{
		CachedSettings->SetScreenResolution(NewResolution);
	}
}

FString USettingsMenuWidget::ResolutionToText(FIntPoint Resolution) const
{
	return FString::Printf(TEXT("%d x %d"), Resolution.X, Resolution.Y);
}

int32 USettingsMenuWidget::GetCurrentScreenModeIndex() const
{
	if (CachedSettings)
	{
		return WindowModeToIndex(CachedSettings->GetFullscreenMode());
	}
	return EWindowMode::Windowed;
}

void USettingsMenuWidget::SetScreenModeByIndex(int32 NewModeIndex)
{
	if (CachedSettings)
	{
		CachedSettings->SetFullscreenMode(IndexToWindowMode(NewModeIndex));
	}
}

TArray<FString> USettingsMenuWidget::GetScreenModeOptions() const
{
	// Urutan di sini WAJIB sama persis kayak urutan di IndexToWindowMode / WindowModeToIndex
	return {
		TEXT("Fullscreen"),
		TEXT("Windowed Fullscreen"),
		TEXT("Windowed")
	};
}

bool USettingsMenuWidget::IsVSyncEnabled() const
{
	if (CachedSettings)
	{
		return CachedSettings->IsVSyncEnabled();
	}
	return false;
}

void USettingsMenuWidget::SetVSyncEnabled(bool bEnabled)
{
	if (CachedSettings)
	{
		CachedSettings->SetVSyncEnabled(bEnabled);
	}
}

void USettingsMenuWidget::ApplySettings()
{
	if (!CachedSettings)
	{
		return;
	}

	// ApplySettings nge-terapin perubahan resolution & screen mode ke window game-nya
	CachedSettings->ApplySettings(false);

	// SaveSettings nyimpen ke GameUserSettings.ini, biar kepake lagi pas game dibuka ulang
	CachedSettings->SaveSettings();

	if (OnSettingsApplied.IsBound())
	{
		OnSettingsApplied.Broadcast();
	}

	UE_LOG(LogTemp, Log, TEXT("SettingsMenuWidget: Settings applied and saved."));
}

void USettingsMenuWidget::RevertSettings()
{
	if (CachedSettings)
	{
		// Load ulang dari GameUserSettings.ini, buang semua perubahan yang belum di-apply
		CachedSettings->LoadSettings(true);
	}
}

EWindowMode::Type USettingsMenuWidget::IndexToWindowMode(int32 Index)
{
	switch (Index)
	{
	case 0: return EWindowMode::Fullscreen;
	case 1: return EWindowMode::WindowedFullscreen;
	case 2: return EWindowMode::Windowed;
	default: return EWindowMode::Windowed;
	}
}

int32 USettingsMenuWidget::WindowModeToIndex(EWindowMode::Type Mode)
{
	switch (Mode)
	{
	case EWindowMode::Fullscreen: return 0;
	case EWindowMode::WindowedFullscreen: return 1;
	case EWindowMode::Windowed: return 2;
	default: return 2;
	}
}
