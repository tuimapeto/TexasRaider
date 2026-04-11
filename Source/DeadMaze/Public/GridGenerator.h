// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <queue>
#include <vector>
#include <tuple>
#include <random>
#include "GridGenerator.generated.h"

struct Cell
{
	int x, y;
};

struct ExitCell
{
	float x, y;
};

struct Node { int x, y; float g, h; Node* parent; };

UCLASS()
class DEADMAZE_API AGridGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridGenerator();

protected:
	// Wall, key, treasure classes you want to spawn
	UPROPERTY(EditAnywhere, Category = "Level Generation")
	TSubclassOf<AActor> WallClass;

	UPROPERTY(EditAnywhere, Category = "Level Generation")
	TSubclassOf<AActor> KeyClass;

	UPROPERTY(EditAnywhere, Category = "Level Generation")
	TSubclassOf<AActor> TreasureClass;

	UPROPERTY(EditAnywhere, Category = "Level Generation")
	TSubclassOf<AActor> PlantClass;

	UPROPERTY(EditAnywhere, Category = "Level Generation")
	TSubclassOf<AActor> TestClass;

	UPROPERTY(EditAnywhere, Category = "Level Generation")
	TSubclassOf<AActor> EnemyClass;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void GenerateGrid(FVector gridOffset);
	bool IsCellReachableAfterWall(int wx, int wy, Cell entrance, Cell exit);
	
private:
	const int MAZESIZE = 18;
	FString MazeGrid[18][18];
    
    void PlaceActorsInGrid(const FVector& gridOffset, const FString& objectClass, UClass* placedActor, float zOffset) const;
	bool HasNearbyWall(int x, int y, int radius) const;

};
