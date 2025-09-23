#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <queue>
#include "LevelGeneration.generated.h"

struct Cell
{
    int x, y;
};

UCLASS()
class DEADMAZE_API ALevelGeneration : public AActor
{
    GENERATED_BODY()

public:
    ALevelGeneration();

protected:
    virtual void BeginPlay() override;

    // Wall, key, treasure classes you want to spawn
    UPROPERTY(EditAnywhere, Category = "Level Generation")
    TSubclassOf<AActor> WallClass;

    UPROPERTY(EditAnywhere, Category = "Level Generation")
    TSubclassOf<AActor> KeyClass;

    UPROPERTY(EditAnywhere, Category = "Level Generation")
    TSubclassOf<AActor> TreasureClass;
    
public:
    virtual void Tick(float DeltaTime) override;

private:
    void GenerateLevel();
    bool IsCellReachableAfterWall(int wx, int wy, Cell entrance, Cell exit);

    const int MAZESIZE = 34;
    FString MazeGrid[34][34];
};