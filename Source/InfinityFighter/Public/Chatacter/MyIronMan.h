// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/CharacterBase.h"
#include "IronManRouter.h"
#include "MyIronMan.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

/**
 * 아이언맨 캐릭터 - hand_lSocket에서 총알을 발사하는 전용 액션 사용
 */
UCLASS()
class INFINITYFIGHTER_API AMyIronMan : public ACharacterBase
{
	GENERATED_BODY()

public:
	AMyIronMan();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// Called when the character lands.
	virtual void Landed(const FHitResult& Hit) override;

	// Overridden from ACharacter to handle jump counting for flight.
	virtual void OnJumped_Implementation() override;

	//~ Replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// 아이언맨 전용 액션들 설정
	void SetupIronManActions();

	// Input Intent 핸들러
	UFUNCTION()
	void OnInputIntent(FName IntentTag);
	
protected:
	// --- FLIGHT CONTROLS ---

	// Toggles flight mode.
	void ToggleFlight();

	// Starts the flight sequence.
	UFUNCTION(Server, Reliable)
	void Server_StartFlying();

	// Stops the flight sequence.
	UFUNCTION(Server, Reliable)
	void Server_StopFlying();

	// Handles the state change on clients.
	UFUNCTION()
	void OnRep_IsFlying();
	
	// Input handlers for vertical movement during flight.
	void FlyUp(float Value);

	// --- FLIGHT PHYSICS ---

	// Applies flight forces during Tick.
	void UpdateFlight(float DeltaTime);

protected:
	// --- FLIGHT PROPERTIES ---

	// True if the character is currently flying.
	UPROPERTY(Transient, ReplicatedUsing = OnRep_IsFlying)
	bool bIsFlying = false;

	// The strength of the forward thrust.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight")
	float FlyThrust = 1500000.0f;

	// The maximum speed while flying.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight")
	float MaxFlySpeed = 4000.0f;

	// How quickly the character slows down when not thrusting (higher is faster).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flight")
	float FlyingBrakingStiffness = 5.0f;

	// The animation montage to play during flight.
	UPROPERTY(EditDefaultsOnly, Category = "Flight|Animation")
	class UAnimMontage* FlyingAnimMontage;

	// --- JUMP PROPERTIES ---

	// The upward velocity to apply on successive jumps.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump")
	float SuccessiveJumpZVelocity = 1000.0f;

	// --- FLIGHT EFFECTS ---

	// The Niagara particle system to use for the foot thrusters.
	UPROPERTY(EditDefaultsOnly, Category = "Flight|Effects")
	UNiagaraSystem* FootThrusterEffect;

private:
	// The active Niagara component for the left foot.
	UPROPERTY()
	UNiagaraComponent* LeftFootThrusterComponent;

	// The active Niagara component for the right foot.
	UPROPERTY()
	UNiagaraComponent* RightFootThrusterComponent;
};
