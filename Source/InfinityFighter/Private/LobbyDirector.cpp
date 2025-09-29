// Fill out your copyright notice in the Description page of Project Settings.
#include "LobbyDirector.h"

#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "GameFramework/Actor.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


class UNavigationSystemV1;

namespace
{
	bool Arrived2D(const AActor* Who, const AActor* Where, float Radius)
	{
		return Who && Where &&
			FVector::Dist2D(Who->GetActorLocation(), Where->GetActorLocation()) <= Radius;
	}
}

// Sets default values
ALobbyDirector::ALobbyDirector()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ALobbyDirector::BeginPlay()
{
	Super::BeginPlay();
	
	PC = UGameplayStatics::GetPlayerController(this, 0);
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	
	if (Characters.Num()   < 3) Characters.SetNum(3);
	if (TargetPoints.Num() < 3) TargetPoints.SetNum(3);

	// 0번은 플레이어로 고정
	Characters[static_cast<uint8>(ELobbyCharacterType::Player)] = PlayerPawn;

	ApplyViewTarget();
	UE_LOG(LogTemp, Warning, TEXT("[LobbyDirector::BeginPlay] bUseTeamAnchors: %d"), bUseTeamAnchors);

	UE_LOG(LogTemp, Warning, TEXT("[LobbyDirector::BeginPlay] RedTeamAnchor Num: %d, BlueTeamAnchor Num: %d"), RedTeamAnchor.Num(), BlueTeamAnchor.Num());

	for (int32 i = 0; i < RedTeamAnchor.Num(); ++i)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyDirector::BeginPlay] RedTeamAnchor[%d] IsValid: %d"), i, IsValid(RedTeamAnchor[i]));
	}
	for (int32 i = 0; i < BlueTeamAnchor.Num(); ++i)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyDirector::BeginPlay] BlueTeamAnchor[%d] IsValid: %d"), i, IsValid(BlueTeamAnchor[i]));
	}

	// 착지 후 LandAnimDelaySec 초 뒤에 이동 시작
	StartWhenLanded(ELobbyCharacterType::Player);
	StartWhenLanded(ELobbyCharacterType::Enemy1);
	StartWhenLanded(ELobbyCharacterType::Enemy2);
}

void ALobbyDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (int32 i = 0; i < 3; ++i)
		GetWorldTimerManager().ClearTimer(ArrivalCheckHandles[i]);

	Super::EndPlay(EndPlayReason);
}


void ALobbyDirector::ApplyViewTarget()
{
	if (!LobbyCam) return;

	if (!PC.IsValid())
		PC = UGameplayStatics::GetPlayerController(this, 0);

	if (PC.IsValid())
	{
		PC->bAutoManageActiveCameraTarget = false;
		PC->SetViewTargetWithBlend(LobbyCam, BlendTime);
	}
	
}

