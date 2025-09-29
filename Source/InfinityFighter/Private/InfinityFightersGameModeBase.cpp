// Fill out your copyright notice in the Description page of Project Settings.

#include "InfinityFightersGameModeBase.h"

#include "Base/CharacterBase.h"
#include "Component/CBrainComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Match/MatchResults.h"
#include "Match/MatchStatsFunctionLibrary.h"
#include "Skill/SpawnPointBase.h"
#include "Utility/ManagerController.h"
#include "Chatacter/MyIronMan.h"
#include "Chatacter/MySpiderMan.h"
#include "Chatacter/MyDoctorStrange.h"
#include "Components/WidgetComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Components/SphereComponent.h"

AInfinityFightersGameModeBase::AInfinityFightersGameModeBase()
{
	// 기본 설정

	// 기본 설정 완료
}


void AInfinityFightersGameModeBase::BeginPlay()
{
	Super::BeginPlay();


	SetupAimForAllEnemies();
	
	// 스폰 포인트 수집 및 분류
	CollectAllSpawnPoints();
	ClassifySpawnPointsByType();

	FTimerHandle PauseTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(PauseTimerHandle, [this]()
	{
		PauseAllCharacters();
        
		FTimerHandle ResumeTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(ResumeTimerHandle, [this]()
		{
			if (!IsValid(this)) return; // `this` 포인터 유효성 검사 추가
			// 캐릭터 움직임 재개
			ResumeAllCharacters();
            
			FTimerHandle AITimerHandle;
			GetWorld()->GetTimerManager().SetTimer(AITimerHandle, [this]()
			{
				SetupAimForAllEnemies();
			}, 1.0f, false);

			GetWorldTimerManager().SetTimer(
				MatchTimerHandle, this, &AInfinityFightersGameModeBase::HandleMatchTimeout, MatchDuration, false
			);

			// UI 타이머 시작
			StartUITimer();
            
		}, 3.0f, false);
        
	}, 0.6f, false);

	// 적 스폰 위치 초기화
	StartEnemySpawnLocations();
	RegisterAllCharactersWithManager();

	// 2분 카운트다운 
	GetWorldTimerManager().SetTimer(
		MatchTimerHandle, this, &AInfinityFightersGameModeBase::HandleMatchTimeout, MatchDuration, false
	);
}


void AInfinityFightersGameModeBase::PauseAllCharacters()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("PauseAllCharacters: World is null. Cannot pause characters."));
		return;
	}

	for (TActorIterator<ACharacterBase> ActorItr(World); ActorItr; ++ActorItr)
	{
		ACharacterBase* Character = *ActorItr;
		if (Character)
		{
			if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
			{
				MovementComp->DisableMovement();
			}

			if (AController* Controller = Character->GetController())
			{
				Controller->SetActorTickEnabled(false);
			}

			if (Character->crosshairUI) // public이거나 getter 함수가 있다고 가정
			{
				Character->crosshairUI->SetVisibility(ESlateVisibility::Hidden);
			}

			if (UWidgetComponent* WC = Character->AimEnemyWidget)
			{
				WC->SetVisibility(false);
				if (UUserWidget* W = WC->GetUserWidgetObject())
					W->SetVisibility(ESlateVisibility::Hidden);
			}

			// AI Brain Tick 비활성화
			if (UCBrainComponent* BrainComp = Character->FindComponentByClass<UCBrainComponent>())
			{
				BrainComp->bBrainActive = false;
			}
		}
	}
}

