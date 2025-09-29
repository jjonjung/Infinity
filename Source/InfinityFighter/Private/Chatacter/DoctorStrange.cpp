// Fill out your copyright notice in the Description page of Project Settings.


#include "Chatacter/DoctorStrange.h"

#include "Camera/CameraActor.h"


// Sets default values
ADoctorStrange::ADoctorStrange()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SkillCamera = CreateDefaultSubobject<ACameraActor>("SkillCamera");
	
}

// Called when the game starts or when spawned
void ADoctorStrange::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADoctorStrange::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ADoctorStrange::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

