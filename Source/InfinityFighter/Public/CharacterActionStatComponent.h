// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DA/ActionStat.h"
#include "Components/ActorComponent.h"
#include "CharacterActionStatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INFINITYFIGHTER_API UCharacterActionStatComponent : public UActorComponent
{
public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeathDelegate, AActor*, DeadActor, AActor*, KillerActor);
	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnDeathDelegate OnDeath;

	int32 GetCurrentBullet() const;
	void SetCurrentBullet(int32 CurrentBullet);
	int32 Get_CurrentHp() const;
	void Set_CurrentHp(int32 CurrentHp);

private:
	GENERATED_BODY()
public:
	float GetAttackDamage() const;
	void SetAttackDamage(float AttackDamage);
	float GetAttackSpeed() const;
	void SetAttackSpeed(float AttackSpeed);
	int32 GetMaxBullet() const;
	void SetMaxBullet(int32 MaxBullet);
	int32 GetBulletBox() const;
	void SetBulletBox(int32 BulletBox);
	void SetCurrentBulletBox(int32 bulletBox);
	int32 GetCurrentBulletBox();
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float CalculateHpPercentage();



public:	
	// Sets default values for this component's properties
	UCharacterActionStatComponent();
 

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
private:
#pragma region 전투관련 스텟 수정 불가/DA를 통해서 초기화 
	float AttackDamage;
	float AttackSpeed;
	int32 MaxBullet;// 최대로 소유할 수 있는 탄알 수 
	int32 BulletBox;//한 탄창에 쏠 수 있는 탄알 수
	int32 _MaxHP;
#pragma endregion 
#pragma region 액션 관련 변수중 런타임에 변하는게 가능한 변수들 모음
	int32 CurrentBullet;//남은 전체 탄알 수 
	int32 _CurrentHp;
	int32 CurrentBulletBox; //현재 탄창에 남은 탄알 수

	UPROPERTY(EditDefaultsOnly,Blueprintable)
	bool IsInWall=false;
	UPROPERTY(EditDefaultsOnly,Blueprintable)
	bool IsFly = false;
#pragma endregion 
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void InitFromDa(const UActionStat* BasicStat);
	UFUNCTION(BlueprintCallable)
	void SetInWall(bool InWall);

	UFUNCTION(BlueprintPure,BlueprintCallable)
	bool GetInWall() const;
	UFUNCTION(BlueprintPure,BlueprintCallable)
	bool GetInFly();

	UFUNCTION(BlueprintCallable)
	void SetInFly(bool InFly);


	UFUNCTION(Category="stats")
	void ApplyDamage(float Amount, AActor*Causer = nullptr);
	
		
};