void AInfinityFightersGameModeBase::ResumeAllCharacters()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("ResumeAllCharacters: World is null. Cannot resume characters."));
		return;
	}
	for (TActorIterator<ACharacterBase> ActorItr(World); ActorItr; ++ActorItr)
	{
		ACharacterBase* Character = *ActorItr;
		if (Character)
		{
			if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
			{
				MovementComp->SetMovementMode(MOVE_Walking);
			}

			if (AController* Controller = Character->GetController())
			{
				Controller->SetActorTickEnabled(true);
			}

			if (Character->crosshairUI)
			{
				Character->crosshairUI->SetVisibility(ESlateVisibility::Visible);
			}

			if (UWidgetComponent* WC = Character->AimEnemyWidget)
			{
				WC->SetVisibility(true);
				if (UUserWidget* W = WC->GetUserWidgetObject())
					W->SetVisibility(ESlateVisibility::Visible);
			}

			// AI Brain Tick 활성화
			if (UCBrainComponent* BrainComp = Character->FindComponentByClass<UCBrainComponent>())
			{
				BrainComp->bBrainActive = true;
			}
		}
	}
}

void AInfinityFightersGameModeBase::SetupAimForAllEnemies()
{

	for (TActorIterator<ACharacterBase> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		ACharacterBase* Character = *ActorItr;

		if (!Character)
		{
			continue;
		}

		UE_LOG(LogTemp, Log, TEXT("FindComponentByClass 전까진 들엉옴: %s"), *Character->GetName());
		// AI 모드인 캐릭터 확인
		UCBrainComponent* BrainComp = Character->FindComponentByClass<UCBrainComponent>();
		if (BrainComp && BrainComp->Mode == EBrainMode::AI)
		{
			// AI 캐릭터는 이제 시야각 체크 시스템에 의해 자동으로 관리됨
			// 초기에는 모든 위젯이 숨겨져 있고, 플레이어 시야각에 들어올 때만 표시됨
			UE_LOG(LogTemp, Log, TEXT("AI 캐릭터 시야각 시스템 활성화: %s"), *Character->GetName());
		}
		else
		{
			// 플레이어 캐릭터는 위젯 사용하지 않음
			Character->ShowAimEnemyWidget(false);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("모든 캐릭터의 시야각 기반 AimEnemy 위젯 시스템 설정 완료"));
}


void AInfinityFightersGameModeBase::RegisterAllCharactersWithManager()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (UManagerController* Manager = Cast<UManagerController>(World->GetGameInstance()))
	{
		auto EnsureDefaultIdentity = [](ACharacterBase* Character)
		{
			if (!Character)
			{
				return;
			}
			if (Character->MatchNickname == NAME_None)
			{
				if (Cast<AMyDoctorStrange>(Character))      { Character->MatchNickname = FName(TEXT("ykd0638")); }
				else if (Cast<AMySpiderMan>(Character))     { Character->MatchNickname = FName(TEXT("nayeong")); }
				else if (Cast<AMyIronMan>(Character))       { Character->MatchNickname = FName(TEXT("jjg")); }
			}
			if (Character->MatchTeam == ETeam::None)
			{
				if (Cast<AMyDoctorStrange>(Character))      { Character->MatchTeam = ETeam::Red; }
				else if (Cast<AMySpiderMan>(Character))     { Character->MatchTeam = ETeam::Blue; }
				else if (Cast<AMyIronMan>(Character))       { Character->MatchTeam = ETeam::Blue; }
			}
		};

		for (TActorIterator<ACharacterBase> It(World); It; ++It)
		{
			ACharacterBase* Character = *It;
			if (!Character)
			{
				continue;
			}

			EnsureDefaultIdentity(Character);

			const FName Nickname = (Character->MatchNickname != NAME_None)
				? Character->MatchNickname
				: FName(*Character->GetName());

			if (Nickname != NAME_None)
			{
				Manager->RegisterPlayer(Nickname, Character->MatchTeam);
			}
		}
	}
}

