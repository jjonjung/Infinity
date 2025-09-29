// Fill out your copyright notice in the Description page of Project Settings.


#include "AttachWall.h"

#include "Components/BoxComponent.h"

// Sets default values
AAttachWall::AAttachWall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	BoxComp=CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	RootComponent=BoxComp;

}

// Called when the game starts or when spawned
void AAttachWall::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAttachWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AAttachWall::CheckInWall()
{
	
	return true;
}





