// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LobbyAvatarCharacter.generated.h"

UCLASS()
class INFINITYFIGHTER_API ALobbyAvatarCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ALobbyAvatarCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	


	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


public:
	UPROPERTY(EditAnywhere, Category = "Lobby")
	float LoobyCharScale = 0.4f;

public:
	virtual void Tick(float DeltaSeconds) override;

	/** 3초마다 재생할 애니메이션 시퀀스  */
	UPROPERTY(EditAnywhere, Category="Anim")
	UAnimSequence* BoredAnim = nullptr;

	/** AnimGraph에 추가한 Slot 이름 */
	UPROPERTY(EditAnywhere, Category="Anim")
	FName SlotName = TEXT("DefaultSlot");

	/** Idle 판정 속도  */
	UPROPERTY(EditAnywhere, Category="Anim")
	float IdleSpeedThreshold = 3.0f;

	/** 전환 블렌드 시간 */
	UPROPERTY(EditAnywhere, Category="Anim")
	float BlendIn  = 0.25f;
	UPROPERTY(EditAnywhere, Category="Anim")
	float BlendOut = 0.25f;

private:
	void StartBoredTimer();   // Idle 진입 시 시작
	void StopBoredTimer();    // Idle 탈출 시 정지
	void PlayBoredOnce();     // 실제 1회 재생

	bool bIsIdle = false;
	FTimerHandle BoredTimer;

	UPROPERTY(Transient)
	bool bBoredPlaying = false;
	UPROPERTY(Transient)
	bool bWasEffectiveIdle = false;
	FTimerHandle IdleWaitTimer;

	void StartIdleWaitTimer(float Seconds = 3.f);
	void StopIdleWaitTimer();
	void OnBoredMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
