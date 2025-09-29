// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DoctorStrange.generated.h"

UCLASS()
class INFINITYFIGHTER_API ADoctorStrange : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADoctorStrange();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	class ACameraActor* SkillCamera; 
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	
};