void ALobbyDirector::StartCharacterMoveToSeat(ELobbyCharacterType CharacterType)
{
	
  const int32 Idx = static_cast<int32>(CharacterType);
    if (!Characters.IsValidIndex(Idx)) return;

    APawn*  TargetCharacter = Characters[Idx];
    if (!TargetCharacter) return;

    // 컨트롤러 보장
    AController* Ctrl = TargetCharacter->GetController();
    if (!Ctrl)
    {
        TargetCharacter->AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
        TargetCharacter->SpawnDefaultController();
        Ctrl = TargetCharacter->GetController();
    }
    if (!Ctrl)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get/spawn controller for %s"), *TargetCharacter->GetName());
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("[StartCharacterMoveToSeat] Controller for %s is valid: %d"), *TargetCharacter->GetName(), IsValid(Ctrl));

    // 이동 중 자연스러운 회전
    if (ACharacter* Char = Cast<ACharacter>(TargetCharacter))
    {
        if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
        {
            UE_LOG(LogTemp, Warning, TEXT("[StartCharacterMoveToSeat] CharacterMovementComponent for %s is valid: %d"), *TargetCharacter->GetName(), IsValid(Move));
            if (IsValid(Move))
            {
                UE_LOG(LogTemp, Warning, TEXT("[StartCharacterMoveToSeat] MaxWalkSpeed: %f, MaxAcceleration: %f"), Move->MaxWalkSpeed, Move->MaxAcceleration);
                Move->SetMovementMode(MOVE_Walking); // 확실히 걷기 모드로 설정
                UE_LOG(LogTemp, Warning, TEXT("[StartCharacterMoveToSeat] MovementMode after SetMovementMode: %d"), (int32)Move->MovementMode);
                Move->StopMovementImmediately(); // 기존 이동 정지
            }
            Move->bOrientRotationToMovement     = true;
            Move->bUseControllerDesiredRotation = true;
            Move->RotationRate = FRotator(0.f, 720.f, 0.f);
        }
        Char->bUseControllerRotationYaw = false;
    }

	FVector GoalLocation = FVector::ZeroVector;
	bool bUseLocation = false;
	AActor* TargetSeat = nullptr; 
	UE_LOG(LogTemp, Warning, TEXT("[StartCharacterMoveToSeat] Initial bUseLocation: %d"), bUseLocation);
	UE_LOG(LogTemp, Warning, TEXT("[StartCharacterMoveToSeat] Current bUseTeamAnchors: %d"), bUseTeamAnchors);

	// 팀 앵커 기반 좌표 사용
	if (bUseTeamAnchors && (RedTeamAnchor.Num() > 0 || BlueTeamAnchor.Num() > 0))
	{
		bUseLocation = ComputeTeamDestination(
			CurrentTeam,
			Idx,
			GoalLocation,
			bMirrorOppositeForEnemiesDefault);
	}else
		{
		
			TargetSeat = TargetPoints[Idx];
			GoalLocation = TargetSeat->GetActorLocation();
		}
	
	
    // 이동 
    if (bUseLocation)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Move] Using Computed Location for %s: %s"), *TargetCharacter->GetName(), *GoalLocation.ToString());
        UAIBlueprintHelperLibrary::SimpleMoveToLocation(Ctrl, GoalLocation);
    }
    else
    {
        // TargetPoints 
        if (!TargetPoints.IsValidIndex(Idx)) return;
        TargetSeat = TargetPoints[Idx];
        if (!TargetSeat) return;

        GoalLocation = TargetSeat->GetActorLocation(); 
        UAIBlueprintHelperLibrary::SimpleMoveToActor(Ctrl, TargetSeat);
    }
	// 경로검사
	if (UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(GetWorld()))
	{
		if (UNavigationPath* Path = Nav->FindPathToLocationSynchronously(GetWorld(), TargetCharacter->GetActorLocation(), GoalLocation))
		{
			UE_LOG(LogTemp, Warning, TEXT("[Move] PathFound:%d NumPts:%d IsPartial:%d"), Path->IsValid() && !Path->IsPartial(), Path->PathPoints.Num(), Path->IsPartial());
		}
		else UE_LOG(LogTemp, Warning, TEXT("[Move] Path NULL"));
	}

    //도착타이머
    GetWorldTimerManager().SetTimer(
        ArrivalCheckHandles[Idx],
        FTimerDelegate::CreateUObject(this, &ALobbyDirector::OnCharacterArrival, CharacterType, GoalLocation),
        0.05f, 
        true
    );
}

