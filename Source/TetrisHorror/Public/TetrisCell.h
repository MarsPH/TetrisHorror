#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TetrisCell.generated.h"

class ATetrisBoard;
class UStaticMeshComponent;

UCLASS()
class TETRISHORROR_API ATetrisCell : public AActor
{
	GENERATED_BODY()

public:
	ATetrisCell();

	void InitializeCell(ATetrisBoard* InBoard, const FIntPoint& InGridCoordinate);
	void BeginClearWarning();
	void MoveToGridCoordinate(const FIntPoint& NewCoordinate, const FVector& NewWorldLocation);

	FIntPoint GetGridCoordinate() const { return GridCoordinate; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tetris Cell")
	TObjectPtr<UStaticMeshComponent> CellMesh;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tetris Cell")
	FIntPoint GridCoordinate = FIntPoint::ZeroValue;

	UFUNCTION(BlueprintImplementableEvent, Category = "Tetris Cell|Events")
	void OnClearWarningStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Tetris Cell|Events")
	void OnGridPositionChanged(FIntPoint NewCoordinate);

private:
	UPROPERTY(Transient)
	TObjectPtr<ATetrisBoard> Board;

	bool bMarkedForClear = false;
};
