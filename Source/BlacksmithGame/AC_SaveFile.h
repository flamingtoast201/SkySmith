// Fill out your copyright notice in the Description page of Project Settings.
//================================================================================================
// Include Only Once
//================================================================================================
#pragma once
//================================================================================================
// Libraries to Include
//================================================================================================
// Engine Required Libraries
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "HAL/PlatformFileManager.h"
#include "AC_SaveFile.generated.h"
// Libraries Added for Save System

//================================================================================================
// Define Struct for Saving Files
//================================================================================================
// Define the struct
USTRUCT(BlueprintType)
struct FSystemSaveRegistration
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Save Data")
	FString SlotName;

	UPROPERTY(BlueprintReadWrite, Category = "Save Data")
	FString SaveDate;

	UPROPERTY(BlueprintReadWrite, Category = "Save Data")
	FString SaveTime;
	// THE FIX: Add the image path string directly to the data contract!
	UPROPERTY(BlueprintReadWrite, Category = "Save Data")
	FString ImagePath;
};
//================================================================================================
// Class Declaration, UAC_SaveFile
//================================================================================================
// Child Class of public UActorComponent
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLACKSMITHGAME_API UAC_SaveFile : public UActorComponent {
	GENERATED_BODY()
public:	
	//============================================================================================
	// Constructors
	//============================================================================================
	UAC_SaveFile();
	//============================================================================================
	// Event Tick
	//============================================================================================
	// Called every frame
	// No event tick. Event driven
	//============================================================================================
	// Other Public Functions
	//============================================================================================
	// Save Files
	UFUNCTION(BlueprintCallable, Category = "Save Recovery System")
	bool SaveFiles(const TArray<FSystemSaveRegistration>& TargetRecords);
	// Load Files
	UFUNCTION(BlueprintCallable, Category = "Save Recovery System")
	bool LoadFiles(TArray<FSystemSaveRegistration>& OutRecords);
	// Delete Record
	UFUNCTION(BlueprintCallable, Category = "Save Recovery System")
	bool DeleteRecord(const FSystemSaveRegistration& RecordToDelete);
	//============================================================================================
protected:
	//============================================================================================
	// Protected Declarations
	//============================================================================================
	// Called when the game starts
	virtual void BeginPlay() override;
	//============================================================================================
private:	
	//============================================================================================
	// Private Declarations
	//============================================================================================
	void EncryptBuffer(TArray<uint8>& InOutBytes);
	void DecryptBuffer(TArray<uint8>& InOutBytes);
	//============================================================================================
};
//================================================================================================
// End of UAC_SaveFile Header
//================================================================================================