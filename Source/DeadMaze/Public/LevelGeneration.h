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

struct ExitCell
{
    float x, y;
};

struct Node { int x, y; float g, h; Node* parent; };

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
    Room(RoomType roomType, float roomSizeX, float roomSizeY, float roomWorldLocX, float roomWorldLocY)
    {
        if (roomType == RoomType::TwoExit)
        {
            exits.reserve(2);
            
            exits.emplace_back(-roomSizeX / 2.0f, 0);
            exits.emplace_back(-roomSizeX / 2.0f, 0);

            m_roomType = roomType;
        }

        else if (roomType == RoomType::OneExit)
        {
            exits.reserve(1);
            
            exits.emplace_back(-roomSizeX / 2.0f, 0);

            m_roomType = roomType;
        }

        else if (roomType == RoomType::ThreeExit)
        {
            exits.reserve(3);
            
            exits.emplace_back(0, roomSizeY / 2.0f);
            exits.emplace_back(roomSizeX - 1, roomSizeY / 2.0f);
            exits.emplace_back(roomSizeX / 2.0f, 0);

            m_roomType = roomType;
        }

        else if (roomType == RoomType::FourExit)
        {
            exits.reserve(4);
            
            exits.emplace_back(0, roomSizeY / 2.0f);
            exits.emplace_back(roomSizeX - 1, roomSizeY / 2.);
            exits.emplace_back(roomSizeX / 2.0f, 0);
            exits.emplace_back(roomSizeX / 2.0f, 0);

            m_roomType = roomType;
        }

        m_roomWorldLocation.X = roomWorldLocX;
        m_roomWorldLocation.Y = roomWorldLocY;
        m_roomWorldLocation.Z = 100.0f;

        m_roomSizeX = roomSizeX;
        m_roomSizeY = roomSizeY;
    }

    FVector GetExitExactLocation(int exitIndex) const
    {
        if (exits.size() >= exitIndex)
        {
            FVector exitLocation;
            exitLocation.X = (exits[exitIndex].x * 100.0f) + m_roomWorldLocation.X;
            exitLocation.Y = (exits[exitIndex].y * 100.0f) + m_roomWorldLocation.Y;
            exitLocation.Z = 100.0f;

            if (exitIndex == 0)
            {
                return exitLocation + FVector(-200.0f, 0, 0);
            }

            else if (exitIndex == 2)
            {
                return exitLocation + FVector(200.0f, 0, 0);
            }

            else
            {
                return exitLocation;    
            }
            
        }
        /*if (exits.size() >= exitIndex)
        {
            FVector exitLocation;
            exitLocation.X = m_roomWorldLocation.X - 2000.0f * ((m_roomSizeX * 100.0f) / 2000.0f);
            exitLocation.Y = m_roomWorldLocation.Y + 2000.0f * ((m_roomSizeY * 100.0f) / 2000.0f);
            exitLocation.Z = 0;
        
            return exitLocation;
        }*/

        else
        {
            return FVector(0, 0, 0);
        }
    }

    FVector GetRoomWorldLocation() const
    {
        return m_roomWorldLocation;
    }

    std::tuple<float, float> GetRoomSize() const
    {
        return std::make_tuple(m_roomSizeX, m_roomSizeY);
    }

    bool CheckRoomBounds(float pointX, float pointY) const
    {
        if (pointX > m_roomWorldLocation.X - (m_roomSizeX / 2.0f) * 100.0f && pointX < m_roomWorldLocation.X + (m_roomSizeX / 2.0f) * 100.0f
            && pointY > m_roomWorldLocation.Y - (m_roomSizeY / 2.0f) * 100.0f && pointY < m_roomWorldLocation.Y + (m_roomSizeY / 2.0f) * 100.0f)
        {
            return true;
        }

        else
        {
            return false;
        }
    }

    bool CheckRoomBoundsY(float point) const
    {
        if (point > m_roomWorldLocation.Y - (m_roomSizeY / 2.0f) * 100.0f && point < m_roomWorldLocation.Y + (m_roomSizeY / 2.0f) * 100.0f
            )
        {
            return true;
        }

        else
        {
            return false;
        }
    }


    AActor* m_roomMesh;
    RoomType m_roomType;
    
private:
    std::vector<ExitCell> exits;
    
    FVector m_roomWorldLocation;

    float m_roomSizeX;
    float m_roomSizeY;
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

    UPROPERTY(EditAnywhere, Category = "Level Generation")
    TSubclassOf<AActor> PassageBlock1CubicMeter;

    UPROPERTY(EditAnywhere, Category = "Level Generation")
    TSubclassOf<AActor> PassageBlock1CubicMeter2;

    UPROPERTY(EditAnywhere, Category = "Level Generation")
    TSubclassOf<AActor> BankVaultSystem;
    
public:
    virtual void Tick(float DeltaTime) override;

private:
    void GenerateGrid();
    bool IsCellReachableAfterWall(int wx, int wy, Cell entrance, Cell exit);
    bool CheckRoomOverlap(const Room& extraRoom) const;

    void CreateRooms();
    void CreatePassages();
    void ReserveRoomSpace(const Room& room);
    Cell WorldToGrid(const FVector& worldLocation) const;
    std::vector<FVector> FindCorridorPath(Cell start, Cell end, int exitIndexStart, int exitIndexEnd);
    void FillWalls() const;
    void DecorateRooms() const;
    void MakeExits();

    const int MAZESIZE = 34;
    FString MazeGrid[34][34];

    std::vector<Room> generatedRooms;
    const int worldSize = 150;
    FString worldOccupancyGrid[150][150];

    //constants
    const float WORLDLOCATIONOFFSET = 15000.0f;
    const float DEFAULTROOMSIZE = 2000.0f;
};