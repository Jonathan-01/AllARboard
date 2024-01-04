// Fill out your copyright notice in the Description page of Project Settings.


#include "PlaceableActor.h"

// Sets default values
APlaceableActor::APlaceableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);


	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(SceneComponent);
}

// Called when the game starts or when spawned
void APlaceableActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APlaceableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Making sure the actor remains on the ARPin that has been found.
	if(PinComponent)
	{
		auto TrackingState = PinComponent->GetTrackingState();
		
		switch (TrackingState)
		{
		case EARTrackingState::Tracking:
			SceneComponent->SetVisibility(true);
			SetActorTransform(PinComponent->GetLocalToWorldTransform());

			// Scale down default cube mesh - Change this for your applications.
			SetActorScale3D(FVector(0.2f, 0.2f, 0.2f));
			break;

		case EARTrackingState::NotTracking:
			PinComponent = nullptr;

			break;

		}
	}
}

void APlaceableActor::CheckNearbyBuildings(const float MaxDistToActor, const bool AddToTracker = false)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("Checking Buildings!"));
	TArray<AActor*, FDefaultAllocator> AllBuildings;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Static"), AllBuildings);

	TArray<AStaticActor*, FDefaultAllocator> NearbyBuildings;

	for (int i = 0; i < AllBuildings.Num(); i++)
	{
		double dist = FVector::Distance(StaticMeshComponent->GetComponentLocation(), Cast<AStaticActor>(AllBuildings[i])->StaticMeshComponent->GetComponentLocation());
		if (dist < MaxDistToActor) 
		{
			NearbyBuildings.Add(Cast<AStaticActor>(AllBuildings[i]));
		}
	}

	if (AddToTracker) 
	{
		for (int i = 0; i < NearbyBuildings.Num(); i++) 
		{
			if (NearbyBuildings[i]->ActorHasTag("House")) 
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("House"));
				NearbyHouses.Add(NearbyBuildings[i]);
			}
			else if (NearbyBuildings[i]->ActorHasTag("Workplace")) 
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("Workplace"));
				NearbyWorkplaces.Add(NearbyBuildings[i]);
				NearbyBuildings[i]->ReduceTimer(20);
			}
		}
	}
}

void APlaceableActor::CheckNearbyStations(const float MaxDistToActor, const bool AddToTracker = false)
{
}

