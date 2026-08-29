#include "TetrisBoard.h"

#include "TetrisCell.h"
#include "TetrisPiece.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ATetrisBoard::ATetrisBoard()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	SetRootComponent(SceneRoot);
}

void ATetrisBoard::BeginPlay()
{
	Super::BeginPlay();
	ScheduleNextPiece(FirstSpawnDelay);
}

void ATetrisBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShowDebugGrid)
	{
		DrawBoardDebug();
	}
}

#if WITH_EDITOR
bool ATetrisBoard::ShouldTickIfViewportsOnly() const
{
	return true;
}
#endif

void ATetrisBoard::DrawBoardDebug() const
{
	UWorld* World = GetWorld();

	if (!IsValid(World) || CellSize <= 0.0f)
	{
		return;
	}

	const FVector Origin = GetActorLocation();
	const float HalfCell = CellSize * 0.5f;

	// The actor is at the centre of cell (0, 0).
	const float MinimumX = Origin.X - HalfCell;
	const float MinimumZ = Origin.Z - HalfCell;

	const float MaximumX = MinimumX + BoardWidth * CellSize;
	const float MaximumZ = MinimumZ + BoardHeight * CellSize;

	const float GridY = Origin.Y - CellSize * 0.51f;
	
	// Vertical grid lines: columns.
	for (int32 Column = 0; Column <= BoardWidth; ++Column)
	{
		const float X = MinimumX + Column * CellSize;

		const bool bIsBorder = Column == 0 || Column == BoardWidth;
		const FColor Colour = bIsBorder ? FColor::White : FColor::Cyan;

		DrawDebugLine(
			World,
			FVector(X, GridY, MinimumZ),
			FVector(X, GridY, MaximumZ),
			Colour,
			false,
			0.0f,
			1,
			DebugLineThickness);
	}

	// Horizontal grid lines: rows.
	for (int32 Row = 0; Row <= BoardHeight; ++Row)
	{
		const float Z = MinimumZ + Row * CellSize;

		const bool bIsBorder = Row == 0 || Row == BoardHeight;
		const FColor Colour = bIsBorder ? FColor::White : FColor::Cyan;

		DrawDebugLine(
			World,
			FVector(MinimumX, GridY, Z),
			FVector(MaximumX, GridY, Z),
			Colour,
			false,
			0.0f,
			1,
			DebugLineThickness);
	}

	// Green point: centre of cell (0, 0), also the actor location.
	DrawDebugPoint(
		World,
		Origin,
		30.0f,
		FColor::Green,
		false,
		0.0f,
		1);

	// Red box: where the next piece's anchor spawns.
	const FVector SpawnLocation =
		GridToWorld(FIntPoint(BoardWidth / 2, SpawnHeightRows));

	DrawDebugBox(
		World,
		SpawnLocation,
		FVector(HalfCell, 20.0f, HalfCell),
		FColor::Red,
		false,
		0.0f,
		1,
		DebugLineThickness);
}

FVector ATetrisBoard::GridToWorld(const FIntPoint GridCoordinate) const
{
	const FVector Origin = GetActorLocation();
	return Origin + FVector(
		GridCoordinate.X * CellSize,
		0.0f,
		GridCoordinate.Y * CellSize);
}

FIntPoint ATetrisBoard::WorldToGrid(const FVector& WorldLocation) const
{
	const FVector Local = WorldLocation - GetActorLocation();
	return FIntPoint(
		FMath::RoundToInt(Local.X / CellSize),
		FMath::RoundToInt(Local.Z / CellSize));
}

void ATetrisBoard::ScheduleNextPiece(const float Delay)
{
	if (bGameOver)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&ATetrisBoard::SpawnNextPiece,
		FMath::Max(Delay, 0.01f),
		false);
}

