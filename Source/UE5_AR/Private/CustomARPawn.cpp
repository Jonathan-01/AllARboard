// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomARPawn.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "ARBlueprintLibrary.h"
#include "Camera/CameraComponent.h"
#include "CustomGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlaceableActor.h"
#include <CustomGameState.h>

// Sets default values
ACustomARPawn::ACustomARPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	// Camera compentent attached to the pawn
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SceneComponent);

	// Load and initialise materials
	auto redMaterial = ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("/Game/Assets/Materials/Red_Mat.Red_Mat"));
	RedMat = redMaterial.Object;

	auto defaultMaterial = ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("/Game/Assets/Materials/Default_Mat.Default_Mat"));
	DefaultMat = defaultMaterial.Object;
}

// Called when the game starts or when spawned
void ACustomARPawn::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACustomARPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check for proximity to actors
	// CheckForNearbyActors();
	StateSwitch();

	if (isFingerDown) 
	{
		OnHold();
	}
}

void ACustomARPawn::StateSwitch()
{
	auto Temp = GetWorld()->GetAuthGameMode();
	auto GM = Cast<ACustomGameMode>(Temp);

	switch (GM->GetStateEnum())
	{
		case (StateEnum::MainMenu):
			// Will only initialise if not on this state the previous frame
			if (CurrentState != StateEnum::MainMenu) MainMenuInit();
			GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Orange, TEXT("MainMenu"));
			break;
		case (StateEnum::Setup):
			if (CurrentState != StateEnum::Setup && CurrentState != StateEnum::Pause) SetUpInit();
			GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Orange, TEXT("Setup"));
			break;
		case (StateEnum::Playing):
			if (CurrentState != StateEnum::Playing) PlayingInit();
			GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Orange, TEXT("Playing"));
			break;
		case (StateEnum::Pause):
			if (CurrentState == StateEnum::Playing) PauseInit();
			GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Orange, TEXT("Pause"));
			break;
		case (StateEnum::Exit):
			GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Orange, TEXT("Exit"));
			break;
		case (StateEnum::Lose):
			if (CurrentState != StateEnum::Lose) LoseInit();
			GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Orange, TEXT("Exit"));
			break;
	}

	CurrentState = GM->GetStateEnum(); 
}

// Initialisers
// Clean the level
void ACustomARPawn::MainMenuInit()
{
	auto Temp = GetWorld()->GetAuthGameMode();
	auto GM = Cast<ACustomGameMode>(Temp);

	// Reset the AR that was running if it had been
	if (UARBlueprintLibrary::GetARSessionStatus().Status == EARSessionStatus::Running) 
	{
		GM->ARManager->ShouldCreateNewPlanes(false);
		GM->ARManager->ResetARCoreSession();
	}

	// Find and remove all actors in the scene
	TArray<AActor*, FDefaultAllocator> AllPlayAreas;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayAreaPlane::StaticClass(), AllPlayAreas);
	for (int i = 0; i < AllPlayAreas.Num(); i++)
	{
		AllPlayAreas[i]->Destroy();
	}

	TArray<AActor*, FDefaultAllocator> AllBuildings;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Building"), AllBuildings);
	for (int i = 0; i < AllBuildings.Num(); i++)
	{
		AllBuildings[i]->Destroy();
	}

	GM->SetScore(0);
}

void ACustomARPawn::SetUpInit()
{
	auto Temp = GetWorld()->GetAuthGameMode();
	auto GM = Cast<ACustomGameMode>(Temp);

	GM->ARManager->StartARCoreSession();
	GM->ARManager->ShouldCreateNewPlanes(true);
}

void ACustomARPawn::PlayingInit()
{
	auto Temp = GetWorld()->GetAuthGameMode();
	auto GM = Cast<ACustomGameMode>(Temp);

	GM->ARManager->ShouldCreateNewPlanes(false);
	GM->ARManager->ResetARCoreSession();

	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Play Init"));
	if (CurrentState == StateEnum::Setup)
	{
		GetWorldTimerManager().SetTimer(BuildingTimer, this, &ACustomARPawn::SpawnBuildings, 20.f, true, 2.f);
		GetWorldTimerManager().SetTimer(ScoreTimer, this, &ACustomARPawn::IncrementScore, 1.f, true, 2.f);
	}

	GetWorldTimerManager().UnPauseTimer(BuildingTimer); 
	GetWorldTimerManager().UnPauseTimer(ScoreTimer);
}

