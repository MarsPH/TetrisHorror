#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TetrisRemoteComponent.generated.h"

class ATetrisBoard;

UCLASS(ClassGroup = (Tetris), meta = (BlueprintSpawnableComponent))
class TETRISHORROR_API UTetrisRemoteComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTetrisRemoteComponent();

	UFUNCTION(BlueprintCallable, Category = "Tetris Remote")
	void MoveLeft();

	UFUNCTION(BlueprintCallable, Category = "Tetris Remote")
	void MoveRight();

	UFUNCTION(BlueprintCallable, Category = "Tetris Remote")
	void RotateClockwise();

	UFUNCTION(BlueprintCallable, Category = "Tetris Remote")
	void RotateCounterClockwise();

	UFUNCTION(BlueprintCallable, Category = "Tetris Remote")
	void DropNow();

	UFUNCTION(BlueprintPure, Category = "Tetris Remote")
	bool IsCellOccupied(int32 GridX, int32 GridY) const;

	UFUNCTION(BlueprintPure, Category = "Tetris Remote")
	bool IsCellPartOfActivePiece(int32 GridX, int32 GridY) const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tetris Remote")
	TObjectPtr<ATetrisBoard> Board;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tetris Remote")
	bool bFindBoardAutomatically = true;
};