void ALobbyDirector::OnCharacterArrival(ELobbyCharacterType CharacterType, FVector GoalLocation)
{
    const int32 Idx = static_cast<int32>(CharacterType);
    if (!Characters.IsValidIndex(Idx))
    {
        GetWorldTimerManager().ClearTimer(ArrivalCheckHandles[Idx]);
        return;
    }

    APawn* TargetCharacter = Characters[Idx];
    if (!IsValid(TargetCharacter))
    {
        GetWorldTimerManager().ClearTimer(ArrivalCheckHandles[Idx]);
        return;
    }

    const float AcceptRadius = 80.f;
    const FVector Now = TargetCharacter->GetActorLocation();
    if (FVector::Dist2D(Now, GoalLocation) <= AcceptRadius)
    {
        GetWorldTimerManager().ClearTimer(ArrivalCheckHandles[Idx]);
        
        // 도착: LobbyCam 반대 방향으로 정렬
        float TargetYaw = TargetCharacter->GetActorRotation().Yaw;

        if (AActor* Cam = LobbyCamActor.Get())
        {
            if (IsValid(Cam))
            {
                const float CamYaw = Cam->GetActorRotation().Yaw;
                TargetYaw = FMath::UnwindDegrees(CamYaw + 180.f);
            }
        }

        if (ACharacter* Char = Cast<ACharacter>(TargetCharacter))
        {
            Char->SetActorRotation(FRotator(0.f, TargetYaw, 0.f));
        }
    }
}

void ALobbyDirector::OnStartSelected()
{
	if (!bTeamChosen) { UE_LOG(LogTemp,Warning,TEXT("[Start] Select team first")); return; }
	auto StepForward = [this](ELobbyCharacterType Who)
	{
		const int32 Idx = static_cast<int32>(Who);
		if (!Characters.IsValidIndex(Idx)) return;

		ACharacter* Char = Cast<ACharacter>(Characters[Idx]);
		if (!IsValid(Char)) return;

		// 이동 가능 상태 보장 (원래 속도 그대로)
		if (UCharacterMovementComponent* M = Char->GetCharacterMovement())
		{
			M->SetMovementMode(MOVE_Walking);
			M->StopMovementImmediately();
			M->bOrientRotationToMovement     = true;
			M->bUseControllerDesiredRotation = true;
			M->RotationRate = FRotator(0.f, 720.f, 0.f);
		}

		const FVector StartLoc = Char->GetActorLocation();
		const FVector Fwd      = Char->GetActorForwardVector();
		const FVector GoalLoc  = StartLoc + Fwd * StartStepForward;

		AController* Ctrl = Char->GetController();
		if (!Ctrl)
		{
			Char->AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
			Char->SpawnDefaultController();
			Ctrl = Char->GetController();
		}
		if (Ctrl)
		{
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(Ctrl, GoalLoc);
		}
	};

	StepForward(ELobbyCharacterType::Player);
	StepForward(ELobbyCharacterType::Enemy1);
	StepForward(ELobbyCharacterType::Enemy2);

	// 로딩 UI는 위젯에서 켜고, 5초 뒤 전환
	FTimerHandle TravelHandle;
	GetWorldTimerManager().SetTimer(
		TravelHandle,
		this,
		&ALobbyDirector::TravelToNextLevel,
		StartToTravelDelay,  // 5.f
		false
	);
}

void ALobbyDirector::TravelToNextLevel()
{
	for (int32 i = 0; i < 3; ++i)
		GetWorldTimerManager().ClearTimer(ArrivalCheckHandles[i]);

	UGameplayStatics::OpenLevel(GetWorld(), FName("INGAME_MAP"), true, TEXT("game=/Game/BluePrint/BP_InfinityGameModeBase.BP_InfinityGameModeBase_C"));//map 이름
}

