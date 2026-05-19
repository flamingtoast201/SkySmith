// Fill out your copyright notice in the Description page of Project Settings.

//================================================================================================
// Libraries to Include
//================================================================================================
#include "AC_SaveFile.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "HAL/PlatformFileManager.h"
//================================================================================================
// Forward Declarations
//================================================================================================
// Standard bitshift operator serialization overloading for our custom C++ struct
FArchive& operator<<(FArchive& Ar, FSystemSaveRegistration& Record)
{
	Ar << Record.SlotName;
	Ar << Record.SaveDate;
	Ar << Record.SaveTime;
	return Ar;
}
//================================================================================================
// UAC_SaveFile CPP
//================================================================================================
// Sets default values for this component's properties
//------------------------------------------------------------------------------------------------
UAC_SaveFile::UAC_SaveFile()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}
//================================================================================================
// Event Begin Play
//================================================================================================
// Called when the game starts
void UAC_SaveFile::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}
//================================================================================================
// Other Functions
//================================================================================================
// Function, Save Files
//------------------------------------------------------------------------------------------------
bool UAC_SaveFile::SaveFiles(const TArray<FSystemSaveRegistration>& TargetRecords)
{
	// 1. Load the existing master registry from disk first so we don't blind-overwrite it
	TArray<FSystemSaveRegistration> MasterRegistry;
	LoadFiles(MasterRegistry);

	// 2. Loop through the incoming records you want to save
	for (const FSystemSaveRegistration& IncomingRecord : TargetRecords)
	{
		bool bFoundAndUpdated = false;

		// Check if this slot already exists in our master registry
		for (int32 i = 0; i < MasterRegistry.Num(); ++i)
		{
			if (MasterRegistry[i].SlotName.Equals(IncomingRecord.SlotName, ESearchCase::IgnoreCase))
			{
				// Match found! Update its metadata and image path with the fresh save data
				MasterRegistry[i].SaveDate = IncomingRecord.SaveDate;
				MasterRegistry[i].SaveTime = IncomingRecord.SaveTime;
				MasterRegistry[i].ImagePath = IncomingRecord.ImagePath;
				bFoundAndUpdated = true;
				break;
			}
		}

		// If it's a brand new slot (like adding DiscoKitty when only ASpicer existed), append it
		if (!bFoundAndUpdated)
		{
			MasterRegistry.Add(IncomingRecord);
		}
	}

	// 3. Now stream the fully merged master list back-to-back to disk
	FString SavePath = FPaths::ProjectSavedDir() / TEXT("SaveGames/System_Save_Registry.bin");
	FBufferArchive BinaryArchive;

	int32 EntryCount = MasterRegistry.Num();
	BinaryArchive << EntryCount;

	for (int32 i = 0; i < EntryCount; ++i)
	{
		// Stream the copy cleanly through our custom operator
		FSystemSaveRegistration RecordCopy = MasterRegistry[i];
		BinaryArchive << RecordCopy;
	}

	// Commit the safe raw byte array back to disk
	return FFileHelper::SaveArrayToFile(BinaryArchive, *SavePath);
}
//================================================================================================
// Function, Load Files
//------------------------------------------------------------------------------------------------
bool UAC_SaveFile::LoadFiles(TArray<FSystemSaveRegistration>& OutRecords)
{
	FString SavePath = FPaths::ProjectSavedDir() / TEXT("SaveGames/System_Save_Registry.bin");
	TArray<uint8> RawBinaryData;

	OutRecords.Empty();

	if (!FFileHelper::LoadFileToArray(RawBinaryData, *SavePath)) return false;
	if (RawBinaryData.Num() == 0) return true;

	FMemoryReader BinaryReader(RawBinaryData, true);

	int32 StoredCount = 0;
	BinaryReader << StoredCount;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	for (int32 i = 0; i < StoredCount; ++i)
	{
		FSystemSaveRegistration TempRecord;
		BinaryReader << TempRecord; // Automatically pulls SlotName, SaveDate, SaveTime, and Saved ImagePath

		// DOUBLE CHECK REAlITY: If the image was moved or renamed, make sure it's actually on disk
		if (!PlatformFile.FileExists(*TempRecord.ImagePath))
		{
			// Failsafe fallback checking your dynamic export folder layout
			FString FallbackPath = FPaths::ProjectSavedDir() / TEXT("SaveGames/") + TempRecord.SlotName + TEXT("_IMG.jpg");

			if (PlatformFile.FileExists(*FallbackPath))
			{
				TempRecord.ImagePath = FallbackPath;
			}
			else
			{
				TempRecord.ImagePath = TEXT(""); // Explicitly empty if missing completely so UI shows default card
			}
		}

		OutRecords.Add(TempRecord);
	}

	return true;
}
//================================================================================================
// Function, Delete Record
//------------------------------------------------------------------------------------------------
bool UAC_SaveFile::DeleteRecord(const FSystemSaveRegistration& RecordToDelete)
{
	// Guard against empty input data
	if (RecordToDelete.SlotName.IsEmpty()) return false;

	// 1. Load the active master registry array from disk
	TArray<FSystemSaveRegistration> MasterRegistry;
	LoadFiles(MasterRegistry);

	bool bFoundAndRemoved = false;

	// 2. Loop through the system register to find the record matching our input struct
	for (int32 i = 0; i < MasterRegistry.Num(); ++i)
	{
		if (MasterRegistry[i].SlotName.Equals(RecordToDelete.SlotName, ESearchCase::IgnoreCase))
		{
			// 3. TARGET FOUND: Construct the image path dynamically: SlotName + "_IMG.jpg"
			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			FString SaveGamesDir = FPaths::ProjectSavedDir() / TEXT("SaveGames/");
			FString CompanionImagePath = SaveGamesDir / MasterRegistry[i].SlotName + TEXT("_IMG.jpg");

			// Delete the physical screenshot image file from the directory
			if (PlatformFile.FileExists(*CompanionImagePath))
			{
				PlatformFile.DeleteFile(*CompanionImagePath);
			}

			// 4. Remove the full struct element from the registry array container
			MasterRegistry.RemoveAt(i);
			bFoundAndRemoved = true;
			break; // Break immediately since the target is removed
		}
	}

	// If the registry didn't contain this file, exit without writing to disk
	if (!bFoundAndRemoved) return false;

	// 5. COMMIT THE UPDATED REGISTRY DATA BACK TO DISK
	FString RegistryPath = FPaths::ProjectSavedDir() / TEXT("SaveGames/System_Save_Registry.bin");
	FBufferArchive BinaryArchive;

	int32 EntryCount = MasterRegistry.Num();
	BinaryArchive << EntryCount;

	for (int32 i = 0; i < EntryCount; ++i)
	{
		FSystemSaveRegistration RecordCopy = MasterRegistry[i];
		BinaryArchive << RecordCopy;
	}

	return FFileHelper::SaveArrayToFile(BinaryArchive, *RegistryPath);
}
//================================================================================================
// End of UAC_SaveFile CPP
//================================================================================================