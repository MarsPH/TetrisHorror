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

	//For visiblity + Mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tetris")
	TObjectPtr<UStaticMeshComponent> PieceMesh;

	//For falling
	UFUNCTION(BlueprintCallable, Category = "Tetris") void StartFalling();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