void ALobbyDirector::StartWhenLanded(ELobbyCharacterType Who)
{
	const int32 Idx = static_cast<int32>(Who);
    if (!Characters.IsValidIndex(Idx) || !TargetPoints.IsValidIndex(Idx)) return;

    APawn*  Pawn  = Characters[Idx];
    AActor* Seat  = TargetPoints[Idx];
    if (!Pawn || !Seat) return;

    // 캐릭터가 아니면 바로 이동 
    if (ACharacter* Char = Cast<ACharacter>(Pawn))
    {
        auto* Move = Char->GetCharacterMovement();
        if (!Move)
        {
            StartCharacterMoveToSeat(Who);
            return;
        }

        // 랜딩 애니 대기 후 이동
        if (Move->IsMovingOnGround() || Move->MovementMode == MOVE_Walking)
        {
            FTimerHandle OneShot;
            GetWorldTimerManager().SetTimer(
                OneShot,
                [this, Who]() { StartCharacterMoveToSeat(Who); },
                LandAnimDelaySec, false);
            return;
        }

        //착지 순간 지연 후 이동
        GetWorldTimerManager().SetTimer(
            LandingCheckHandles[Idx],
            [this, Who]()
            {
                const int32 I = static_cast<int32>(Who);
                if (!Characters.IsValidIndex(I)) { return; }

                ACharacter* C = Cast<ACharacter>(Characters[I]);
                if (!IsValid(C)) { GetWorldTimerManager().ClearTimer(LandingCheckHandles[I]); return; }

                if (UCharacterMovementComponent* M = C->GetCharacterMovement())
                {
                    if (M->MovementMode == MOVE_Walking)
                    {
                        // 착지 확인 → 감시 타이머 해제
                        GetWorldTimerManager().ClearTimer(LandingCheckHandles[I]);

                        // 랜딩 애니 재생 시간만큼 대기 후 이동
                        FTimerHandle OneShot;
                        GetWorldTimerManager().SetTimer(
                            OneShot,
                            [this, Who]() { StartCharacterMoveToSeat(Who); },
                            LandAnimDelaySec, false);
                    }
                }
            },
            LandCheckIntervalSec, true);
    }
    else
    {
        // 캐릭터가 아니면 바로 이동
        StartCharacterMoveToSeat(Who);
    }
}

void ALobbyDirector::StartAutoWalkTo(APawn* Character, AActor* Seat)
{
	if (!Seat || !Character) return;
	// 도착 상태였다면 다시 걷게 전환
	if (auto* AsCharacter = Cast<ACharacter>(Character))
	{
		if (auto* Move = AsCharacter->GetCharacterMovement())
		{
			if (Move->MovementMode == MOVE_None)
				Move->SetMovementMode(MOVE_Walking);
		}
	}
		
	AController* CharacterController = Character->GetController();
	if (CharacterController)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToActor(CharacterController, Seat);
		
		FTimerHandle TempArrivalCheckHandle;
		GetWorldTimerManager().SetTimer(
			TempArrivalCheckHandle,
			[this, Character, Seat, &TempArrivalCheckHandle]()
			{
				if (!Character || !Seat) return;
				
				
				const float TempAcceptRadius = 80.f; 
				const bool TempbDisableMovementOnArrive = true;
				
				if (Arrived2D(Character, Seat, TempAcceptRadius))
				{
					GetWorldTimerManager().ClearTimer(TempArrivalCheckHandle);
					
					if (AController* C = Character->GetController())
						C->StopMovement();
					
					// 도착 즉시 LobbyCam방향으로 회전
					float TargetYaw = Character->GetActorRotation().Yaw; 
					if (AActor* Cam = LobbyCamActor.Get())
					{
						const float CamYaw = Cam->GetActorRotation().Yaw;
						TargetYaw = FMath::UnwindDegrees(CamYaw + 180.f);  
					}
					
					// Yaw만 맞춰 회전
					Character->SetActorRotation(FRotator(0.f, TargetYaw, 0.f));
					
					if (TempbDisableMovementOnArrive) //  잠금
					{
						if (ACharacter* Char = Cast<ACharacter>(Character))
						{
							if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
								Move->DisableMovement();
						}
					}
				}
			}, 0.05f, true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Character %s has no controller to move"), *Character->GetName());
	}
}

