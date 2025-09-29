// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/SpawnPointBase.h"


// Sets default values
ASpawnPointBase::ASpawnPointBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASpawnPointBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASpawnPointBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

