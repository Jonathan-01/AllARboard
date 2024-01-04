// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "PlayAreaPlane.h"
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AREmulaterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class UE5_AR_API AAREmulaterGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	AAREmulaterGameMode();

	UPROPERTY(Category = "AREmulation", EditAnywhere, BlueprintReadWrite)
		APlayAreaPlane* PlayArea;

	bool bPlaneSpawned;
public:
	virtual void LineTraceSpawnActor(FVector ScreenPos);
};
