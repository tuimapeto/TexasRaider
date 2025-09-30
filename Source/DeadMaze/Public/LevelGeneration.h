#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <queue>
#include <vector>
#include <tuple>
#include <random>
#include "LevelGeneration.generated.h"

struct Cell
{
    int x, y;
};

enum class RoomType
{
    OneExit = 0,
    TwoExit = 1,
    ThreeExit = 2,
    FourExit = 3
};

class Room
{
public:
    Room(RoomType roomType, int roomSizeX, int roomSizeY, float roomWorldLocX, float roomWorldLocY)
    {
        if (roomType == RoomType::TwoExit)
        {
            exits.reserve(2);
            
            exits.emplace_back(0, roomSizeY / 2);
            exits.emplace_back(roomSizeX - 1, roomSizeY / 2);

            m_roomType = roomType;
        }

        m_roomWorldLocation.X = roomWorldLocX;
        m_roomWorldLocation.Y = roomWorldLocY;
        m_roomWorldLocation.Z = 0;

        m_roomSizeX = roomSizeX;
        m_roomSizeY = roomSizeY;
    }

    FVector GetExitExactLocation(int exitIndex) const
    {
        if (exits.size() >= exitIndex)
        {
            FVector exitLocation;
            exitLocation.X = exits[exitIndex].x + m_roomWorldLocation.X;
            exitLocation.Y = exits[exitIndex].y + m_roomWorldLocation.Y;
            exitLocation.Z = 0;

            return exitLocation;    
        }

        else
        {
            return FVector(0, 0, 0);
        }
    }

    FVector GetRoomWorldLocation() const
    {
        return m_roomWorldLocation;
    }

    std::tuple<int, int> GetRoomSize() const
    {
        return std::make_tuple(m_roomSizeX, m_roomSizeY);
    }

    AActor* m_roomMesh;
    RoomType m_roomType;
    
private:
    std::vector<Cell> exits;
    
    FVector m_roomWorldLocation;

    int m_roomSizeX;
    int m_roomSizeY;

    
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

    UPROPERTY(EditAnywhere, Category = "Level Generation")
    TSubclassOf<AActor> RoomThreeExit;

    UPROPERTY(EditAnywhere, Category = "Level Generation")
    TSubclassOf<AActor> RoomOneExit;

    UPROPERTY(EditAnywhere, Category = "Level Generation")
    TSubclassOf<AActor> RoomTwoExit;

    UPROPERTY(EditAnywhere, Category = "Level Generation")
    TSubclassOf<AActor> RoomFourExit;
    
public:
    virtual void Tick(float DeltaTime) override;

private:
    void GenerateGrid();
    bool IsCellReachableAfterWall(int wx, int wy, Cell entrance, Cell exit);
    bool CheckRoomOverlap(const Room& extraRoom) const;

    void CreateRooms();
    void CreatePassages();

    const int MAZESIZE = 34;
    FString MazeGrid[34][34];

    std::vector<Room> generatedRooms;

    //constants
    const float WORLDLOCATIONOFFSET = 15000.0f;
    const float DEFAULTROOMSIZE = 2000.0f;
};