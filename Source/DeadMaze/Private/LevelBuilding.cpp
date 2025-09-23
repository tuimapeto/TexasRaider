// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelBuilding.h"

// Sets default values
ALevelBuilding::ALevelBuilding()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALevelBuilding::BeginPlay()
{
	Super::BeginPlay();
	int v = 6;
	
}

// Called every frame
void ALevelBuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