void ACustomARPawn::PauseInit()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Pause Init"));
	GetWorldTimerManager().PauseTimer(BuildingTimer);
	GetWorldTimerManager().PauseTimer(ScoreTimer);
}

void ACustomARPawn::LoseInit()
{
	GetWorldTimerManager().ClearTimer(BuildingTimer);
	GetWorldTimerManager().ClearTimer(ScoreTimer);
}

void ACustomARPawn::SpawnBuildings()
{
	auto Temp = GetWorld()->GetAuthGameMode();
	auto GM = Cast<ACustomGameMode>(Temp);

	GM->SpawnBuildings();
}

void ACustomARPawn::IncrementScore()
{
	auto Temp = GetWorld()->GetAuthGameMode();
	auto GM = Cast<ACustomGameMode>(Temp);

	GM->IncrementScore(5);
}

// Called to bind functionality to input
void ACustomARPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Bind various player inputs to functions
	// There are a few types - BindTouch, BindAxis, and BindEvent.  
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ACustomARPawn::OnScreenTouch);
	PlayerInputComponent->BindTouch(IE_Released, this, &ACustomARPawn::OnScreenRelease);
}

void ACustomARPawn::OnScreenTouch(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{

	// Get access to gameMode to interact with the ARManager
	auto Temp = GetWorld()->GetAuthGameMode();
	auto GM = Cast<ACustomGameMode>(Temp);

	// Debug to screen space
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("ScreenTouch Reached"));

	FHitResult ActorHitResult;

	isFingerDown = true;

	// Quit here so we can reach OnHold
	if (CurrentState != StateEnum::Playing) return;

	// If raytrace hits something
	if (WorldHitTest(FVector2D(ScreenPos.X, ScreenPos.Y), ActorHitResult))
	{
		// Debug to screen space
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple, TEXT("HitTestSuccessful"));

		// Get object of actor hit.
		UClass* hitActorClass = UGameplayStatics::GetObjectClass(ActorHitResult.GetActor());

		// If hit actor belongs to APlaceableActor
		if (UKismetMathLibrary::ClassIsChildOf(hitActorClass, APlaceableActor::StaticClass())) 
		{
			// If a previous actor is slected remove the redMat so it doesn't look selecetd
			if (SelectedActor != NULL)
			{
				//SelectedActor->StaticMeshComponent->SetMaterial(0, DefaultMat);
			}
			
			// Track the last selected actor
			SelectedActor = Cast<APlaceableActor>(ActorHitResult.GetActor());
			SelectedActor->CheckNearbyBuildings(30, true);
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("Checked Buildings!"));
		}
		// If an actor is already selected
		else if (SelectedActor != NULL)
		{
			// Deselect the previous actor
			SelectedActor = NULL;
		}
		// Otherwise if a playable area is selected
		else if (UKismetMathLibrary::ClassIsChildOf(hitActorClass, APlayAreaPlane::StaticClass()))
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("Touched Plane!"));

			GM->LineTraceSpawnStation(ScreenPos);
		}
	}
}

void ACustomARPawn::OnHold()
{	
	if (CurrentState != StateEnum::Setup && CurrentState != StateEnum::Playing) return;

	// If there is no selected actor or finger is not held down
	if (!isFingerDown) return;

	// Get access to gameMode to interact with the ARManager
	auto Temp = GetWorld()->GetAuthGameMode();
	auto GM = Cast<ACustomGameMode>(Temp);

	// Debug to screen space
	GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Green, TEXT("On Hold Reached"));

	// Setup
	if (CurrentState == StateEnum::Setup) 
	{
		// Finger X and Y position
		float fingerX;
		float fingerY;

		bool isTouched;    // Needed for the input touch state funtion

		// Get inputs from player controller
		GetWorld()->GetFirstPlayerController()->GetInputTouchState(ETouchIndex::Touch1, fingerX, fingerY, isTouched);

		GM->LineTraceSpawnPlane(FVector2D(fingerX, fingerY));
		return;
	}

	// If we reach here we're in playing state
	if (SelectedActor != NULL)
	{
		MoveSelectedActor();
	}
}

