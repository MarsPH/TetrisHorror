#include "TetrisPiece.h"

#include "TetrisBoard.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/DamageType.h"
#include "Components/CapsuleComponent.h"


ATetrisPiece::ATetrisPiece()
{
	PrimaryActorTick.bCanEverTick = false;

	PieceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Piece Mesh"));
	SetRootComponent(PieceMesh);

	PieceMesh->SetMobility(EComponentMobility::Movable);
	PieceMesh->SetSimulatePhysics(false);
	PieceMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PieceMesh->SetNotifyRigidBodyCollision(true);
	PieceMesh->SetGenerateOverlapEvents(true);


	// The piece collides physically with the environment,
	// but passes through the player without being deflected.
	PieceMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	PieceMesh->OnComponentBeginOverlap.AddDynamic(
		this,
		&ATetrisPiece::OnPieceOverlap);

	PieceMesh->OnComponentHit.AddDynamic(
		this, &ATetrisPiece::OnPieceHit);
}

void ATetrisPiece::InitializePiece(ATetrisBoard* InBoard)
{
	Board = InBoard;
}

void ATetrisPiece::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(SpawnSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpawnSound, GetActorLocation());
	}

	StartControlWindow();
}

void ATetrisPiece::StartControlWindow()
{
	if (!IsValid(PieceMesh))
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
	StopFallingAudio();

	bIsControllable = true;
	bIsFalling = false;
	bLandingRequested = false;
	bWarningStarted = false;
	TimeRemaining = ControlDuration;

	PieceMesh->SetSimulatePhysics(false);
	PieceMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

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
		true);

	bHasCrushedPlayer = false;
}

void ATetrisPiece::CountdownStep()
{
	TimeRemaining = FMath::Max(TimeRemaining - 1, 0);
	OnCountdownChanged(TimeRemaining);

	if (!bWarningStarted && TimeRemaining <= WarningTime && TimeRemaining > 0)
	{
		BeginWarning();
	}

	if (TimeRemaining == 0)
	{
		StartFalling();
	}
}

void ATetrisPiece::BeginWarning()
{
	bWarningStarted = true;

	if (IsValid(WarningSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, WarningSound, GetActorLocation());
	}

	OnWarningStarted();
}

void ATetrisPiece::StartFalling()
{
	if (!IsValid(PieceMesh) || bIsFalling)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);

	bIsControllable = false;
	bIsFalling = true;
	OnControlEnded();

	PieceMesh->SetSimulatePhysics(true);
	PieceMesh->SetMassOverrideInKg(NAME_None, PieceMassKg, true);
	PieceMesh->SetPhysicsLinearVelocity(FVector(0.0f, 0.0f, -InitialDownwardSpeed));
	PieceMesh->WakeAllRigidBodies();

	if (IsValid(FallingSound))
	{
		FallingAudioComponent = UGameplayStatics::SpawnSoundAttached(
			FallingSound,
			PieceMesh,
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset);
	}
}

void ATetrisPiece::DropNow()
{
	if (bIsControllable)
	{
		StartFalling();
	}
}

void ATetrisPiece::MoveHorizontal(const int32 Direction)
{
	if (bIsControllable && IsValid(Board))
	{
		Board->TryMoveActivePiece(FMath::Sign(Direction));
	}
}

void ATetrisPiece::RotatePiece(const int32 Direction)
{
	if (bIsControllable && IsValid(Board))
	{
		Board->TryRotateActivePiece(FMath::Sign(Direction));
	}
}

void ATetrisPiece::ApplyControlledGridPosition(const FVector& NewWorldLocation)
{
	if (!bIsControllable)
	{
		return;
	}

	SetActorLocation(NewWorldLocation, false, nullptr, ETeleportType::TeleportPhysics);
}

void ATetrisPiece::ApplyControlledRotation(const int32 NewRotationStep)
{
	if (!bIsControllable)
	{
		return;
	}

	RotationStep = ((NewRotationStep % 4) + 4) % 4;

	// A Tetris board lies in the world X/Z plane, so rotation is around Y (Pitch).
    SetActorRotation(FRotator(-RotationStep * 90.0f, 0.0f, 0.0f));}

