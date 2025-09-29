// Fill out your copyright notice in the Description page of Project Settings.


#include "Chatacter/IronMan.h"

#include "Components/BoxComponent.h"


// Sets default values
AIronMan::AIronMan()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	IronComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IronComp"));
	IronComp->SetupAttachment(RootComponent);

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	SetRootComponent(BoxComp);

	IronComp->SetGenerateOverlapEvents(false);
	IronComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
}

// Called when the game starts or when spawned
void AIronMan::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AIronMan::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Center = BoxComp->GetComponentLocation();
	FVector Extent = BoxComp->GetScaledBoxExtent();
	FQuat Rot = BoxComp->GetComponentQuat();

	// collisionBox
	DrawDebugBox(GetWorld(), Center, Extent, Rot, FColor::Red,
				 false,   // 지속선 여부 (false: LifeTime만큼만)
				 -1.0f,   // LifeTime (-1 = 한 프레임만, Tick에서 반복되므로 OK)
				 0,       // DepthPriority
				 2.0f);   // 두께
}

// Called to bind functionality to input
void AIronMan::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