void ACustomARPawn::OnScreenRelease(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{
	if (CurrentState != StateEnum::Setup && CurrentState != StateEnum::Playing) return;

	// Debug to screen space
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("On Release Reached"));

	// Release the onHold function
	isFingerDown = false;
}

bool ACustomARPawn::WorldHitTest(FVector2D screenPos, FHitResult& hitResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("HitTest Reached"));

	// Get player controller
	APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0);

	// Perform deprojection taking 2d clicked area and generating reference in 3d world-space.
	FVector worldPosition;
	FVector worldDirection; // Unit Vector

	bool deprojectionSuccess = UGameplayStatics::DeprojectScreenToWorld(playerController, screenPos, /*out*/ worldPosition, /*out*/ worldDirection);

	// construct trace vector (from point clicked to 1000.0 units beyond in same direction)
	FVector traceEndVector = worldDirection * 3000.0;
	traceEndVector = worldPosition + traceEndVector;

	// perform line trace (Raycast)
	bool traceSuccess = GetWorld()->LineTraceSingleByChannel(hitResult, worldPosition, traceEndVector, ECollisionChannel::ECC_WorldStatic);

	// return if the operation was successful
	return traceSuccess;
}

void ACustomARPawn::CheckForNearbyActors() 
{
	//GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Yellow, TEXT("ActorSearching Reached"));

	// Array to store found actors
	TArray<AActor*, FDefaultAllocator> PlacedActors;

	// Find placed actors
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlaceableActor::StaticClass(), PlacedActors);

	// Get camera loaction
	APlayerCameraManager* camManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager; 
	FVector camLocation = camManager->GetCameraLocation(); 

	// Initialise FVector for actor location
	FVector actorLocation;

	// Distance to the actor for material switch
	int16 distToActor = 100;

	// Loop through each found actor
	for (int i = 0; i < PlacedActors.Num(); i++) {
		// Current actor location
		actorLocation = PlacedActors[i]->GetActorLocation();

		// Find whether actor is within range
		if (FVector::Distance(camLocation, actorLocation) < distToActor)
		{
			Cast<APlaceableActor>(PlacedActors[i])->StaticMeshComponent->SetMaterial(0, RedMat);
			GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Yellow, TEXT("Actor Within Range"));
		}
		else 
		{
			Cast<APlaceableActor>(PlacedActors[i])->StaticMeshComponent->SetMaterial(0, DefaultMat);
			GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Yellow, TEXT("Actor Outside Range"));
		}

		// Reset the location
		actorLocation = FVector::ZeroVector;
	}	
}

void ACustomARPawn::MoveSelectedActor()
{
	// Finger X and Y position
	float fingerX;
	float fingerY;

	bool isTouched;    // Needed for the input touch state funtion

	// Get inputs from player controller
	GetWorld()->GetFirstPlayerController()->GetInputTouchState(ETouchIndex::Touch1, fingerX, fingerY, isTouched);

	// Convert varaibles to a 2D vector
	FVector2D screenPos = FVector2D(fingerX, fingerY);


	//Basic variables for functionality
	APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0);
	FVector WorldPos;
	FVector WorldDir;

	//Gets the screen touch in world space and the tracked objects from a line trace from the touch
	UGameplayStatics::DeprojectScreenToWorld(playerController, FVector2D(screenPos), WorldPos, WorldDir);

	// construct trace vector (from point clicked to 1000.0 units beyond in same direction)
	FVector traceEndVector = WorldDir * 2000.0;
	traceEndVector = WorldPos + traceEndVector;

	FHitResult hitResult;

	// perform line trace (Raycast)
	bool traceSuccess = GetWorld()->LineTraceSingleByChannel(hitResult, WorldPos, traceEndVector, ECollisionChannel::ECC_WorldDynamic);

	if (traceSuccess) 
	{
		FHitResult HitResultBin;
		SelectedActor->StaticMeshComponent->K2_SetWorldLocation(hitResult.Location, false, HitResultBin, false);
	}
}