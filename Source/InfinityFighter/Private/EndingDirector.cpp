// Fill out your copyright notice in the Description page of Project Settings.


#include "EndingDirector.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/ManagerController.h"

// Sets default values
AEndingDirector::AEndingDirector()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AEndingDirector::BeginPlay()
{
	Super::BeginPlay();
	
	// 1) GameInstance에서 결과 자동 로드 (엔딩 맵으로 이미 넘어온 뒤)
	if (const UManagerController* GI = GetGameInstance<UManagerController>())
	{
		CachedResult = GI->CachedMatchResult;
		UE_LOG(LogTemp, Warning, TEXT("EndingDirector: CachedMatchResult.bValid = %s, PlayerStats.Num() = %d"), CachedResult.bValid ? TEXT("True") : TEXT("False"), CachedResult.PlayerStats.Num());
	}

	if (!CachedResult.bValid) return;
	PlayCeremonyWith(CachedResult);
}


void AEndingDirector::PlayAfterDelayAndHold(ACharacter* Char, UAnimMontage* Montage, float DelaySec)
{
	
	if (!Char || !Montage) return;

	TWeakObjectPtr<ACharacter>     WChar = Char;
	TWeakObjectPtr<UAnimMontage>   WM    = Montage;

	FTimerHandle StartHandle;
	GetWorldTimerManager().SetTimer(StartHandle, [this, WChar, WM]()
	{
		ACharacter* C = WChar.Get();
		UAnimMontage* M = WM.Get();
		if (!C || !M) return;

		UAnimInstance* Anim = C->GetMesh() ? C->GetMesh()->GetAnimInstance() : nullptr;
		if (!Anim) return;

		const float Duration = Anim->Montage_Play(M, 1.0f);
		if (Duration <= 0.f) return;

		const float HoldEpsilon = 0.02f;
		const float PauseAt = FMath::Max(0.f, Duration - HoldEpsilon);

		FTimerHandle PauseHandle;
		GetWorldTimerManager().SetTimer(PauseHandle, [Anim, M, PauseAt]()
		{
			if (!Anim || !M) return;
			Anim->Montage_SetPosition(M, PauseAt);
			Anim->Montage_Pause(M);
		}, PauseAt, false);

	}, DelaySec, false);
}

void AEndingDirector::PlayCeremonyWith(const FMatchResult& Result)
{
	CachedResult = Result;
	SpawnTop3(CachedResult);
	PlayPoses();
}

void AEndingDirector::SpawnTop3(const FMatchResult& R)
{
	const int32 Count = FMath::Min(3, R.PlayerStats.Num());
	SpawnedChars.Reset();

	UE_LOG(LogTemp, Warning, TEXT("EndingDirector: SpawnTop3 called. PlayerStats.Num() = %d, Effective Count = %d"), R.PlayerStats.Num(), Count);

	for (int32 i = 0; i < Count; ++i)
	{
		if (!PodiumSpots.IsValidIndex(i)) {
			UE_LOG(LogTemp, Error, TEXT("EndingDirector: PodiumSpots[%d] is invalid."), i);
			continue;
		}

		const FPlayerMatchStat& S = R.PlayerStats[i];  
		TSubclassOf<ACharacter> ClassToSpawn = ResolveDisplayClassFor(S);
		if (!ClassToSpawn) {
			UE_LOG(LogTemp, Error, TEXT("EndingDirector: ClassToSpawn is null for player %s (Rank %d). Using DisplayCharacterClass."), *S.PlayerId, i+1);
			ClassToSpawn = DisplayCharacterClass;
		}

		if (!ClassToSpawn) {
			UE_LOG(LogTemp, Error, TEXT("EndingDirector: ClassToSpawn is still null after fallback for player %s (Rank %d). Skipping spawn."), *S.PlayerId, i+1);
			continue;
		}

		const FTransform& T = PodiumSpots[i];
		UE_LOG(LogTemp, Warning, TEXT("EndingDirector: Spawning player %s (Rank %d) at location %s"), *S.PlayerId, i+1, *T.GetLocation().ToString());

		ACharacter* C = GetWorld()->SpawnActorDeferred<ACharacter>(
			ClassToSpawn, T, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
		);
		if (!C) continue;

		// 디버그 태그 & 닉네임 태그
		C->Tags.Add(*FString::Printf(TEXT("Rank_%d"), i + 1));
		if (!S.PlayerId.IsEmpty())
		{
			C->Tags.Add(*S.PlayerId);
		}

		UGameplayStatics::FinishSpawningActor(C, T);
		SpawnedChars.Add(C);
	}
}

void AEndingDirector::PlayPoses()
{
	if (SpawnedChars.Num() > 0) PlayAfterDelayAndHold(SpawnedChars[0], WinMontage,  WinDelaySec);  // 1등
	if (SpawnedChars.Num() > 1) PlayAfterDelayAndHold(SpawnedChars[1], ClapMontage, ClapDelaySec); // 2등
	if (SpawnedChars.Num() > 2) PlayAfterDelayAndHold(SpawnedChars[2], ClapMontage, ClapDelaySec); // 3등

}

TSubclassOf<ACharacter> AEndingDirector::ResolveDisplayClassFor(const FPlayerMatchStat& Stat) const
{
	// 1) 닉네임 매핑 우선
	const FName Key = FName(*Stat.PlayerId);  
	if (const TSoftClassPtr<ACharacter>* Found = NameToDisplayClass.Find(Key))
	{
		if (Found->IsValid())
		{
			return Found->Get();
		}
		//  로드
		if (UClass* Loaded = Found->LoadSynchronous())
		{
			return Loaded;
		}
	}
	// 2) 실패 시
	return DisplayCharacterClass;
}

void AEndingDirector::RefreshPodiumSpotsFromActors()
{
	PodiumSpots.Reset();

	UE_LOG(LogTemp, Warning, TEXT("EndingDirector: PodiumAnchors.Num() = %d"), PodiumAnchors.Num());
	for (AActor* Anchor : PodiumAnchors)
	{
		if (!IsValid(Anchor)) continue;
		PodiumSpots.Add(Anchor->GetActorTransform());
	}
	UE_LOG(LogTemp, Warning, TEXT("EndingDirector: PodiumSpots.Num() after refresh = %d"), PodiumSpots.Num());
	
}



