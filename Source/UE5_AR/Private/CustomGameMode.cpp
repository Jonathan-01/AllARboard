// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomGameMode.h"
#include "CustomARPawn.h"
#include "CustomGameState.h"
#include "CustomActor.h"
#include "HelloARManager.h"
#include "ARPin.h"
#include "ARBlueprintLibrary.h"
#include "PlayAreaPlane.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "PlaceableActor.h"

ACustomGameMode::ACustomGameMode():
	SpawnedActor(nullptr)
{
	// Add this line to your code if you wish to use the Tick() function
	PrimaryActorTick.bCanEverTick = true;

	// Set the default pawn and gamestate to be our custom pawn and gamestate programatically
	DefaultPawnClass = ACustomARPawn::StaticClass();
	GameStateClass = ACustomGameState::StaticClass();
	
}


void ACustomGameMode::StartPlay() 
{
	SpawnInitialActors();

	// This is called before BeginPlay
	StartPlayEvent();
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Current Score: %d"), GetScore()));

	// This function will transcend to call BeginPlay on all the actors 
	Super::StartPlay();

	if (UIWidget) 
	{
		UUserWidget* ui = CreateWidget<UUserWidget>(GetWorld(), UIWidget);
		ui->AddToViewport();
	}
}

// An implementation of the StartPlayEvent which can be triggered by calling StartPlayEvent() 
void ACustomGameMode::StartPlayEvent_Implementation() 
{
	// Start a timer which will call the SpawnCube Function every 4 seconds
	// GetWorldTimerManager().SetTimer(Ticker, this, &ACustomGameMode::SpawnCube, 4.0f, true, 0.0f);
}

int32 ACustomGameMode::GetScore()
{
	return GetGameState<ACustomGameState>()->Score;
}

void ACustomGameMode::SetScore(const int32 NewScore)
{
	GetGameState<ACustomGameState>()->Score = NewScore;
}

void ACustomGameMode::IncrementScore(const int32 ScoreToAdd)
{
	GetGameState<ACustomGameState>()->Score += ScoreToAdd;
}

StateEnum ACustomGameMode::GetStateEnum()
{
	return GetGameState<ACustomGameState>()->StateTracker;	
}

void ACustomGameMode::SetStateEnum(const StateEnum state)
{
	GetGameState<ACustomGameState>()->StateTracker = state;	
}

void ACustomGameMode::ResetPlanesSpawned()
{
	bPlaneSpawned = false;
	spawnedPlanes.Empty(); 
	workplaceSpawnTracker = 0;
}

void ACustomGameMode::AddPlane()
{
	if (PlayArea == NULL) return;

	bPlaneSpawned = false;
	spawnedPlanes.Add(PlayArea);
	PlayArea = NULL;
}

void ACustomGameMode::RemovePlane()
{
	bPlaneSpawned = false;

	if (PlayArea) 
	{
		PlayArea->Destroy();
		PlayArea = NULL;
	}
	else  if (spawnedPlanes.Num() > 0)
	{
		spawnedPlanes.Last()->Destroy();
		spawnedPlanes.Pop();
	}
}

bool ACustomGameMode::CheckPlanes()
{
	return (spawnedPlanes.Num() > 0);	
}

void ACustomGameMode::HidePlanes()
{
	for (int i = 0; i < spawnedPlanes.Num(); i++) 
	{
		spawnedPlanes[i]->UpdatePlaneMaterial(TransMat);
	}
}

void ACustomGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ACustomGameMode::SpawnInitialActors()
{
	// Spawn an instance of the HelloARManager class
	ARManager = GetWorld()->SpawnActor<AHelloARManager>();
}