void ATetrisBoard::SpawnNextPiece()
{
	if (bGameOver || IsValid(ActivePiece))
	{
		return;
	}

	TArray<TSubclassOf<ATetrisPiece>> ValidClasses;
	for (const TSubclassOf<ATetrisPiece>& PieceClass : PieceClasses)
	{
		if (PieceClass)
		{
			ValidClasses.Add(PieceClass);
		}
	}

	if (ValidClasses.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("TetrisBoard has no PieceClasses."));
		return;
	}

	const int32 ClassIndex = FMath::RandRange(0, ValidClasses.Num() - 1);
	const TSubclassOf<ATetrisPiece> ChosenClass = ValidClasses[ClassIndex];

	const FIntPoint SpawnAnchor(BoardWidth / 2, SpawnHeightRows);
	const FTransform SpawnTransform(
		FRotator::ZeroRotator,
		GridToWorld(SpawnAnchor));

	ATetrisPiece* NewPiece = GetWorld()->SpawnActorDeferred<ATetrisPiece>(
		ChosenClass,
		SpawnTransform,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!IsValid(NewPiece))
	{
		UE_LOG(LogTemp, Error, TEXT("TetrisBoard could not spawn a piece."));
		ScheduleNextPiece(NextPieceDelay);
		return;
	}

	NewPiece->InitializePiece(this);
	UGameplayStatics::FinishSpawningActor(NewPiece, SpawnTransform);
	ActivePiece = NewPiece;

	if (!CanPlacePieceAt(ActivePiece, SpawnAnchor, 0))
	{
		ActivePiece->Destroy();
		ActivePiece = nullptr;
		bGameOver = true;
		OnGameOver();
	}
}

bool ATetrisBoard::TryMoveActivePiece(const int32 Direction)
{
	if (!IsValid(ActivePiece) || !ActivePiece->IsControllable() || Direction == 0)
	{
		return false;
	}

	FIntPoint CandidateAnchor = WorldToGrid(ActivePiece->GetActorLocation());
	CandidateAnchor.X += FMath::Sign(Direction);

	if (!CanPlacePieceAt(ActivePiece, CandidateAnchor, ActivePiece->GetRotationStep()))
	{
		return false;
	}

	ActivePiece->ApplyControlledGridPosition(GridToWorld(CandidateAnchor));
	return true;
}

bool ATetrisBoard::TryRotateActivePiece(const int32 Direction)
{
	if (!IsValid(ActivePiece) || !ActivePiece->IsControllable() || Direction == 0)
	{
		return false;
	}

	const FIntPoint Anchor = WorldToGrid(ActivePiece->GetActorLocation());
	const int32 CandidateRotation = ActivePiece->GetRotationStep() + FMath::Sign(Direction);

	if (CanPlacePieceAt(ActivePiece, Anchor, CandidateRotation))
	{
		ActivePiece->ApplyControlledRotation(CandidateRotation);
		return true;
	}

	// One-cell wall kicks keep rotation usable beside the board edges.
	for (const int32 Kick : {-1, 1})
	{
		const FIntPoint KickedAnchor(Anchor.X + Kick, Anchor.Y);
		if (CanPlacePieceAt(ActivePiece, KickedAnchor, CandidateRotation))
		{
			ActivePiece->ApplyControlledGridPosition(GridToWorld(KickedAnchor));
			ActivePiece->ApplyControlledRotation(CandidateRotation);
			return true;
		}
	}

	return false;
}

void ATetrisBoard::DropActivePiece()
{
	if (IsValid(ActivePiece))
	{
		ActivePiece->DropNow();
	}
}

bool ATetrisBoard::CanPlacePieceAt(
	const ATetrisPiece* Piece,
	const FIntPoint& Anchor,
	const int32 RotationStep) const
{
	if (!IsValid(Piece))
	{
		return false;
	}

	TArray<FIntPoint> Offsets;
	Piece->GetRotatedCellOffsets(RotationStep, Offsets);

	for (const FIntPoint& Offset : Offsets)
	{
		const FIntPoint Coordinate = Anchor + Offset;

		if (Coordinate.X < 0 || Coordinate.X >= BoardWidth || Coordinate.Y < 0)
		{
			return false;
		}

		if (OccupiedCells.Contains(Coordinate))
		{
			return false;
		}
	}

	return true;
}

void ATetrisBoard::NotifyPieceImpact(ATetrisPiece* Piece)
{
	if (!IsValid(Piece) || Piece != ActivePiece || bGameOver)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(LockTimerHandle);
	GetWorldTimerManager().SetTimer(
		LockTimerHandle,
		this,
		&ATetrisBoard::LockActivePiece,
		FMath::Max(SettleDelay, 0.01f),
		false);
}

