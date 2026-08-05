#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TetrisPiece.generated.h"

class ATetrisBoard;
class ATetrisCell;
class UAudioComponent;
class UPrimitiveComponent;
class USoundBase;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ETetrisShape : uint8
{
	I UMETA(DisplayName = "I"),
	O UMETA(DisplayName = "O"),
	T UMETA(DisplayName = "T"),
	L UMETA(DisplayName = "L"),
	J UMETA(DisplayName = "J"),
	S UMETA(DisplayName = "S"),
	Z UMETA(DisplayName = "Z")
};

UCLASS()
class TETRISHORROR_API ATetrisPiece : public AActor
{
	GENERATED_BODY()

public:
	ATetrisPiece();

	void InitializePiece(ATetrisBoard* InBoard);
	void GetRotatedCellOffsets(int32 InRotationStep, TArray<FIntPoint>& OutOffsets) const;
	void ApplyControlledGridPosition(const FVector& NewWorldLocation);
	void ApplyControlledRotation(int32 NewRotationStep);
	void PrepareForLock();

	int32 GetRotationStep() const { return RotationStep; }
	TSubclassOf<ATetrisCell> GetLockedCellClass() const { return LockedCellClass; }
	bool IsControllable() const { return bIsControllable; }

	UFUNCTION(BlueprintCallable, Category = "Tetris Piece|Control")
	void MoveHorizontal(int32 Direction);

	UFUNCTION(BlueprintCallable, Category = "Tetris Piece|Control")
	void RotatePiece(int32 Direction);

	UFUNCTION(BlueprintCallable, Category = "Tetris Piece|Control")
	void DropNow();

	UFUNCTION(BlueprintCallable, Category = "Tetris Piece|State")
	void StartControlWindow();

	UFUNCTION(BlueprintCallable, Category = "Tetris Piece|State")
	void StartFalling();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tetris Piece")
	TObjectPtr<UStaticMeshComponent> PieceMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tetris Piece")
	ETetrisShape Shape = ETetrisShape::T;

	// This class is spawned once for each of the four logical blocks after landing.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tetris Piece")
	TSubclassOf<ATetrisCell> LockedCellClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tetris Piece|Timing",
		meta = (ClampMin = "1"))
	int32 ControlDuration = 8;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tetris Piece|Timing",
		meta = (ClampMin = "0"))
	int32 WarningTime = 3;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tetris Piece|Timing")
	int32 TimeRemaining = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Tetris Piece|Audio")
	TObjectPtr<USoundBase> SpawnSound;

	UPROPERTY(EditDefaultsOnly, Category = "Tetris Piece|Audio")
	TObjectPtr<USoundBase> WarningSound;

	// Make this sound loop if it should continue for the whole fall.
	UPROPERTY(EditDefaultsOnly, Category = "Tetris Piece|Audio")
	TObjectPtr<USoundBase> FallingSound;

	UPROPERTY(EditDefaultsOnly, Category = "Tetris Piece|Audio")
	TObjectPtr<USoundBase> ImpactSound;

	UPROPERTY(EditDefaultsOnly, Category = "Tetris Piece|Impact",
		meta = (ClampMin = "0.0"))
	float MinimumHeavyImpact = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Tetris Piece|Impact",
		meta = (ClampMin = "0.0"))
	float MaximumHeavyImpact = 10000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Tetris Piece|Falling",
		meta = (ClampMin = "0.0"))
	float PieceMassKg = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Tetris Piece|Falling",
		meta = (ClampMin = "0.0"))
	float InitialDownwardSpeed = 400.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Tetris Piece|Damage",
    	meta = (ClampMin = "0.0"))
    float CrushDamage = 100.0f;
	

	UFUNCTION(BlueprintImplementableEvent, Category = "Tetris Piece|Events")
	void OnCountdownChanged(int32 NewTimeRemaining);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tetris Piece|Events")
	void OnWarningStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Tetris Piece|Events")
	void OnControlEnded();

	UFUNCTION(BlueprintImplementableEvent, Category = "Tetris Piece|Events")
	void OnHeavyImpact(float Strength, FVector ImpactLocation);

private:
	UFUNCTION()
	void OnPieceHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UFUNCTION()
	void OnPieceOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void CountdownStep();
	void BeginWarning();
	void StopFallingAudio();

	UPROPERTY(Transient)
	TObjectPtr<ATetrisBoard> Board;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> FallingAudioComponent;

	FTimerHandle CountdownTimerHandle;

	int32 RotationStep = 0;
	bool bIsControllable = false;
	bool bIsFalling = false;
	bool bWarningStarted = false;
	bool bLandingRequested = false;
	bool bHasCrushedPlayer = false;
};
