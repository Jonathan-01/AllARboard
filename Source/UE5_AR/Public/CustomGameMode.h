// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Components/Widget.h"
#include "PlayAreaPlane.h"
#include "HelloARManager.h"
#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "StaticActor.h"
#include "PlaceableActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "CustomGameMode.generated.h"
//Forward Declarations
class APlaceableActor;

/**
 * 
 */

UCLASS()
class UE5_AR_API ACustomGameMode : public AGameModeBase
{
	GENERATED_BODY()
private:

	FTimerHandle Ticker;
	APlaceableActor* SpawnedActor;

	bool bPlaneSpawned = false;
	int workplaceSpawnTracker = 0;
	TArray<APlayAreaPlane*> spawnedPlanes;

public:
	ACustomGameMode();
	virtual ~ACustomGameMode() = default;

	virtual void StartPlay() override;
	/**
	 * @brief An example of an event as generally seen within the Event Graph of Blueprints
	 *  You can have different implementations of this event which will be called when you call this as a delegate.
	 *	See the CPP For more information on the implementation
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GameModeBase", DisplayName = "Start Play")
		void StartPlayEvent();

	UFUNCTION(BlueprintCallable, Category = "Score")
		int32 GetScore();

	UFUNCTION(BlueprintCallable, Category="Score")
		void SetScore(const int32 NewScore);

	UFUNCTION(BlueprintCallable, Category="Score")
		void IncrementScore(const int32 ScoreToAdd);
	
	UFUNCTION(BlueprintCallable, Category = "Tracking")
		virtual StateEnum GetStateEnum();
	
	UFUNCTION(BlueprintCallable, Category = "Tracking")
		virtual void SetStateEnum(const StateEnum state);
	
	UFUNCTION(BlueprintCallable, Category = "ARSetup")
		virtual void ResetPlanesSpawned();
	
	UFUNCTION(BlueprintCallable, Category = "ARSetup")
		virtual void AddPlane();
	
	UFUNCTION(BlueprintCallable, Category = "ARSetup")
		virtual void RemovePlane(); 
	
	UFUNCTION(BlueprintCallable, Category = "ARSetup")
		virtual bool CheckPlanes(); 
	
	UFUNCTION(BlueprintCallable, Category = "ARSetup")
		virtual void HidePlanes(); 
	
	virtual void Tick(float DeltaSeconds) override;

	virtual void SpawnBuildings();

	/**
	 * @brief Perform a line trace at the screen position provided and place a default Placeable actor on it.
	 * Create another actor class that inherits from APlaceableActor and is spawned here instead of the default one. Make sure this new object is placed on the Plane rather than halfway through a plane
	 * @param ScreenPos Pass the Screen Position as an FVector
	 */
	virtual void LineTraceSpawnStation(FVector ScreenPos);
	virtual void LineTraceSpawnPlane(FVector2D ScreenPos);
	

	/**
	 * @brief This function is virtual - you can inherit this class to override this function
	 * Each level can have their own unique spawned actors but with similar base qualities using inheritance
	 */
	virtual void SpawnInitialActors();

	 UPROPERTY(Category="Placeable",EditAnywhere,BlueprintReadWrite)
	 TSubclassOf<AActor> HouseToSpawn;

	 UPROPERTY(Category="Placeable",EditAnywhere,BlueprintReadWrite)
	 TSubclassOf<AActor> StationToSpawn;

	 UPROPERTY(Category="Placeable",EditAnywhere,BlueprintReadWrite)
	 TSubclassOf<AActor> WorkplaceToSpawn;
	 
	 UPROPERTY(Category="Placeable",EditAnywhere)
	 AHelloARManager* ARManager;

	 UPROPERTY(Category="AR Actors", EditAnywhere, BlueprintReadWrite)
	 APlayAreaPlane* PlayArea;

	 UPROPERTY(Category = "CustomARClasses", EditAnywhere, BlueprintReadWrite)
	 UMaterialInterface* TransMat;

	 UPROPERTY(Category="UI Elements", EditAnywhere, BlueprintReadWrite)
	 TSubclassOf<UUserWidget> UIWidget;
};
