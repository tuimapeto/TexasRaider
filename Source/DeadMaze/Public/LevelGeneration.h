#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <queue>
#include <vector>
#include "LevelGeneration.generated.h"

struct Cell
{
    int x, y;
};

enum class RoomType
{
    TwoExit = 0,
    ThreeExit = 1,
    FourExit = 2
};

class Room
{
public:
    Room(RoomType roomType, int roomSizeX, int roomSizeY, int roomWorldLocX, int roomWorldLocY)
    {
        if (roomType == RoomType::TwoExit)
        {
            exits.reserve(2);
            
            exits.emplace_back(0, roomSizeY / 2);
            exits.emplace_back(roomSizeX - 1, roomSizeY / 2);
        }

        m_roomWorldLocationX = roomWorldLocX;
        m_roomWorldLocationY = roomWorldLocY;
    }

    FVector GetExitExactLocation(int exitIndex) const
    {
        if (exits.size() >= exitIndex)
        {
            FVector exitLocation;
            exitLocation.X = exits[exitIndex].x + m_roomWorldLocationX;
            exitLocation.Y = exits[exitIndex].y + m_roomWorldLocationY;
            exitLocation.Z = 0;

            return exitLocation;    
        }

        else
        {
            return FVector(0, 0, 0);
        }
    }
private:
    std::vector<Cell> exits;
    
    int m_roomWorldLocationX = 0;
    int m_roomWorldLocationY = 0;
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