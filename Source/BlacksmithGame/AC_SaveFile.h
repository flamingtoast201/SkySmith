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
#include "AC_SaveFile.generated.h"

// DECLARATION FIX: Tell the compiler this operator exists before reading the class functions

// My Libraries Added
//================================================================================================
// Define Struct for Saving Files
//================================================================================================
// Define the struct in C++ so BOTH systems know exactly what it is
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
class BLACKSMITHGAME_API UAC_SaveFile : public UActorComponent
{
	GENERATED_BODY()
public:	
	//============================================================================================
	// Constructors
	//============================================================================================
	// Sets default values for this component's properties
	UAC_SaveFile();
	//============================================================================================
	// Event Tick
	//============================================================================================
	// Called every frame
	
	//============================================================================================
	// Other Public Functions
	//============================================================================================
	// Explicit parameters mean zero thunk macros, zero wildcards, and zero crashes
	// Now we can use our own clean struct array directly! No wildcards, no thunks, no crashes.
	UFUNCTION(BlueprintCallable, Category = "Save Recovery System")
	bool SaveFiles(const TArray<FSystemSaveRegistration>& TargetRecords);

	UFUNCTION(BlueprintCallable, Category = "Save Recovery System")
	bool LoadFiles(TArray<FSystemSaveRegistration>& OutRecords);

	// Updated to intake the target struct directly from Blueprints
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
	// Called every frame


	//============================================================================================
};
//================================================================================================
// End of UAC_SaveFile Header
//================================================================================================