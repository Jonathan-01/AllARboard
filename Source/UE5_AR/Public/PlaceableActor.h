// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <Kismet/GameplayStatics.h>
#include "ARPin.h"
#include "ARBlueprintLibrary.h"
#include "StaticActor.h"
#include "GameFramework/Actor.h"
#include "PlaceableActor.generated.h"

class UARPin;

UCLASS()
class UE5_AR_API APlaceableActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlaceableActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void CheckNearbyBuildings(const float MaxDistToActor, const bool AddToTracker);

	virtual void CheckNearbyStations(const float MaxDistToActor, const bool AddToTracker);

	UPROPERTY(Category = "myCategory", VisibleAnywhere, BlueprintReadWrite)
		USceneComponent* SceneComponent;

	UPROPERTY(Category = "myCategory", VisibleAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* StaticMeshComponent;

	UARPin* PinComponent;

	TArray<AActor*> NearbyHouses;
	TArray<AActor*> NearbyWorkplaces;
};