void AInfinityFightersGameModeBase::StartEnemySpawnLocations()
{
	SpawnLocations.Empty();

	UE_LOG(LogTemp, Log, TEXT("StartEnemySpawnLocations 시작"));

	// BP_SpawnZoneAssemble 액터들 찾기
	TArray<AActor*> FoundSpawnZones;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundSpawnZones);

	for (AActor* Actor : FoundSpawnZones)
	{
		if (Actor && Actor->GetName().Contains(TEXT("BP_SpawnZoneAssemble")))
		{
			FVector SpawnLocation = Actor->GetActorLocation();
			SpawnLocations.Add(SpawnLocation);
			UE_LOG(LogTemp, Log, TEXT("스폰 위치 추가: %s - %s"),
				*Actor->GetName(), *SpawnLocation.ToString());
		}
	}

	// ASpawnPointBase 타입도 추가
	TArray<AActor*> FoundSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnPointBase::StaticClass(), FoundSpawnPoints);

	for (AActor* Actor : FoundSpawnPoints)
	{
		if (Actor)
		{
			FVector SpawnLocation = Actor->GetActorLocation();
			SpawnLocations.Add(SpawnLocation);
			UE_LOG(LogTemp, Log, TEXT("ASpawnPointBase 스폰 위치 추가: %s - %s"),
				*Actor->GetName(), *SpawnLocation.ToString());
		}
	}

	// 아무것도 없으면 기본 위치 사용
	if (SpawnLocations.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("스폰 포인트를 찾을 수 없어 기본 위치 사용"));
		SpawnLocations.Add(FVector(-1030.0f, 1490.0f, 57.183952f));
		SpawnLocations.Add(FVector(-365.0f, -285.0f, 0.0f));
	}

	UE_LOG(LogTemp, Warning, TEXT("총 %d개의 스폰 위치 설정 완료"), SpawnLocations.Num());
}

void AInfinityFightersGameModeBase::RequestPlayerRespawn(AController* PlayerController)
{
	if (!PlayerController) return;

	FVector SpawnLocation = GetSpawnLocation();
	UE_LOG(LogTemp, Log, TEXT("리스폰: %s"), *SpawnLocation.ToString());

	if (SpawnLocation == FVector::ZeroVector)
	{
		FVector Location1(-1030.0f, 1490.0f, 57.183952f);
		FVector Location2(-320.f,-290.f,88.f);
		SpawnLocation = FMath::RandBool() ? Location1 : Location2;
		UE_LOG(LogTemp, Warning, TEXT("기본 위치로 리스폰: %s"), *SpawnLocation.ToString());
	}

	APawn* Pawn = PlayerController->GetPawn();

	if (IsValid(Pawn) && !Pawn->IsPendingKillPending())
	{
		Pawn->SetActorLocation(SpawnLocation);
		Pawn->SetActorRotation(FRotator::ZeroRotator);

		if (APlayerController* PC = Cast<APlayerController>(PlayerController))
		{
			PC->SetControlRotation(FRotator::ZeroRotator);
		}

		if (ACharacterBase* Character = Cast<ACharacterBase>(Pawn))
		{
			Character->InitStatsFromDA();
			Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Character->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

			if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
			{
				MovementComp->SetMovementMode(MOVE_Walking);
			}

			// 리스폰 보호 및 버블 효과 적용
			ApplyRespawnProtection(Character);
		}
	}
	

}

