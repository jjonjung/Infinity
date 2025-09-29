// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/CharacterBase.h"
#include "GameFramework/Actor.h"
#include "AttachWall.generated.h"

UCLASS()
class INFINITYFIGHTER_API AAttachWall : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAttachWall();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	bool CheckInWall();
	UPROPERTY(EditDefaultsOnly)
	class UBoxComponent* BoxComp;
private:
	
	void IsWallNormalValid()                        ; 

	
	
};
