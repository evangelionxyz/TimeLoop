// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameUserSettings.h"
#include "SettingsMenuWidget.generated.h"

// Delegate ini dipanggil kalau setting berhasil di-apply & disimpan,
// biar WBP bisa nampilin notifikasi semacam "Settings Applied" kalau perlu.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingsApplied);

// USettingsMenuWidget
// C++ base class untuk menu Settings.
// Semua logic resolution & screen mode ditaruh di sini, jadi WBP_SettingsMenu
// tinggal binding UI (ComboBox, Button, dll) ke fungsi-fungsi BlueprintCallable di bawah.
UCLASS()
class TIMELOOP_API USettingsMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// --- Resolution ---

	// Ambil semua resolusi layar yang didukung monitor, buat diisi ke ComboBox resolution
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Settings")
	TArray<FIntPoint> GetSupportedResolutions() const;

	// Ambil resolusi yang lagi dipakai sekarang
	UFUNCTION(BlueprintPure, Category = "TimeLoop|Settings")
	FIntPoint GetCurrentResolution() const;

	// Ubah resolusi (belum langsung ke-apply, harus panggil ApplySettings dulu)
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Settings")
	void SetResolution(FIntPoint NewResolution);

	// Helper buat nge-convert FIntPoint jadi text "1920 x 1080", biar gampang ditampilin di ComboBox
	UFUNCTION(BlueprintPure, Category = "TimeLoop|Settings")
	FString ResolutionToText(FIntPoint Resolution) const;

	// --- Screen Mode ---

	// Index screen mode yang lagi aktif sekarang (0 = Fullscreen, 1 = Windowed Fullscreen, 2 = Windowed)
	UFUNCTION(BlueprintPure, Category = "TimeLoop|Settings")
	int32 GetCurrentScreenModeIndex() const;

	// Ganti screen mode pakai index (0 = Fullscreen, 1 = Windowed Fullscreen, 2 = Windowed)
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Settings")
	void SetScreenModeByIndex(int32 NewModeIndex);

	// List nama screen mode, urutannya harus sama kayak index di atas. Diisi ke ComboBox/Dropdown di WBP.
	UFUNCTION(BlueprintPure, Category = "TimeLoop|Settings")
	TArray<FString> GetScreenModeOptions() const;

	// --- Apply & Revert ---

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

	// Helper convert index dropdown <-> EWindowMode
	static EWindowMode::Type IndexToWindowMode(int32 Index);
	static int32 WindowModeToIndex(EWindowMode::Type Mode);
};
