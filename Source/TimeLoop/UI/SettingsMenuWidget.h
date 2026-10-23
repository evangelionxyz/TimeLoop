// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameUserSettings.h"
#include "SettingsMenuWidget.generated.h"

// Delegate ini dipanggil kalau setting berhasil di-apply & disimpan,
// biar WBP bisa nampilin notifikasi semacam "Settings Applied" kalau perlu.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingsApplied);

UENUM()
enum ETimeLoopScalabilityLevel : int
{
	Low = 0,
	Medium,
	High,
	Epic,
};

// USettingsMenuWidget
// C++ base class untuk menu Settings.
// Semua logic resolution & screen mode ditaruh di sini, jadi WBP_SettingsMenu
// tinggal binding UI (ComboBox, Button, dll) ke fungsi-fungsi BlueprintCallable di bawah.
UCLASS()
class TIMELOOP_API USettingsMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Settings")
	void PopulateSettings();

public:

	// -1: Custom
	// Scalability
	UFUNCTION(BlueprintPure, Category = "TimeLoop|Settings")
	ETimeLoopScalabilityLevel GetCurrentScalabilityLevel() const;

	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Settings")
	void SetScalabilityLevel(ETimeLoopScalabilityLevel NewScalabilityLevel);

	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Settings")
	void CycleScalabilityLevel(const bool bIncrement);

	UFUNCTION(BlueprintPure, Category = "TimeLoop|Settings")
	FString GetCurrentScalabilityLevelLabel() const;

	// Ambil resolusi yang lagi dipakai sekarang
	UFUNCTION(BlueprintPure, Category = "TimeLoop|Settings")
	FIntPoint GetCurrentResolution() const;

	// Ubah resolusi (belum langsung ke-apply, harus panggil ApplySettings dulu)
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Settings")
	void SetResolution(FIntPoint NewResolution) const;

	UFUNCTION(BlueprintPure, Category = "TimeLoop|Settings")
	FString GetCurrentResolutionLabel() const;

	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Settings")
	void CycleResolution(const bool bIncrement);

	// Index screen mode yang lagi aktif sekarang (0 = Fullscreen, 1 = Windowed Fullscreen, 2 = Windowed)
	UFUNCTION(BlueprintPure, Category = "TimeLoop|Settings")
	EWindowMode::Type GetCurrentWindowMode() const;

	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Settings")
	void CycleWindowMode(const bool bIncrement);

	// List nama screen mode, urutannya harus sama kayak index di atas. Diisi ke ComboBox/Dropdown di WBP.
	UFUNCTION(BlueprintPure, Category = "TimeLoop|Settings")
	FString GetCurrentWindowModeLabel() const;

	// Cek apakah V-Sync lagi nyala atau nggak
	UFUNCTION(BlueprintPure, Category = "TimeLoop|Settings")
	bool IsVSyncEnabled() const;

	// Nyalain/matiin V-Sync (belum langsung ke-apply, harus panggil ApplySettings dulu)
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Settings")
	void SetVSyncEnabled(bool bEnabled) const;

	UFUNCTION(BlueprintPure, Category = "TimeLoop|Settings")
	FString GetCurrentVSyncLabel() const;

	// Ambil brightness saat ini, dalam skala 0-100 (gampang ditampilin di UI)
	UFUNCTION(BlueprintPure, Category = "TimeLoop|Settings")
	float GetCurrentBrightness() const;

	// Ubah brightness (belum langsung ke-apply, harus panggil ApplySettings dulu)
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Settings")
	void SetBrightness(float NewBrightness) const;

	// Geser brightness naik/turun sejumlah BrightnessStep, dipanggil dari tombol < >
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Settings")
	void CycleBrightness(const bool bIncrement);

	// Label buat ditampilin di UI, misal "80%"
	UFUNCTION(BlueprintPure, Category = "TimeLoop|Settings")
	FString GetCurrentBrightnessLabel() const;

	// Simpan & apply semua perubahan settings. Panggil ini pas user klik tombol "Apply".
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Settings")
	void ApplySettings();

	// Batalin perubahan yang belum di-apply, balikin ke settings terakhir yang tersimpan di disk.
	// Cocok dipanggil pas widget ini ditutup tanpa nge-Apply, atau pas tombol "Cancel" diklik.
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Settings")
	void RevertSettings();

	// Broadcast tiap kali ApplySettings berhasil dipanggil, dengerin ini kalau mau nampilin feedback ke user
	UPROPERTY(BlueprintAssignable, Category = "TimeLoop|Settings|Events")
	FOnSettingsApplied OnSettingsApplied;

protected:
	// Cache GameUserSettings biar nggak manggil GetGameUserSettings() berulang-ulang tiap fungsi dipanggil
	UPROPERTY(Transient)
	TObjectPtr<UGameUserSettings> CachedSettings;

	TArray<FIntPoint> Resolutions;

	// Step kenaikan/penurunan brightness tiap klik < atau >, dalam skala 0-100
	UPROPERTY(EditDefaultsOnly, Category = "TimeLoop|Settings")
	float BrightnessStep = 10.0f;

	// Batas gamma asli Unreal yang dipetakan ke skala 0-100 UI
	const float MinGamma = 1.0f;
	const float MaxGamma = 3.4f;
};