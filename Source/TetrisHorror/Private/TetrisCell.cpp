#include "TetrisCell.h"

#include "Components/StaticMeshComponent.h"

ATetrisCell::ATetrisCell()
{
	PrimaryActorTick.bCanEverTick = false;

	CellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cell Mesh"));
	SetRootComponent(CellMesh);

	CellMesh->SetMobility(EComponentMobility::Movable);
	CellMesh->SetSimulatePhysics(false);
	CellMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ATetrisCell::InitializeCell(ATetrisBoard* InBoard, const FIntPoint& InGridCoordinate)
{
	Board = InBoard;
	GridCoordinate = InGridCoordinate;
}

void ATetrisCell::BeginClearWarning()
{
	if (bMarkedForClear)
	{
		return;
	}

	bMarkedForClear = true;
	OnClearWarningStarted();
}

void ATetrisCell::MoveToGridCoordinate(
	const FIntPoint& NewCoordinate,
	const FVector& NewWorldLocation)
{
	GridCoordinate = NewCoordinate;
	SetActorLocation(NewWorldLocation, false, nullptr, ETeleportType::TeleportPhysics);
	OnGridPositionChanged(NewCoordinate);
}