void ALobbyDirector::OnTeamSelected(ETeam Team, bool bMirrorOppositeForEnemies)
{
	CurrentTeam = Team;
	bMirrorOppositeForEnemiesDefault = bMirrorOppositeForEnemies;
	bUseTeamAnchors = true;
	bTeamChosen = true;         
	bTeamPlaced = false;  
	TeamArrivedCount = 0;     
	UE_LOG(LogTemp, Warning, TEXT("[OnTeamSelected] bUseTeamAnchors set to: %d"), bUseTeamAnchors);

	StopAllAndReRoute();

	auto TryRoute = [this](ELobbyCharacterType Who)
	{
		const int32 I = static_cast<int32>(Who);

		ACharacter* Ch = nullptr;
		if (Characters.IsValidIndex(I))
			Ch = Cast<ACharacter>(Characters[I]);

		UE_LOG(LogTemp, Warning, TEXT("[OnTeamSelected::TryRoute] CharIdx %d: Characters[I] IsValid: %d, Ch (Cast result) IsValid: %d"),
			I, IsValid(Characters.IsValidIndex(I) ? Characters[I] : nullptr), IsValid(Ch));

		if (!IsValid(Ch))
		{
			UE_LOG(LogTemp, Warning, TEXT("[OnTeamSelected] No valid Character for idx %d"), I);
			return;
		}

		
		UCharacterMovementComponent* M = Ch->GetCharacterMovement();

		if (M)
		{
			// MOVE
			M->SetMovementMode(MOVE_Walking);
			M->StopMovementImmediately();
			M->bOrientRotationToMovement     = true;
			M->bUseControllerDesiredRotation = true;
			M->RotationRate = FRotator(0.f, 720.f, 0.f);

			UE_LOG(LogTemp, Warning, TEXT("[OnTeamSelected] Reroute %s (Mode:%d Walking:%d)"),
				*GetNameSafe(Ch), (int32)M->MovementMode, (int32)(M->IsMovingOnGround() || M->MovementMode == MOVE_Walking));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[OnTeamSelected] %s has no CharacterMovement!"), *GetNameSafe(Ch));
		}

		// 지상이든 공중이든 바로 새 목적지로 재라우팅
		UE_LOG(LogTemp, Warning, TEXT("[OnTeamSelected] Attempting StartCharacterMoveToSeat for idx %d (%s)"), I, *GetNameSafe(Ch));
		StartCharacterMoveToSeat(Who);
	};
	
	UE_LOG(LogTemp, Warning, TEXT("[OnTeamSelected] Calling TryRoute for Player (Idx 0)"));
	TryRoute(ELobbyCharacterType::Player);
	TryRoute(ELobbyCharacterType::Enemy1);
	TryRoute(ELobbyCharacterType::Enemy2);
}

void ALobbyDirector::OnReadySelected()
{
	// 팀 이동 모드 OFF → 대기석 모드
	bUseTeamAnchors = false;
	UE_LOG(LogTemp, Warning, TEXT("[Ready] Back to waiting seats"));

	// 진행 중 이동/타이머 끊기
	StopAllAndReRoute();

	//즉시 '대기석(TargetPoints)'으로 재라우팅
	auto ForceRouteToSeat = [this](ELobbyCharacterType Who)
	{
		const int32 I = (int32)Who;
		ACharacter* Ch = Characters.IsValidIndex(I) ? Cast<ACharacter>(Characters[I]) : nullptr;
		if (!IsValid(Ch)) return;

	
		if (UCharacterMovementComponent* M = Ch->GetCharacterMovement())
		{
			M->SetMovementMode(MOVE_Walking);
			M->StopMovementImmediately();
			M->bOrientRotationToMovement     = true;
			M->bUseControllerDesiredRotation = true;
			M->RotationRate = FRotator(0.f, 720.f, 0.f);
		}
		StartCharacterMoveToSeat(Who); 
	};

	ForceRouteToSeat(ELobbyCharacterType::Player);
	ForceRouteToSeat(ELobbyCharacterType::Enemy1);
	ForceRouteToSeat(ELobbyCharacterType::Enemy2);
}

