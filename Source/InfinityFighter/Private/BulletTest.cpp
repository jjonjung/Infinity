// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletTest.h"

#include "Components/BoxComponent.h"


// Sets default values
ABulletTest::ABulletTest()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bulletMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
	boxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	moveComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MoveComp"));
	SetRootComponent(boxComp);
	bulletMeshComp->SetupAttachment(GetRootComponent());
	moveComp->InitialSpeed=5000.f;
	moveComp->MaxSpeed=5000.f;
	
}

// Called when the game starts or when spawned
void ABulletTest::BeginPlay()
{
	Super::BeginPlay();
	
	
}

// Called every frame
void ABulletTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

