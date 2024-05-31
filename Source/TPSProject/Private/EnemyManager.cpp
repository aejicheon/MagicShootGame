// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyManager.h"
#include "Enemy.h"
#include <EngineUtils.h>
#include <Kismet/GameplayStatics.h>

// Sets default values
AEnemyManager::AEnemyManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AEnemyManager::BeginPlay()
{
	Super::BeginPlay();

	float createTime = FMath::RandRange(minTime, maxTime);
	GetWorld()->GetTimerManager().SetTimer(spawnTimerHandle, this,
		&AEnemyManager::CreateEnemy, createTime);
	
	FindSpawnPoints();
}

// Called every frame
void AEnemyManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemyManager::CreateEnemy() {
	int index = FMath::RandRange(0, spawnPoints.Num() - 1);
	GetWorld()->SpawnActor<AEnemy>(enemyFactory, spawnPoints[index]->GetActorLocation(),
		FRotator(0));

	float createTime = FMath::RandRange(minTime, maxTime);
	GetWorld()->GetTimerManager().SetTimer(spawnTimerHandle, this,
		&AEnemyManager::CreateEnemy, createTime);
}


//void AEnemyManager::FindSpawnPoints() {
//	for (TActorIterator<AActor> It(GetWorld()); It; ++It) {
//		AActor* spawn = *It;
//		if (spawn->GetName().Contains(TEXT("BP_EnemySpawnPoint"))) {
//			spawnPoints.Add(spawn);
//		}
//	}
//}

void AEnemyManager::FindSpawnPoints() {
	TArray<AActor*> allActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), allActors);
	for (auto spawn : allActors) {
		if (spawn->GetName().Contains(TEXT("BP_EnemySpawnPoint"))) {
			spawnPoints.Add(spawn);
		}
	}
}
