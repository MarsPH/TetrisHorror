#include "TetrisRemoteComponent.h"

#include "TetrisBoard.h"
#include "Kismet/GameplayStatics.h"

UTetrisRemoteComponent::UTetrisRemoteComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTetrisRemoteComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!IsValid(Board) && bFindBoardAutomatically)
	{
		Board = Cast<ATetrisBoard>(
			UGameplayStatics::GetActorOfClass(this, ATetrisBoard::StaticClass()));
	}
}

void UTetrisRemoteComponent::MoveLeft()
{
	if (IsValid(Board))
	{
		Board->TryMoveActivePiece(-1);
	}
}

void UTetrisRemoteComponent::MoveRight()
{
	if (IsValid(Board))
	{
		Board->TryMoveActivePiece(1);
	}
}

void UTetrisRemoteComponent::RotateClockwise()
{
	if (IsValid(Board))
	{
		Board->TryRotateActivePiece(1);
	}
}

void UTetrisRemoteComponent::RotateCounterClockwise()
{
	if (IsValid(Board))
	{
		Board->TryRotateActivePiece(-1);
	}
}

void UTetrisRemoteComponent::DropNow()
{
	if (IsValid(Board))
	{
		Board->DropActivePiece();
	}
}