bool ATetrisBoard::FindFreeLockAnchor(
	const ATetrisPiece* Piece,
	FIntPoint& InOutAnchor) const
{
	TArray<FIntPoint> Offsets;
	Piece->GetRotatedCellOffsets(Piece->GetRotationStep(), Offsets);

	for (const FIntPoint& Offset : Offsets)
	{
		const int32 Column = InOutAnchor.X + Offset.X;
		if (Column < 0 || Column >= BoardWidth)
		{
			return false;
		}
	}

	for (int32 Attempt = 0; Attempt <= BoardHeight + 4; ++Attempt)
	{
		bool bBlocked = false;

		for (const FIntPoint& Offset : Offsets)
		{
			const FIntPoint Coordinate = InOutAnchor + Offset;
			if (Coordinate.Y < 0 || OccupiedCells.Contains(Coordinate))
			{
				bBlocked = true;
				break;
			}
		}

		if (!bBlocked)
		{
			return true;
		}

		++InOutAnchor.Y;
	}

	return false;
}

void ATetrisBoard::LockActivePiece()
{
	if (!IsValid(ActivePiece) || bGameOver)
	{
		return;
	}

	ActivePiece->PrepareForLock();

	FIntPoint Anchor = WorldToGrid(ActivePiece->GetActorLocation());
	if (!FindFreeLockAnchor(ActivePiece, Anchor))
	{
		ActivePiece->Destroy();
		ActivePiece = nullptr;
		bGameOver = true;
		OnGameOver();
		return;
	}

	TArray<FIntPoint> Offsets;
	ActivePiece->GetRotatedCellOffsets(ActivePiece->GetRotationStep(), Offsets);

	for (const FIntPoint& Offset : Offsets)
	{
		const FIntPoint Coordinate = Anchor + Offset;
		if (!IsInsidePlayableGrid(Coordinate))
		{
			ActivePiece->Destroy();
			ActivePiece = nullptr;
			bGameOver = true;
			OnGameOver();
			return;
		}
	}

	TSubclassOf<ATetrisCell> CellClass = ActivePiece->GetLockedCellClass();
	if (!CellClass)
	{
		CellClass = DefaultCellClass;
	}

	if (!CellClass)
	{
		UE_LOG(LogTemp, Error, TEXT("No LockedCellClass or DefaultCellClass was assigned."));
		ActivePiece->Destroy();
		ActivePiece = nullptr;
		bGameOver = true;
		OnGameOver();
		return;
	}

	TArray<TObjectPtr<ATetrisCell>> SpawnedCells;
	TArray<FIntPoint> SpawnedCoordinates;
	SpawnedCells.Reserve(4);
	SpawnedCoordinates.Reserve(4);

	for (const FIntPoint& Offset : Offsets)
	{
		const FIntPoint Coordinate = Anchor + Offset;
		const FTransform CellTransform(FRotator::ZeroRotator, GridToWorld(Coordinate));

		ATetrisCell* Cell = GetWorld()->SpawnActorDeferred<ATetrisCell>(
			CellClass,
			CellTransform,
			this,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		if (IsValid(Cell))
		{
			Cell->InitializeCell(this, Coordinate);
			UGameplayStatics::FinishSpawningActor(Cell, CellTransform);
			SpawnedCells.Add(Cell);
			SpawnedCoordinates.Add(Coordinate);
		}
		else
		{
			for (ATetrisCell* SpawnedCell : SpawnedCells)
			{
				if (IsValid(SpawnedCell))
				{
					SpawnedCell->Destroy();
				}
			}

			ActivePiece->Destroy();
			ActivePiece = nullptr;
			bGameOver = true;
			OnGameOver();
			return;
		}
	}

	for (int32 Index = 0; Index < SpawnedCells.Num(); ++Index)
	{
		OccupiedCells.Add(SpawnedCoordinates[Index], SpawnedCells[Index]);
	}

	ActivePiece->Destroy();
	ActivePiece = nullptr;

	CheckForCompletedLines();
}

bool ATetrisBoard::IsInsidePlayableGrid(const FIntPoint& Coordinate) const
{
	return Coordinate.X >= 0
		&& Coordinate.X < BoardWidth
		&& Coordinate.Y >= 0
		&& Coordinate.Y < BoardHeight;
}

void ATetrisBoard::CheckForCompletedLines()
{
	TArray<int32> CompletedRows;

	for (int32 Row = 0; Row < BoardHeight; ++Row)
	{
		bool bRowComplete = true;

		for (int32 Column = 0; Column < BoardWidth; ++Column)
		{
			const TObjectPtr<ATetrisCell>* FoundCell =
				OccupiedCells.Find(FIntPoint(Column, Row));

			if (FoundCell == nullptr || !IsValid(FoundCell->Get()))
			{
				bRowComplete = false;
				break;
			}
		}

		if (bRowComplete)
		{
			CompletedRows.Add(Row);
		}
	}

	if (CompletedRows.IsEmpty())
	{
		ScheduleNextPiece(NextPieceDelay);
		return;
	}

	StartLineClearWarning(CompletedRows);
}

void ATetrisBoard::StartLineClearWarning(const TArray<int32>& CompletedRows)
{
	PendingClearRows = CompletedRows;
	PendingClearCells.Reset();

	for (const int32 Row : PendingClearRows)
	{
		for (int32 Column = 0; Column < BoardWidth; ++Column)
		{
			if (TObjectPtr<ATetrisCell>* Cell =
				OccupiedCells.Find(FIntPoint(Column, Row)))
			{
				if (IsValid(Cell->Get()))
				{
					PendingClearCells.Add(Cell->Get());
					Cell->Get()->BeginClearWarning();
				}
			}
		}
	}

	if (IsValid(LineWarningSound))
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			LineWarningSound,
			GetActorLocation());
	}

	OnLineClearWarning(PendingClearRows.Num());

	GetWorldTimerManager().SetTimer(
		LineClearTimerHandle,
		this,
		&ATetrisBoard::ClearCompletedLines,
		FMath::Max(LineClearWarningDuration, 0.01f),
		false);
}