void AInfinityFightersGameModeBase::ApplyRespawnProtection(ACharacterBase* Character)
{
	if (!Character) 
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyRespawnProtection: Character가 null입니다."));
        return;
    }

    // 무적 상태 설정 
    // 완전히 충돌을 끄는 대신 NoCollision으로 설정하여 더 안전하게 처리
    if (Character->GetCapsuleComponent())
    {
        //Character->GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
        //Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block); // 바닥은 유지
    }

    // 메시 충돌도 무시하도록 설정
    if (Character->GetMesh())
    {
        Character->GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
    }
    
    // 리스폰 보호 스피어만 활성화
    if (Character->RespawnProtectionSphere)
    {
        FVector CharacterLocation = Character->GetActorLocation();
        FVector BubbleLocation = CharacterLocation + FVector(0, 0, 50); // 캐릭터보다 살짝 위에

        // Sphere를 캐릭터 위치로 이동하고 활성화
        Character->RespawnProtectionSphere->SetWorldLocation(BubbleLocation);
        Character->RespawnProtectionSphere->SetVisibility(true);
    	Character->RespawnProtectionSphere->Activate(true);
        Character->RespawnProtectionSphere->SetHiddenInGame(false);
        
        
        /*Character->RespawnBubbleEffect->SetWorldLocation(BubbleLocation);
        Character->RespawnBubbleEffect->SetVisibility(true);
        Character->RespawnBubbleEffect->SetHiddenInGame(false);
        Character->RespawnBubbleEffect->Activate(true); // 나이아가라 시스템 활성화*/
        
        UE_LOG(LogTemp, Log, TEXT("리스폰 보호 효과 활성화: %s 위치: %s"), 
               *Character->GetName(), *BubbleLocation.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("RespawnProtectionSphere 또는 RespawnBubbleEffect가 없습니다: %s"), 
               *Character->GetName());
    }

    // 캐릭터 상태 플래그 설정 (캐릭터 클래스에 이런 변수가 있다면)
    // Character->bIsInRespawnProtection = true;
    
    FTimerHandle RespawnProtectionTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        RespawnProtectionTimerHandle,
        [this, Character]()
        {
            RemoveRespawnProtection(Character);
        },
        3.0f, // 3초간 보호
        false
    );
    
    UE_LOG(LogTemp, Log, TEXT("리스폰 보호 적용 완료: %s (3초간)"), *Character->GetName());
}

void AInfinityFightersGameModeBase::RemoveRespawnProtection(class ACharacterBase* Character)
{
	if (!Character || !IsValid(Character))
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveRespawnProtection: Character가 유효하지 않습니다."));
		return;
	}

	// 충돌 복구
	if (Character->GetCapsuleComponent())
	{
		Character->GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
		// 필요에 따라 특정 채널은 다시 설정
		Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}

	if (Character->GetMesh())
	{
		Character->GetMesh()->SetCollisionResponseToAllChannels(ECR_Block);
		Character->GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}
	UE_LOG(LogTemp, Log, TEXT("들어옴?1"));
	// 보호 효과 비활성화
	if (Character->RespawnProtectionSphere)
	{
	UE_LOG(LogTemp, Log, TEXT("들어옴?2"));
		Character->RespawnProtectionSphere->SetVisibility(false);
		Character->RespawnProtectionSphere->SetHiddenInGame(true);
		Character->RespawnProtectionSphere->DestroyComponent();
	}

	/*if (Character->RespawnBubbleEffect)
	{
		Character->RespawnBubbleEffect->Deactivate(); // 나이아가라 시스템 비활성화
		Character->RespawnBubbleEffect->SetVisibility(false);
		Character->RespawnBubbleEffect->SetHiddenInGame(true);
	}*/

	// 캐릭터 상태 플래그 해제
	// Character->bIsInRespawnProtection = false;

	UE_LOG(LogTemp, Log, TEXT("리스폰 보호 해제 완료: %s"), *Character->GetName());
}

void AInfinityFightersGameModeBase::CollectAllSpawnPoints()
{
	AvailableSpawnPoints.Empty();
	UsedSpawnPoints.Empty();

	// BP_SpawnPoint 액터들을 모두 찾기
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (Actor && (Actor->GetName().Contains(TEXT("BP_SpawnZoneAssemble")) ||
					  Actor->GetName().Contains(TEXT("Spawn")) ||
					  Actor->GetName().Contains(TEXT("SpawnZone"))))
		{
			AvailableSpawnPoints.Add(Actor);
			//UE_LOG(LogTemp, Warning, TEXT("스폰 포인트 발견: %s"), *Actor->GetName());
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("총 %d개의 스폰 포인트를 찾았습니다."), AvailableSpawnPoints.Num());
}

FVector AInfinityFightersGameModeBase::GetRandomAvailableSpawnLocation()
{
	if (AvailableSpawnPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("사용 가능한 스폰 포인트가 없습니다!"));
		return FVector::ZeroVector;
	}

	// 랜덤 인덱스 선택
	int32 RandomIndex = FMath::RandRange(0, AvailableSpawnPoints.Num() - 1);
	AActor* SelectedSpawnPoint = AvailableSpawnPoints[RandomIndex];

	// 선택된 스폰 포인트를 사용됨 목록으로 이동
	AvailableSpawnPoints.RemoveAt(RandomIndex);
	UsedSpawnPoints.Add(SelectedSpawnPoint);

	FVector SpawnLocation = SelectedSpawnPoint->GetActorLocation();
	UE_LOG(LogTemp, Warning, TEXT("스폰 위치 선택: %s"), *SpawnLocation.ToString());

	return SpawnLocation;
}

