// Fill out your copyright notice in the Description page of Project Settings.


#include "TetrisPiece.h"
#include "Components/StaticMeshComponent.h"

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
	
}

void ATetrisPiece::StartFalling()
{
	if (!IsValid(PieceMesh))
	{
		return;
	}

	PieceMesh->SetSimulatePhysics(true);
	PieceMesh->WakeAllRigidBodies();
}

// Called when the game starts or when spawned
void ATetrisPiece::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATetrisPiece::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

