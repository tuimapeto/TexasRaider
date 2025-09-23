// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAI.h"

// Sets default values
AEnemyAI::AEnemyAI()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemyAI::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemyAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//UE_LOG(LogTemp, Warning, TEXT("TEST!"));
	if (actionState == EEnemyActionState::IsAttacking)
	{
		HandleAttack();
	}

}

void AEnemyAI::HandleAttack()
{
	// FVector playerLocation = GetWorld()->GetFirstPlayerController()->GetCharacter()->GetActorLocation();
	//
	// FRotator newRot = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), playerLocation);
	//
	// SetActorRotation(newRot);
    
	//UE_LOG(LogTemp, Warning, TEXT("ROTATE!"));
	
	
}