FVector AInfinityFightersGameModeBase::GetRandomSpawnLocationWithMinDistance(float MinDistance)
{
	if (AvailableSpawnPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("사용 가능한 스폰 포인트가 없습니다!"));
		return FVector::ZeroVector;
	}

	TArray<AActor*> ValidSpawnPoints;

	// 사용된 스폰 포인트들과 최소 거리 이상 떨어진 포인트만 필터링
	for (AActor* SpawnPoint : AvailableSpawnPoints)
	{
		bool bValidDistance = true;

		for (AActor* UsedPoint : UsedSpawnPoints)
		{
			float Distance = FVector::Dist(SpawnPoint->GetActorLocation(),
										 UsedPoint->GetActorLocation());
			if (Distance < MinDistance)
			{
				bValidDistance = false;
				break;
			}
		}

		if (bValidDistance)
		{
			ValidSpawnPoints.Add(SpawnPoint);
		}
	}

	// 유효한 포인트가 있으면 그 중에서 랜덤 선택
	if (ValidSpawnPoints.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, ValidSpawnPoints.Num() - 1);
		AActor* SelectedPoint = ValidSpawnPoints[RandomIndex];

		AvailableSpawnPoints.Remove(SelectedPoint);
		UsedSpawnPoints.Add(SelectedPoint);

		FVector SpawnLocation = SelectedPoint->GetActorLocation();
		UE_LOG(LogTemp, Warning, TEXT("최소 거리 고려한 스폰 위치 선택: %s"), *SpawnLocation.ToString());

		return SpawnLocation;
	}

	// 최소 거리 조건을 만족하는 포인트가 없으면 아무 포인트나 선택
	UE_LOG(LogTemp, Warning, TEXT("최소 거리 조건을 만족하는 포인트가 없어 일반 랜덤 선택합니다."));
	return GetRandomAvailableSpawnLocation();
}

void AInfinityFightersGameModeBase::ResetSpawnPoints()
{
	// 사용된 스폰 포인트들을 다시 사용 가능하게 만들기
	AvailableSpawnPoints.Append(UsedSpawnPoints);
	UsedSpawnPoints.Empty();

	UE_LOG(LogTemp, Warning, TEXT("스폰 포인트가 리셋되었습니다. 사용 가능한 포인트: %d개"), AvailableSpawnPoints.Num());
}

void AInfinityFightersGameModeBase::SpawnCharactersAtRandomLocations()
{
	// 모든 CharacterBase 찾기
	TArray<AActor*> AllCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacterBase::StaticClass(), AllCharacters);

	UE_LOG(LogTemp, Warning, TEXT("총 %d개의 캐릭터를 랜덤 위치에 배치합니다."), AllCharacters.Num());

	for (AActor* Character : AllCharacters)
	{
		FVector SpawnLocation = GetRandomSpawnLocationWithMinDistance(300.0f); // 최소 300 유닛 거리
		if (SpawnLocation != FVector::ZeroVector)
		{
			Character->SetActorLocation(SpawnLocation);
			UE_LOG(LogTemp, Warning, TEXT("캐릭터 %s를 위치 %s에 배치했습니다."),
				*Character->GetName(), *SpawnLocation.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("캐릭터 %s의 배치 위치를 찾을 수 없습니다."), *Character->GetName());
		}
	}
}

