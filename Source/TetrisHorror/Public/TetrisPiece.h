// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TetrisPiece.generated.h"

UCLASS()
class TETRISHORROR_API ATetrisPiece : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATetrisPiece();

	UFUNCTION(BlueprintCallable, Category = "Tetris") void StartControlWindow();

	//For falling
	UFUNCTION(BlueprintCallable, Category = "Tetris") void StartFalling();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//For visiblity + Mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tetris")
	TObjectPtr<UStaticMeshComponent> PieceMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tetris | Timing",
		meta = (ClampMin = "1.0"))
	int32 ControlDuration = 8;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tetris | Timing",
		meta = (ClampMin = "1.0"))
	int32 WarningTime = 3;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tetris Piece|Timing")
	int32 TimeRemaining = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Tetris Piece|Audio")
	TObjectPtr<USoundBase> SirenSound;

	// Blueprint can use this to update a HUD/widget.
	UFUNCTION(BlueprintImplementableEvent, Category = "Tetris Piece|Events")
	void OnCountdownChanged(int32 NewTimeRemaining);

	// Blueprint can flash lights, change material, shake the camera, etc.
	UFUNCTION(BlueprintImplementableEvent, Category = "Tetris Piece|Events")
	void OnWarningStarted();

	UFUNCTION()
	void OnPieceHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tetris Piece|Impact")
	void OnHeavyImpact(float Strength, FVector ImpactLocation);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	
	void CountdownStep();
	void BeginWarning();
	void CountdownFinished();

	bool bIsFalling = false;

	UPROPERTY(EditDefaultsOnly, Category = "Tetris Piece|Impact",
		meta = (ClampMin = "0.0"))
	float MinimumHeavyImpact = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Tetris Piece|Impact",
		meta = (ClampMin = "0.0"))
	float MaximumHeavyImpact = 10000.0f;
	

	FTimerHandle CountdownTimerHandle;

	bool bWarningStarted = false;
};
