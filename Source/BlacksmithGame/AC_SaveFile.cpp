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
// Standard bitshift operator serialization overloading
FArchive& operator<<(FArchive& Ar, FSystemSaveRegistration& Record) {
	Ar << Record.SlotName;
	Ar << Record.SaveDate;
	Ar << Record.SaveTime;
	return Ar;
}
//================================================================================================
// UAC_SaveFile CPP
//================================================================================================
// Fucntion, Constructor
//------------------------------------------------------------------------------------------------
// Sets default values for this component's properties
UAC_SaveFile::UAC_SaveFile() {
	PrimaryComponentTick.bCanEverTick = false;
}
//================================================================================================
// Function, Event Begin Play
//------------------------------------------------------------------------------------------------
// Called when the game starts
void UAC_SaveFile::BeginPlay() {
	Super::BeginPlay();
}
//================================================================================================
// Other Functions
//================================================================================================
// Function, Save Files
//------------------------------------------------------------------------------------------------
bool UAC_SaveFile::SaveFiles(const TArray<FSystemSaveRegistration>& TargetRecords) {
	// Load the System_Save_Registry.bin file into a master array to merge with the incoming data
	TArray<FSystemSaveRegistration> MasterRegistry;
	LoadFiles(MasterRegistry);
	//Loop through the incoming records you want to save
	for (const FSystemSaveRegistration& IncomingRecord : TargetRecords)
	{
		bool bFoundAndUpdated = false;
		// Check if this slot already exists in our master registry
		for (int32 i = 0; i < MasterRegistry.Num(); ++i)
		{
			if (MasterRegistry[i].SlotName.Equals(IncomingRecord.SlotName, ESearchCase::IgnoreCase))
			{
				// If match is found then update the existing record with the new data. 
				MasterRegistry[i].SaveDate = IncomingRecord.SaveDate;
				MasterRegistry[i].SaveTime = IncomingRecord.SaveTime;
				MasterRegistry[i].ImagePath = IncomingRecord.ImagePath;
				bFoundAndUpdated = true;
				break;
			}
		}
		// If it's a brand new slot, append it
		if (!bFoundAndUpdated)
		{
			MasterRegistry.Add(IncomingRecord);
		}
	}
	//  Now stream the fully merged master list back-to-back to disk
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
bool UAC_SaveFile::LoadFiles(TArray<FSystemSaveRegistration>& OutRecords) {
	FString SavePath = FPaths::ProjectSavedDir() / TEXT("SaveGames/System_Save_Registry.bin");
	TArray<uint8> RawBinaryData;
	// Clear the output array to ensure it's empty before loading new data
	OutRecords.Empty();
	// Load the raw byte data from disk into a temporary array. If this fails, return false to indicate loading failure.
	if (!FFileHelper::LoadFileToArray(RawBinaryData, *SavePath)) return false;
	if (RawBinaryData.Num() == 0) return true;
	// Open Memmory Reader
	FMemoryReader BinaryReader(RawBinaryData, true);
	// Setup the reader to match the endianness and serialization settings of the writer
	int32 StoredCount = 0;
	BinaryReader << StoredCount;
	// Use the platform file manager to check for the existence of each image path as we load, correcting or emptying the path if the file is missing
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	// Loop through the binary data and deserialize each struct record back into the output array
	for (int32 i = 0; i < StoredCount; ++i)
	{
		FSystemSaveRegistration TempRecord;
		BinaryReader << TempRecord; // Automatically pulls SlotName, SaveDate, SaveTime, and Saved ImagePath
		// DOUBLE CHECK: If the image was moved or renamed, make sure it's actually on disk
		if (!PlatformFile.FileExists(*TempRecord.ImagePath))
		{
			// Failsafe fallback checking your dynamic export folder layout
			FString FallbackPath = FPaths::ProjectSavedDir() / TEXT("SaveGames/") + TempRecord.SlotName + TEXT("_IMG.jpg");
			// If the expected image file is missing, check the most likely fallback location before giving up and emptying the path
			if (PlatformFile.FileExists(*FallbackPath))
			{
				TempRecord.ImagePath = FallbackPath;
			}
			else
			{
				TempRecord.ImagePath = TEXT("");
			}
		}
		// Add the fully validated record to the output array
		OutRecords.Add(TempRecord);
	}
	return true;
}
//================================================================================================
// Function, Delete Record
//------------------------------------------------------------------------------------------------
bool UAC_SaveFile::DeleteRecord(const FSystemSaveRegistration& RecordToDelete) {
	// Guard against empty input data
	if (RecordToDelete.SlotName.IsEmpty()) return false;
	// Load the active master registry array from disk
	TArray<FSystemSaveRegistration> MasterRegistry;
	LoadFiles(MasterRegistry);
	bool bFoundAndRemoved = false;
	// Loop through the system register to find the record matching our input struct
	for (int32 i = 0; i < MasterRegistry.Num(); ++i)
	{
		if (MasterRegistry[i].SlotName.Equals(RecordToDelete.SlotName, ESearchCase::IgnoreCase))
		{
			// TARGET FOUND: Construct the image path dynamically: SlotName + "_IMG.jpg"
			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			FString SaveGamesDir = FPaths::ProjectSavedDir() / TEXT("SaveGames/");
			FString CompanionImagePath = SaveGamesDir / MasterRegistry[i].SlotName + TEXT("_IMG.jpg");
			// Delete the physical screenshot image file from the directory
			if (PlatformFile.FileExists(*CompanionImagePath))
			{
				PlatformFile.DeleteFile(*CompanionImagePath);
			}
			// Remove the full struct element from the registry array container
			MasterRegistry.RemoveAt(i);
			bFoundAndRemoved = true;
			break; // Break immediately since the target is removed
		}
	}
	// If the registry didn't contain this file, exit without writing to disk
	if (!bFoundAndRemoved) return false;
	// COMMIT THE UPDATED REGISTRY DATA BACK TO DISK
	FString RegistryPath = FPaths::ProjectSavedDir() / TEXT("SaveGames/System_Save_Registry.bin");
	FBufferArchive BinaryArchive;
	int32 EntryCount = MasterRegistry.Num();
	BinaryArchive << EntryCount;
	// Loop through the updated master registry and stream each record back-to-back to the binary archive using our custom operator
	for (int32 i = 0; i < EntryCount; ++i)
	{
		FSystemSaveRegistration RecordCopy = MasterRegistry[i];
		BinaryArchive << RecordCopy;
	}
	// Commit the safe raw byte array back to disk, returning true if successful
	return FFileHelper::SaveArrayToFile(BinaryArchive, *RegistryPath);
}
//================================================================================================
// End of UAC_SaveFile CPP
//================================================================================================