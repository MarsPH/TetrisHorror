// Fill out your copyright notice in the Description page of Project Settings.


#include "TetrisPiece.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATetrisPiece::ATetrisPiece()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to
 	// improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PieceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Piece Mesh"));

	SetRootComponent(PieceMesh);

	PieceMesh->SetMobility(EComponentMobility::Movable);
	PieceMesh->SetSimulatePhysics(false);

	PieceMesh->SetNotifyRigidBodyCollision(true);

	PieceMesh->OnComponentHit.AddDynamic(
		this,
		&ATetrisPiece::OnPieceHit
	);
	
}

void ATetrisPiece::StartControlWindow()
{
	if (!IsValid(PieceMesh))
	{
		return;
	}

	bIsFalling = false;

	PieceMesh->SetSimulatePhysics(false);

	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);

	TimeRemaining = ControlDuration;
	bWarningStarted = false;

	OnCountdownChanged(TimeRemaining);

	if (TimeRemaining <= WarningTime)
	{
		BeginWarning();
	}

	GetWorldTimerManager().SetTimer(
		CountdownTimerHandle,
		this,
		&ATetrisPiece::CountdownStep,
		1.0f,
		true
	);
}

void ATetrisPiece::CountdownStep()
{
	TimeRemaining =FMath::Max(TimeRemaining - 1,0);

	OnCountdownChanged(TimeRemaining);

	if(!bWarningStarted &&
		TimeRemaining <= WarningTime &&
		TimeRemaining > 0)
	{
		BeginWarning();
	}

	if (TimeRemaining == 0)
	{
		CountdownFinished();
	}
}

void ATetrisPiece::BeginWarning()
{
	bWarningStarted = true;

	if(IsValid(SirenSound))
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			SirenSound,
			GetActorLocation());
	}

	OnWarningStarted();
}

void ATetrisPiece::CountdownFinished()
{
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);

	StartFalling();
}



void ATetrisPiece::StartFalling()
{
	if (!IsValid(PieceMesh))
	{
		return;
	}

	bIsFalling = true;

	PieceMesh->SetSimulatePhysics(true);
	PieceMesh->WakeAllRigidBodies();
}

// Called when the game starts or when spawned
void ATetrisPiece::BeginPlay()
{
	Super::BeginPlay();
	StartControlWindow();
	
}

// Called every frame
void ATetrisPiece::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATetrisPiece::OnPieceHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!bIsFalling)
	{
		return;
	}

	const float ImpactForce = NormalImpulse.Size();

	if (ImpactForce < MinimumHeavyImpact)
	{
		return;
	}

	const float Strength = FMath::GetMappedRangeValueClamped(
		FVector2D(MinimumHeavyImpact, MaximumHeavyImpact),
		FVector2D(0.0f, 1.0f),
		ImpactForce
	);

	bIsFalling = false;

	OnHeavyImpact(Strength, Hit.ImpactPoint);
	
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Tetris piece impact: %f"),
		ImpactForce
	);
}