void ATetrisPiece::GetRotatedCellOffsets(
	const int32 InRotationStep,
	TArray<FIntPoint>& OutOffsets) const
{
	OutOffsets.Reset(4);

	switch (Shape)
	{
	case ETetrisShape::I:
		OutOffsets = {FIntPoint(-1, 0), FIntPoint(0, 0), FIntPoint(1, 0), FIntPoint(2, 0)};
		break;
	case ETetrisShape::O:
		OutOffsets = {FIntPoint(0, 0), FIntPoint(1, 0), FIntPoint(0, 1), FIntPoint(1, 1)};
		break;
	case ETetrisShape::T:
		OutOffsets = {FIntPoint(-1, 0), FIntPoint(0, 0), FIntPoint(1, 0), FIntPoint(0, 1)};
		break;
	case ETetrisShape::L:
		OutOffsets = {FIntPoint(-1, 0), FIntPoint(0, 0), FIntPoint(1, 0), FIntPoint(1, 1)};
		break;
	case ETetrisShape::J:
		OutOffsets = {FIntPoint(-1, 1), FIntPoint(-1, 0), FIntPoint(0, 0), FIntPoint(1, 0)};
		break;
	case ETetrisShape::S:
		OutOffsets = {FIntPoint(-1, 0), FIntPoint(0, 0), FIntPoint(0, 1), FIntPoint(1, 1)};
		break;
	case ETetrisShape::Z:
		OutOffsets = {FIntPoint(-1, 1), FIntPoint(0, 1), FIntPoint(0, 0), FIntPoint(1, 0)};
		break;
	default:
		checkNoEntry();
		break;
	}

	if (Shape == ETetrisShape::O)
	{
		return;
	}

	const int32 NormalizedSteps = ((InRotationStep % 4) + 4) % 4;

	for (FIntPoint& Cell : OutOffsets)
	{
		for (int32 Step = 0; Step < NormalizedSteps; ++Step)
		{
			// A clockwise quarter-turn in the X/Z board plane.
			Cell = FIntPoint(Cell.Y, -Cell.X);
		}
	}
}

void ATetrisPiece::OnPieceHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	
	if (!bIsFalling || bLandingRequested || OtherActor == this)
	{
		return;
	}

	// Ignore contact with vertical sides.
	// A landing surface should push mostly upward.
	if (Hit.ImpactNormal.Z < 0.6f)
	{
		return;
	}

	bLandingRequested = true;
	StopFallingAudio();

	const float ImpactForce = NormalImpulse.Size();

	if (IsValid(ImpactSound))
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, Hit.ImpactPoint);
	}

	if (ImpactForce >= MinimumHeavyImpact)
	{
		const float Strength = FMath::GetMappedRangeValueClamped(
			FVector2D(MinimumHeavyImpact, MaximumHeavyImpact),
			FVector2D(0.0f, 1.0f),
			ImpactForce);

		OnHeavyImpact(Strength, Hit.ImpactPoint);
	}

	UE_LOG(LogTemp, Log, TEXT("Tetris piece impact: %.2f"), ImpactForce);

	if (IsValid(Board))
	{
		Board->NotifyPieceImpact(this);
	}
}

void ATetrisPiece::OnPieceOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bIsFalling ||
		bHasCrushedPlayer ||
		!IsValid(OtherActor) ||
		!IsValid(PieceMesh))
	{
		return;
	}

	ACharacter* HitCharacter = Cast<ACharacter>(OtherActor);

	if (!IsValid(HitCharacter))
	{
		return;
	}

	UCapsuleComponent* PlayerCapsule =
		HitCharacter->GetCapsuleComponent();

	if (!IsValid(PlayerCapsule))
	{
		return;
	}

	// The piece must actually be descending quickly.
	const float DownwardSpeed =
		-PieceMesh->GetPhysicsLinearVelocity().Z;

	if (DownwardSpeed < MinimumCrushSpeed)
	{
		return;
	}

	const float PieceBottomZ =
		PieceMesh->Bounds.Origin.Z -
		PieceMesh->Bounds.BoxExtent.Z;

	const float PlayerTopZ =
		PlayerCapsule->Bounds.Origin.Z +
		PlayerCapsule->Bounds.BoxExtent.Z;

	// If the bottom of the piece is far below the player's head,
	// this is side contact, not a crushing hit.
	if (PieceBottomZ < PlayerTopZ - CrushTopTolerance)
	{
		return;
	}

	bHasCrushedPlayer = true;

	UGameplayStatics::ApplyDamage(
		HitCharacter,
		CrushDamage,
		nullptr,
		this,
		UDamageType::StaticClass());
}

void ATetrisPiece::PrepareForLock()
{
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
	StopFallingAudio();

	bIsControllable = false;
	bIsFalling = false;

	if (IsValid(PieceMesh))
	{
		PieceMesh->SetSimulatePhysics(false);
		PieceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ATetrisPiece::StopFallingAudio()
{
	if (IsValid(FallingAudioComponent))
	{
		FallingAudioComponent->Stop();
		FallingAudioComponent = nullptr;
	}
}
