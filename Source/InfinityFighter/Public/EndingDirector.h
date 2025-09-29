// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Match/MatchResults.h"
#include "EndingDirector.generated.h"

UCLASS()
class INFINITYFIGHTER_API AEndingDirector : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEndingDirector();


public:
	// 시상용 캐릭터
	UPROPERTY(EditAnywhere, Category="Spawn")
	TSubclassOf<class ACharacter> DisplayCharacterClass;
	//캐릭터 닉네임으로 불러오기
	UPROPERTY(EditAnywhere, Category="Spawn")
	TMap<FName, TSoftClassPtr<ACharacter>> NameToDisplayClass;

	// 1~3등 단상 위치/회전
	UPROPERTY(EditAnywhere, Category="Layout")
	TArray<FTransform> PodiumSpots; // [0]=1등, [1]=2등, [2]=3등
	UPROPERTY(EditInstanceOnly, Category="Layout")
	TArray<TObjectPtr<AActor>> PodiumAnchors;

	UFUNCTION(CallInEditor, Category="Layout")
	void RefreshPodiumSpotsFromActors();

	// 모션
	UPROPERTY(EditAnywhere, Category="FX")
	class UAnimMontage* WinMontage;
	UPROPERTY(EditAnywhere, Category="FX")
	class UAnimMontage* ClapMontage;

	UPROPERTY(EditAnywhere, Category="Ceremony")
	float WinDelaySec = 0.0f;

	UPROPERTY(EditAnywhere, Category="Ceremony")
	float ClapDelaySec = 0.5f;

	UPROPERTY(EditAnywhere, Category="Ceremony")
	float HoldEpsilon = 0.02f; // 끝에서 이만큼 앞에서 멈추기(초)

	void PlayAfterDelayAndHold(ACharacter* Char, UAnimMontage* Montage, float DelaySec);

	// 결과 주입
	UFUNCTION(BlueprintCallable, Category="Ceremony")
	void PlayCeremonyWith(const FMatchResult& Result);


	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	
private:
	// 저장해 둔 결과(내부/외부 주입 공통 사용)
	FMatchResult CachedResult;

	// 스폰된 디스플레이 캐릭터
	UPROPERTY(Transient)
	TArray<ACharacter*> SpawnedChars;
	void SpawnTop3(const FMatchResult& R);
	void PlayPoses();
	TSubclassOf<ACharacter> ResolveDisplayClassFor(const FPlayerMatchStat& Stat) const;


};