void AInfinityFightersGameModeBase::RequestAIRespawn(AController* PlayerController)
{
	if (!PlayerController) return;

	FVector SpawnLocation = GetSpawnLocation();
	UE_LOG(LogTemp, Log, TEXT("AI 리스폰: %s"), *SpawnLocation.ToString());

	if (SpawnLocation == FVector::ZeroVector)
	{
		const FVector Location1(-1030.0f, 1490.0f, 57.183952f);
		const FVector Location2(-320.f, -290.f, 88.f);
		SpawnLocation = FMath::RandBool() ? Location1 : Location2;
		UE_LOG(LogTemp, Warning, TEXT("기본 위치로 AI 리스폰: %s"), *SpawnLocation.ToString());
	}

	APawn* Pawn = PlayerController->GetPawn();
	if (IsValid(Pawn) && !Pawn->IsPendingKillPending())
	{
		Pawn->SetActorLocation(SpawnLocation);
		Pawn->SetActorRotation(FRotator::ZeroRotator);

		if (APlayerController* PC = Cast<APlayerController>(PlayerController))
		{
			PC->SetControlRotation(FRotator::ZeroRotator);
		}

		if (ACharacterBase* Character = Cast<ACharacterBase>(Pawn))
		{
			Character->InitStatsFromDA();
			Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Character->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

			if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
			{
				Move->SetMovementMode(MOVE_Walking);
			}

			// 리스폰 보호 적용 (버블 효과 포함)
			ApplyRespawnProtection(Character);

			UE_LOG(LogTemp, Log, TEXT("AI 리스폰 완료 (보호 적용): %s"), *Character->GetName());
		}
	}
	
	/*if (!PlayerController) return;

	FVector SpawnLocation = GetSpawnLocation();
	UE_LOG(LogTemp, Log, TEXT("리스폰: %s"), *SpawnLocation.ToString());

	if (SpawnLocation == FVector::ZeroVector)
	{
		const FVector Location1(-1030.0f, 1490.0f, 57.183952f);
		const FVector Location2(-320.f, -290.f, 88.f);
		SpawnLocation = FMath::RandBool() ? Location1 : Location2;
		UE_LOG(LogTemp, Warning, TEXT("기본 위치로 리스폰: %s"), *SpawnLocation.ToString());
	}

	APawn* Pawn = PlayerController->GetPawn();
	if (IsValid(Pawn) && !Pawn->IsPendingKillPending())
	{
		Pawn->SetActorLocation(SpawnLocation);
		Pawn->SetActorRotation(FRotator::ZeroRotator);

		if (APlayerController* PC = Cast<APlayerController>(PlayerController))
		{
			PC->SetControlRotation(FRotator::ZeroRotator);
		}

		if (ACharacterBase* Character = Cast<ACharacterBase>(Pawn))
		{
			Character->InitStatsFromDA();
			Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Character->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

			if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
			{
				Move->SetMovementMode(MOVE_Walking);
			}

			// // NS_Pendulum 버블 효과 스폰
			// if (NS_PendulumSystem)
			// {
			// 	FVector EffectLocation = SpawnLocation + FVector(0, 0, 50); // 캐릭터보다 살짝 위에
			// 	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			// 		GetWorld(),
			// 		NS_PendulumSystem,
			// 		EffectLocation,
			// 		FRotator::ZeroRotator,
			// 		FVector(1.0f), // 스케일
			// 		true, // 자동 제거
			// 		true, // 자동 활성화
			// 		ENCPoolMethod::None
			// 	);
			// 	UE_LOG(LogTemp, Warning, TEXT("AI 리스폰에 NS_Pendulum 버블 효과 추가: %s"), *EffectLocation.ToString());
			// }
			// else
			// {
			// 	UE_LOG(LogTemp, Warning, TEXT("NS_PendulumSystem이 설정되지 않았습니다!"));
			// }
		}
	}*/
}

