// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "InfinityFightersGameModeBase.generated.h"

class ACharacterBase;
class ASpawnPointBase;
class UGameUIWidget;

UCLASS()
class INFINITYFIGHTER_API AInfinityFightersGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AInfinityFightersGameModeBase();

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void RequestPlayerRespawn(AController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Respawn Protection")
	void ApplyRespawnProtection(class ACharacterBase* Character);
	
	UFUNCTION(BlueprintCallable, Category = "Respawn Protection")
	void RemoveRespawnProtection(class ACharacterBase* Character);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn Protection")
	float RespawnProtectionDuration = 3.0f;
	
	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void RequestAIRespawn(AController* AIController);

	// 스폰 포인트 타입별 분류
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void ClassifySpawnPointsByType();

	void PauseAllCharacters();
	void ResumeAllCharacters();
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Aim")
	void SetupAimForAllEnemies();
	
	// BP_Aim (사용 안함 - CharacterBase Widget Component로 대체됨)
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aim")
	// TSubclassOf<UUserWidget> BPAimClass;

	// BP_AimEnemy (사용 안함 - CharacterBase Widget Component로 대체됨)
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aim")
	// TSubclassOf<AActor> BPAimEnemyClass;


	// UPROPERTY()
	// AActor* SpawnedBPAim;

	// 적 캐릭터들과 연결된 BP_AimEnemy들 (사용 안함)
	// UPROPERTY()
	// TMap<ACharacterBase*, AActor*> EnemyAimMap;

	//리스폰
	UPROPERTY(EditAnywhere, Category = "Respawn")
	TArray<FVector> SpawnLocations;
	
	void StartEnemySpawnLocations();
	void RegisterAllCharactersWithManager();

	UPROPERTY(EditInstanceOnly, Category="Respawn")
	TObjectPtr<AActor> EnemySpawnActor;

	UPROPERTY(EditInstanceOnly, Category="Respawn")
	TObjectPtr<AActor> PlayerSpawnActor;

	// 스폰 포인트 관리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TArray<AActor*> AvailableSpawnPoints;

	UPROPERTY()
	TArray<AActor*> UsedSpawnPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawn")
	TArray<TObjectPtr<ASpawnPointBase>> AISpawnPoints;
	// 스폰 위치 할당
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	FVector GetSpawnLocation();

	UPROPERTY()
	TArray<AActor*> UsedPlayerSpawnPoints;

	UPROPERTY()
	TArray<AActor*> UsedAISpawnPoints;

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void CollectAllSpawnPoints();

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	FVector GetRandomAvailableSpawnLocation();

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	FVector GetRandomSpawnLocationWithMinDistance(float MinDistance = 500.0f);

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void ResetSpawnPoints();

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void SpawnCharactersAtRandomLocations();

//엔딩관련(킬로그 관련)
public:
	UFUNCTION(BlueprintCallable)
	void OnMatchEnded();

	// UI 타이머 시작
	UFUNCTION(BlueprintCallable)
	void StartUITimer();

private:
	FTimerHandle MatchTimerHandle;
	float MatchDuration = 120.f; // 2분
	bool bEnded = false;

	void HandleMatchTimeout(); // 타이머 0 됐을 때 호출
};
