#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TetrisBoard.generated.h"

class ATetrisCell;
class ATetrisPiece;
class USceneComponent;
class USoundBase;

UCLASS()
class TETRISHORROR_API ATetrisBoard : public AActor
{
	GENERATED_BODY()

public:
	ATetrisBoard();

	UFUNCTION(BlueprintCallable, Category = "Tetris Board|Control")
	bool TryMoveActivePiece(int32 Direction);

	UFUNCTION(BlueprintCallable, Category = "Tetris Board|Control")
	bool TryRotateActivePiece(int32 Direction);

	UFUNCTION(BlueprintCallable, Category = "Tetris Board|Control")
	void DropActivePiece();

	void NotifyPieceImpact(ATetrisPiece* Piece);

	UFUNCTION(BlueprintPure, Category = "Tetris Board")
	ATetrisPiece* GetActivePiece() const { return ActivePiece.Get(); }

	UFUNCTION(BlueprintPure, Category = "Tetris Board")
	FVector GridToWorld(FIntPoint GridCoordinate) const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tetris Board")
	TObjectPtr<USceneComponent> SceneRoot;

	// Put this actor at the centre of grid cell (column 0, row 0).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tetris Board|Grid",
		meta = (ClampMin = "4"))
	int32 BoardWidth = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tetris Board|Grid",
		meta = (ClampMin = "4"))
	int32 BoardHeight = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tetris Board|Grid",
		meta = (ClampMin = "1.0"))
	float CellSize = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tetris Board|Spawn",
		meta = (ClampMin = "4"))
	int32 SpawnHeightRows = 23;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tetris Board|Spawn",
		meta = (ClampMin = "0.0"))
	float FirstSpawnDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tetris Board|Spawn",
		meta = (ClampMin = "0.0"))
	float NextPieceDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tetris Board|Spawn")
	TArray<TSubclassOf<ATetrisPiece>> PieceClasses;

	// Used when a piece Blueprint does not specify its own LockedCellClass.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tetris Board|Spawn")
	TSubclassOf<ATetrisCell> DefaultCellClass;

	// Let the physical piece bounce/settle briefly before converting it into grid cells.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tetris Board|Landing",
		meta = (ClampMin = "0.0"))
	float SettleDelay = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tetris Board|Line Clear",
		meta = (ClampMin = "0.0"))
	float LineClearWarningDuration = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Tetris Board|Audio")
	TObjectPtr<USoundBase> LineWarningSound;

	UPROPERTY(EditAnywhere, Category = "Tetris Board|Audio")
	TObjectPtr<USoundBase> LineClearSound;

	UFUNCTION(BlueprintImplementableEvent, Category = "Tetris Board|Events")
	void OnLineClearWarning(int32 NumberOfLines);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tetris Board|Events")
	void OnLinesCleared(int32 NumberOfLines);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tetris Board|Events")
	void OnGameOver();

private:
	void SpawnNextPiece();
	void ScheduleNextPiece(float Delay);
	void LockActivePiece();
	void CheckForCompletedLines();
	void StartLineClearWarning(const TArray<int32>& CompletedRows);
	void ClearCompletedLines();
	void CollapseRows(const TArray<int32>& ClearedRows);

	bool CanPlacePieceAt(
		const ATetrisPiece* Piece,
		const FIntPoint& Anchor,
		int32 RotationStep) const;

	bool FindFreeLockAnchor(
		const ATetrisPiece* Piece,
		FIntPoint& InOutAnchor) const;

	bool IsInsidePlayableGrid(const FIntPoint& Coordinate) const;
	FIntPoint WorldToGrid(const FVector& WorldLocation) const;

	UPROPERTY(Transient)
	TObjectPtr<ATetrisPiece> ActivePiece;

	UPROPERTY(Transient)
	TMap<FIntPoint, TObjectPtr<ATetrisCell>> OccupiedCells;

	UPROPERTY(Transient)
	TArray<TObjectPtr<ATetrisCell>> PendingClearCells;

	UPROPERTY(Transient)
	TArray<int32> PendingClearRows;

	FTimerHandle SpawnTimerHandle;
	FTimerHandle LockTimerHandle;
	FTimerHandle LineClearTimerHandle;

	bool bGameOver = false;
};