void AInfinityFightersGameModeBase::ClassifySpawnPointsByType()
{
	AISpawnPoints.Empty();

	UE_LOG(LogTemp, Log, TEXT("스폰 포인트 분류 시작..."));

	// 월드에서 모든 ASpawnPointBase 타입 액터 찾기
	TArray<AActor*> FoundSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnPointBase::StaticClass(), FoundSpawnPoints);

	UE_LOG(LogTemp, Log, TEXT("월드에서 %d개의 ASpawnPointBase 발견"), FoundSpawnPoints.Num());

	// ASpawnPointBase 타입의 액터들을 AISpawnPoints에 추가
	for (AActor* Actor : FoundSpawnPoints)
	{
		if (ASpawnPointBase* SpawnPointBase = Cast<ASpawnPointBase>(Actor))
		{
			AISpawnPoints.Add(SpawnPointBase);
			UE_LOG(LogTemp, Log, TEXT("AI 스폰 포인트 추가: %s"), *SpawnPointBase->GetName());
		}
	}

	// BP_SpawnZoneAssemble 타입도 추가로 확인
	for (AActor* SpawnPoint : AvailableSpawnPoints)
	{
		if (!SpawnPoint) continue;

		FString SpawnPointName = SpawnPoint->GetName();

		// BP_SpawnZoneAssemble 또는 관련 이름 확인
		if (SpawnPointName.Contains(TEXT("BP_SpawnZoneAssemble")) ||
			SpawnPointName.Contains(TEXT("SpawnZone")) ||
			SpawnPointName.Contains(TEXT("Spawn")))
		{
			// ASpawnPointBase로 캐스팅 시도
			if (ASpawnPointBase* SpawnPointBase = Cast<ASpawnPointBase>(SpawnPoint))
			{
				if (!AISpawnPoints.Contains(SpawnPointBase))
				{
					AISpawnPoints.Add(SpawnPointBase);
					UE_LOG(LogTemp, Log, TEXT("스폰 존 추가: %s"), *SpawnPointName);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("스폰 포인트 '%s'가 ASpawnPointBase 타입이 아닙니다"), *SpawnPointName);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("최종 AI 스폰 포인트: %d개"), AISpawnPoints.Num());

	// 디버깅: 각 스폰 포인트 위치 출력
	for (int32 i = 0; i < AISpawnPoints.Num(); i++)
	{
		if (AISpawnPoints[i])
		{
			FVector Location = AISpawnPoints[i]->GetActorLocation();
			UE_LOG(LogTemp, Log, TEXT("스폰 포인트 %d: %s - 위치: %s"),
				i, *AISpawnPoints[i]->GetName(), *Location.ToString());
		}
	}
}


FVector AInfinityFightersGameModeBase::GetSpawnLocation()
{
	UE_LOG(LogTemp, Log, TEXT("GetSpawnLocation 호출됨. AISpawnPoints 개수: %d"), AISpawnPoints.Num());

	// AISpawnPoints 배열에서 유효한 스폰 포인트들만 필터링
	TArray<ASpawnPointBase*> ValidArray;
	ValidArray.Reserve(AISpawnPoints.Num());

	for (int32 i = 0; i < AISpawnPoints.Num(); i++)
	{
		ASpawnPointBase* P = AISpawnPoints[i];
		if (IsValid(P))
		{
			ValidArray.Add(P);
			UE_LOG(LogTemp, Log, TEXT("유효한 스폰 포인트 %d: %s - 위치: %s"),
				i, *P->GetName(), *P->GetActorLocation().ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("유효하지 않은 스폰 포인트 %d"), i);
		}
	}

	// 유효한 스폰 포인트가 있으면 랜덤 선택
	if (ValidArray.Num() > 0)
	{
		const int32 Idx = FMath::RandRange(0, ValidArray.Num() - 1);
		FVector SelectedLocation = ValidArray[Idx]->GetActorLocation();
		UE_LOG(LogTemp, Warning, TEXT("스폰 위치 선택됨: %s (인덱스: %d/%d)"),
			*SelectedLocation.ToString(), Idx, ValidArray.Num() - 1);
		return SelectedLocation;
	}

	// AISpawnPoints가 비어있다면 다시 수집 시도
	UE_LOG(LogTemp, Warning, TEXT("AISpawnPoints가 비어있어 재수집 시도"));

	// BP_SpawnZoneAssemble 타입 액터들 직접 검색
	TArray<AActor*> FoundSpawnZones;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundSpawnZones);

	TArray<AActor*> ValidSpawnZones;
	for (AActor* Actor : FoundSpawnZones)
	{
		if (Actor && Actor->GetName().Contains(TEXT("BP_SpawnZoneAssemble")))
		{
			ValidSpawnZones.Add(Actor);
			UE_LOG(LogTemp, Log, TEXT("BP_SpawnZoneAssemble 발견: %s - 위치: %s"),
				*Actor->GetName(), *Actor->GetActorLocation().ToString());
		}
	}

	if (ValidSpawnZones.Num() > 0)
	{
		const int32 Idx = FMath::RandRange(0, ValidSpawnZones.Num() - 1);
		FVector SelectedLocation = ValidSpawnZones[Idx]->GetActorLocation();
		UE_LOG(LogTemp, Warning, TEXT("BP_SpawnZoneAssemble에서 스폰 위치 선택됨: %s"), *SelectedLocation.ToString());
		return SelectedLocation;
	}

	// 그래도 못 찾으면 ASpawnPointBase 타입 직접 검색
	TArray<AActor*> FoundSpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnPointBase::StaticClass(), FoundSpawnPoints);

	UE_LOG(LogTemp, Log, TEXT("ASpawnPointBase 타입 %d개 발견"), FoundSpawnPoints.Num());

	if (FoundSpawnPoints.Num() > 0)
	{
		const int32 Idx = FMath::RandRange(0, FoundSpawnPoints.Num() - 1);
		FVector SelectedLocation = FoundSpawnPoints[Idx]->GetActorLocation();
		UE_LOG(LogTemp, Warning, TEXT("ASpawnPointBase에서 스폰 위치 선택됨: %s"), *SelectedLocation.ToString());
		return SelectedLocation;
	}

	// 아무것도 못 찾으면 경고 출력 후 ZeroVector 반환
	UE_LOG(LogTemp, Error, TEXT("스폰 포인트를 전혀 찾을 수 없습니다! ZeroVector 반환"));
	return FVector::ZeroVector;
}

