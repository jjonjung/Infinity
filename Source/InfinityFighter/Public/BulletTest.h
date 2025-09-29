// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "BulletTest.generated.h"

UCLASS()
class INFINITYFIGHTER_API ABulletTest : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABulletTest();
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* bulletMeshComp;
	UPROPERTY(EditAnywhere)
	class UBoxComponent* boxComp;
	UPROPERTY(EditAnywhere)
	class UProjectileMovementComponent* moveComp;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