void ATetrisBoard::ClearCompletedLines()
{
	const int32 NumberOfLines = PendingClearRows.Num();

	for (const int32 Row : PendingClearRows)
	{
		for (int32 Column = 0; Column < BoardWidth; ++Column)
		{
			OccupiedCells.Remove(FIntPoint(Column, Row));
		}
	}

	for (ATetrisCell* Cell : PendingClearCells)
	{
		if (IsValid(Cell))
		{
			Cell->Destroy();
		}
	}

	if (IsValid(LineClearSound))
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			LineClearSound,
			GetActorLocation());
	}

	CollapseRows(PendingClearRows);

	OnLinesCleared(NumberOfLines);
	PendingClearRows.Reset();
	PendingClearCells.Reset();

	ScheduleNextPiece(NextPieceDelay);
}

void ATetrisBoard::CollapseRows(const TArray<int32>& ClearedRows)
{
	TArray<int32> SortedRows = ClearedRows;
	SortedRows.Sort();

	TArray<TPair<FIntPoint, TObjectPtr<ATetrisCell>>> RemainingCells;
	RemainingCells.Reserve(OccupiedCells.Num());

	for (const TPair<FIntPoint, TObjectPtr<ATetrisCell>>& Pair : OccupiedCells)
	{
		if (IsValid(Pair.Value))
		{
			RemainingCells.Add(Pair);
		}
	}

	OccupiedCells.Reset();

	for (const TPair<FIntPoint, TObjectPtr<ATetrisCell>>& Pair : RemainingCells)
	{
		int32 RowsBelow = 0;
		for (const int32 ClearedRow : SortedRows)
		{
			if (ClearedRow < Pair.Key.Y)
			{
				++RowsBelow;
			}
		}

		const FIntPoint NewCoordinate(Pair.Key.X, Pair.Key.Y - RowsBelow);
		Pair.Value->MoveToGridCoordinate(NewCoordinate, GridToWorld(NewCoordinate));
		OccupiedCells.Add(NewCoordinate, Pair.Value);
	}
	
}

bool ATetrisBoard::IsCellOccupied(int32 GridX, int32 GridY) const
{
	return OccupiedCells.Contains(FIntPoint(GridX, GridY));
}
