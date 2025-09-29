// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CMoveRouter.generated.h"

// Forward declaration to resolve ACharacter in TWeakObjectPtr
class ACharacter;

/**
 * 공통 이동 실행부 (SRP) 
 * - OOP/SOLID(SRP). 플레이어/AI가 같은 경로로 이동 => 협업/유지보수 용이
 * - 3D 프로그래밍: 월드/로컬 벡터 사용, AddMovementInput 사용
 * - 네트워크 확장 포인트: 추후 Server_RPC 진입점이 이 레이어 앞단에 추가되기 쉬움
 */
// OOP: 이동 로직을 Router로 캡슐화(SRP). 유닛 테스트/교체 용이(OCP)
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INFINITYFIGHTER_API UCMoveRouter : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCMoveRouter();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	//상속받는 캐릭터 데이터 캐싱
	UPROPERTY()
	TWeakObjectPtr<ACharacter> OwnerChar;
	class ACharacterBase* OwnerCharacterBase;

	// Brain에서만 호출: 실제 이동 실행부
	UFUNCTION(BlueprintCallable, Category="Router")
	void HandleMoveInput(float Forward, float Right);
	FVector GetWallTangentVector(ACharacter* C);
	void HandleWallMovement(ACharacter* C, float Forward, float Right);

	UFUNCTION()
	void HandleLookInput(float Yaw, float Pitch);

	bool isRunning = false;
	 
};