void ACustomGameMode::SpawnBuildings()
{

	// Every 3 times this timer triggers reset the tracker to 0
	if (workplaceSpawnTracker == 3) 
	{
		workplaceSpawnTracker = 0;
	}

	// Make an array to store the vertices
	TArray<FVector> vertices;

	// Spawn a workplace every 3 times starting the 1st
	if (workplaceSpawnTracker == 0) 
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Spawning Workplace"));

		// Pick a random player defined play area
		int randomPlaneInt = UKismetMathLibrary::RandomInteger64InRange(0, spawnedPlanes.Num() - 1);

		// Get the vertices and location of the randomly selected plane
		TArray<FVector> verticesOnPlane = spawnedPlanes[randomPlaneInt]->GetVertices();
		FVector planeLoc = spawnedPlanes[randomPlaneInt]->GetActorLocation();

		// Pick 2 random vertices
		vertices.Empty();
		for (int i = 0; i < 2; i++)
		{
			int randomInt = UKismetMathLibrary::RandomInteger64InRange(0, verticesOnPlane.Num() - 1);

			vertices.Add(verticesOnPlane[randomInt]);
		}

		// Pick a radom number between 0 and 1 to use as a ratio
		float randomFloat = UKismetMathLibrary::RandomFloatInRange(0.f, 1.f);

		const FActorSpawnParameters SpawnInfo;
		const FRotator MyRot(0.f, UKismetMathLibrary::RandomFloatInRange(0.f, 360.f), 0.f);
		FVector spawnLocation;

		// Using the random ratio find a location on the plane between the two points
		spawnLocation = (vertices[0] * randomFloat) + (vertices[1] * (1 - randomFloat)) + planeLoc;

		GetWorld()->SpawnActor<AStaticActor>(WorkplaceToSpawn, spawnLocation, MyRot, SpawnInfo);
	}

	// Pick a random player defined play area
	int randomPlaneInt = UKismetMathLibrary::RandomInteger64InRange(0, spawnedPlanes.Num() - 1);

	// Spawn houses
	for (int o = 0; o < 4; o++) 
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Spawning House"));

		// Get the vertices and location of the randomly selected plane
		TArray<FVector> verticesOnPlane = spawnedPlanes[randomPlaneInt]->GetVertices();
		FVector planeLoc = spawnedPlanes[randomPlaneInt]->GetActorLocation();

		// Pick 2 random vertices
		vertices.Empty();
		for (int i = 0; i < 2; i++)
		{
			int randomInt = UKismetMathLibrary::RandomInteger64InRange(0, verticesOnPlane.Num() - 1);

			vertices.Add(verticesOnPlane[randomInt]);
		}

		// Pick a radom number between 0 and 1 to use as a ratio
		float randomFloat = UKismetMathLibrary::RandomFloatInRange(0.f, 1.f);

		const FActorSpawnParameters SpawnInfo;
		const FRotator MyRot(0.f, UKismetMathLibrary::RandomFloatInRange(0.f, 360.f), 0.f);
		FVector spawnLocation;

		// Using the random ratio find a location on the plane between the two points
		spawnLocation = (vertices[0] * randomFloat) + (vertices[1] * (1 - randomFloat)) + planeLoc;

		GetWorld()->SpawnActor<AStaticActor>(HouseToSpawn, spawnLocation, MyRot, SpawnInfo);
	}

	// Increment the tracker
	workplaceSpawnTracker++;
}

void ACustomGameMode::LineTraceSpawnStation(FVector ScreenPos)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Line Trace Reached"));

	//Basic variables for functionality
	APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0);
	FVector WorldPos;
	FVector WorldDir;

	//Gets the screen touch in world space and the tracked objects from a line trace from the touch
	UGameplayStatics::DeprojectScreenToWorld(playerController, FVector2D(ScreenPos), WorldPos, WorldDir);

	// construct trace vector (from point clicked to 1000.0 units beyond in same direction)
	FVector traceEndVector = WorldDir * 2000.0;
	traceEndVector = WorldPos + traceEndVector;

	FHitResult hitResult;

	// perform line trace (Raycast)
	bool traceSuccess = GetWorld()->LineTraceSingleByChannel(hitResult, WorldPos, traceEndVector, ECollisionChannel::ECC_WorldStatic);

	if (traceSuccess)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Trace Succeeded"));
		const FActorSpawnParameters SpawnInfo;
		const FRotator MyRot(0, 0, 0);
		const FVector MyLoc(hitResult.Location.X, hitResult.Location.Y, hitResult.Location.Z + 0.01);

		GetWorld()->SpawnActor<APlaceableActor>(StationToSpawn, MyLoc, MyRot, SpawnInfo);
	}
}

void ACustomGameMode::LineTraceSpawnPlane(FVector2D ScreenPos)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Line Trace Reached"));

	//Basic variables for functionality
	APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0);
	FVector WorldPos;
	FVector WorldDir;

	//Gets the screen touch in world space and the tracked objects from a line trace from the touch
	UGameplayStatics::DeprojectScreenToWorld(playerController, ScreenPos, WorldPos, WorldDir);

	// construct trace vector (from point clicked to 1000.0 units beyond in same direction)
	FVector traceEndVector = WorldDir * 1000.0;
	traceEndVector = WorldPos + traceEndVector;

	FHitResult hitResult;

	// perform line trace (Raycast)
	auto TraceResult = UARBlueprintLibrary::LineTraceTrackedObjects(FVector2D(ScreenPos), false, false, false, true);

	if (TraceResult.IsValidIndex(0))
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Purple, TEXT("Trace Succeeded"));

		// Get the first found object in the line trace - ignoring the rest of the array elements
		auto TrackedTF = TraceResult[0].GetLocalToWorldTransform();

		const FActorSpawnParameters SpawnInfo;
		const FRotator MyRot(0, 0, 0);
		const FVector MyLoc = TrackedTF.GetLocation();

		if (!bPlaneSpawned)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Purple, TEXT("Spawn Succeeded"));
			PlayArea = GetWorld()->SpawnActor<APlayAreaPlane>(MyLoc, MyRot, SpawnInfo);
			bPlaneSpawned = true;
		}
		PlayArea->AddVertex(MyLoc);			
	}
}