void AInfinityFightersGameModeBase::OnMatchEnded()
{
	if (bEnded) return;
	if (!HasAuthority()) return;

	if (UManagerController* GI = GetGameInstance<UManagerController>())
	{
		const FMatchResult Result = GI->GetMatchResult();
		const int32 Count   = Result.PlayerStats.Num();
		const TCHAR* TopName = (Count > 0) ? *Result.PlayerStats[0].PlayerId : TEXT("None");
		const int32 TopKills = (Count > 0) ? Result.PlayerStats[0].Kills : -1;

		UE_LOG(LogTemp, Warning, TEXT("MatchResult: Valid=%s, Players=%d, Top=%s, Kills=%d"),
			Result.bValid ? TEXT("true") : TEXT("false"),
			Count, TopName, TopKills);

		//SpawnedBPAim->SetActorHiddenInGame(true);
		// 캐시에 굽기
		GI->SetMatchResult(Result);
	}

	bEnded = true; 
	UGameplayStatics::OpenLevel(this, TEXT("Ending_Map"));
}

void AInfinityFightersGameModeBase::HandleMatchTimeout()
{

	OnMatchEnded();
}

void AInfinityFightersGameModeBase::StartUITimer()
{
	// 플레이어 컨트롤러에서 HUD 위젯에 접근하여 타이머 시작
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (APlayerController* PlayerController = Cast<APlayerController>(PC))
			{
				// HUD에서 GameUI 위젯 찾기
				if (AHUD* HUD = PlayerController->GetHUD())
				{
					// HUD에 GameUI 위젯이 있다면 타이머 시작
					UE_LOG(LogTemp, Warning, TEXT("UI 타이머 시작 시도 - MatchDuration: %f초"), MatchDuration);

					// 블루프린트에서 구현된 타이머 시작 이벤트 호출
					// 또는 직접 위젯에 접근하여 타이머 시작
				}
			}
		}
	}
}
