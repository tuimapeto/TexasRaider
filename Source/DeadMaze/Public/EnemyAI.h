// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/KismetMathLibrary.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "EnemyAI.generated.h"

UENUM(BlueprintType)
enum class EEnemyActionState : uint8
{
	IsPatrolling = 0 UMETA(DisplayName = "Patrolling"),
	IsInvestigating  UMETA(DisplayName = "Investigating"),
	IsChasing        UMETA(DisplayName = "Chasing"),
	IsAttacking      UMETA(DisplayName = "Attacking"),
	IsDying          UMETA(DisplayName = "Dying")		
};

UENUM(BlueprintType)
enum class EKoState : uint8
{
	IsAlive  UMETA(DisplayName = "Alive"),
	IsDead   UMETA(DisplayName = "Dead")
};

UCLASS()
class DEADMAZE_API AEnemyAI : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemyAI();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action State")
	EEnemyActionState actionState = EEnemyActionState::IsPatrolling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KO State")
	EKoState KoState = EKoState::IsAlive;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void HandleAttack();

};