bool ALobbyDirector::ComputeTeamDestination(ETeam Team, int32 MemberIdx, FVector& OutNavLoc,
                                            bool bOppositeForEnemies) const
{
	
	if (MemberIdx < 0 || MemberIdx > 2) { UE_LOG(LogTemp, Warning, TEXT("[ComputeTeamDestination] Invalid MemberIdx: %d"), MemberIdx); return false; }

	const bool bIsEnemy = (MemberIdx == 1 || MemberIdx == 2);

	// 팀의 앵커 배열 
	const TArray<AActor*>& UseArray =
		(Team == ETeam::Red)
			? (bOppositeForEnemies && bIsEnemy ? BlueTeamAnchor : RedTeamAnchor)
			: (bOppositeForEnemies && bIsEnemy ? RedTeamAnchor : BlueTeamAnchor);

	if (UseArray.Num() == 0) { UE_LOG(LogTemp, Warning, TEXT("[ComputeTeamDestination] UseArray is empty for team %d"), (int32)Team); return false; }

	
	const AActor* Anchor =
		(UseArray.IsValidIndex(MemberIdx) && IsValid(UseArray[MemberIdx]))
			? UseArray[MemberIdx]
			: UseArray[0];

	if (!IsValid(Anchor)) { UE_LOG(LogTemp, Warning, TEXT("[ComputeTeamDestination] Anchor is invalid for MemberIdx %d"), MemberIdx); return false; }

	const FVector  BaseLoc = Anchor->GetActorLocation();
	const FRotator BaseRot = Anchor->GetActorRotation();

	// 로컬 오프셋
	const FVector Local = FormationLocalOffsets.IsValidIndex(MemberIdx)
							? FormationLocalOffsets[MemberIdx]
							: FVector::ZeroVector;

	// 앵커의 Forward/Right 기준
	const FVector Forward = BaseRot.Vector(); 
	const FVector Right   = FRotationMatrix(BaseRot).GetScaledAxis(EAxis::Y);
	FVector WorldOffset   = Forward * Local.X + Right * Local.Y;
	WorldOffset.Z = 0.f;

	const FVector Desired = BaseLoc + WorldOffset;
	UE_LOG(LogTemp, Warning, TEXT("[ComputeTeamDestination] Desired Location: %s"), *Desired.ToString());

	// NavMesh 
	if (const UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
	{
		FNavLocation Projected;
		if (NavSys->ProjectPointToNavigation(Desired, Projected, FVector(150.f,150.f,300.f)))
		{
			OutNavLoc = Projected.Location;
			UE_LOG(LogTemp, Warning, TEXT("[ComputeTeamDestination] Projected to NavMesh: %s"), *OutNavLoc.ToString());
			return true;
		}
	}

	//실패 시 앵커 위치로 
	OutNavLoc = BaseLoc;
	UE_LOG(LogTemp, Warning, TEXT("[ComputeTeamDestination] Failed to project to NavMesh, using BaseLoc: %s"), *OutNavLoc.ToString());
	return true;
}

void ALobbyDirector::StopAllAndReRoute()
{
	for (int32 i = 0; i < 3; ++i)
	{
		if (!Characters.IsValidIndex(i)) continue;
		APawn* P = Characters[i];
		if (!P) continue;

		// 현재 이동 중지
		if (AController* C = P->GetController())
		{
			C->StopMovement();
		}
		// CharacterMovementComponent가 있다면 걷기 모드로 재설정
		if (ACharacter* Char = Cast<ACharacter>(P))
		{
			if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
			{
				Move->SetMovementMode(MOVE_Walking);
				UE_LOG(LogTemp, Warning, TEXT("[StopAllAndReRoute] %s MovementMode set to Walking: %d"), *GetNameSafe(Char), (int32)Move->MovementMode);
			}
		}
	}
}
