// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/CharacterBase.h"
#include "ActionBase.h"
#include "ActionComponent.h"
#include "DA/ActionLoadOutDA.h"
#include "Router/ActionRouter.h"
#include "CharacterActionStatComponent.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Component/InputProxyComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Router/CMoveRouter.h"
#include "InfinityFightersGameModeBase.h"
#include "Component/CBrainComponent.h"

#include "EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"
#include "Component/CBrainComponent.h"
#include "Component/InputProxyComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "BattleManager.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SphereComponent.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ActionRouter = CreateDefaultSubobject<UActionRouter>(TEXT("ActionRouter"));
	ActionComponent = CreateDefaultSubobject<UActionComponent>(TEXT("ActionComp"));
	ActionStatComp = CreateDefaultSubobject<UCharacterActionStatComponent>(TEXT("ActionStatComp"));
	_SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	_SpringArm->SetupAttachment(GetCapsuleComponent());
	_SpringArm->SetRelativeLocation(FVector(0, 0, 0));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CamComp"));
    Camera->SetupAttachment(_SpringArm);
    FlyingObject = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlyingObject"));
    if (FlyingObject)
    {
        FlyingObject->SetupAttachment(GetMesh());
        FlyingObject->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
        FlyingObject->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        FlyingObject->SetVisibility(false, true);
        static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
        if (SphereMesh.Succeeded())
        {
            FlyingObject->SetStaticMesh(SphereMesh.Object);
            FlyingObject->SetWorldScale3D(FVector(0.5f));
        }
    }

	

	_SpringArm->bUsePawnControlRotation = true; // 컨트롤러 회전을 스프링암이 따름(특히 Pitch)
	_SpringArm->bInheritPitch = true;           // 안전빵 (보통 기본값 true지만 혹시 모르니)
	_SpringArm->bInheritYaw   = true;
	_SpringArm->bInheritRoll  = true;

	// 카메라는 스프링암의 회전을 그대로 사용
	Camera->bUsePawnControlRotation = false;

	InputProxy = CreateDefaultSubobject<UInputProxyComponent>(TEXT("InputProxy"));
	Brain      = CreateDefaultSubobject<UCBrainComponent>(TEXT("Brain"));
	MoveRouter = CreateDefaultSubobject<UCMoveRouter>(TEXT("MoveRouter"));

	// AimEnemy 위젯 컴포넌트 생성
	AimEnemyWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("AimEnemyWidget"));
	AimEnemyWidget->SetupAttachment(GetMesh());
	AimEnemyWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); // 캐릭터 머리 위
	AimEnemyWidget->SetWorldScale3D(FVector(0.3f)); // 요청된 스케일
	AimEnemyWidget->SetWidgetSpace(EWidgetSpace::Screen); // 항상 화면을 향하도록
	AimEnemyWidget->SetVisibility(false); // 기본적으로 숨김

	// UI - 여러 경로 시도
	static ConstructorHelpers::FClassFinder<UUserWidget> tempCrosshair(TEXT("'/Game/UI/Aim/WBP_Aim.WBP_Aim_C'"));
	if (tempCrosshair.Succeeded())
	{
		crosshairUIFactory = tempCrosshair.Class;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WBP_Aim_C 위젯 클래스 로드 실패: WBP_Aim_C"));
	}

	// BP_AimEnemy 위젯 클래스 설정
	static ConstructorHelpers::FClassFinder<UUserWidget> AimEnemyWidgetFinder(TEXT("/UI/Aim/BP_AimEnemy.BP_AimEnemy_C"));
	if (AimEnemyWidgetFinder.Succeeded())
	{
		AimEnemyWidgetClass = AimEnemyWidgetFinder.Class;
		UE_LOG(LogTemp, Log, TEXT("BP_AimEnemy 위젯 클래스 로드 성공"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BP_AimEnemy 위젯 클래스 로드 실패: BP_AimEnemy"));
	}

	bUseControllerRotationPitch = false;   // 메시/폰은 Pitch 영향 X
	bUseControllerRotationYaw   = true;
	bUseControllerRotationRoll  = false;

	if (auto* M = GetCharacterMovement())
	{
		M->bOrientRotationToMovement   = false;
		M->bUseControllerDesiredRotation = true;
	}

	// 카메라 세팅
	_SpringArm->bUsePawnControlRotation = true;  // 카메라만 컨트롤러 Pitch를 따라감
	Camera->bUsePawnControlRotation     = false;

	// 프레이밍(에임 중앙, 캐릭터 아래)
	_SpringArm->SetRelativeLocation(FVector(0, 0, 30)); // 피벗 조금 낮추고
	_SpringArm->TargetOffset      = FVector(0, 0, 50);  // 보는 지점을 위로
	_SpringArm->TargetArmLength   = 380.f;              // 거리
	Camera->SetFieldOfView(80.f);

	_SpringArm->bDoCollisionTest = true; // 유지
	_SpringArm->ProbeSize        = 6.f;  // 기본 12 → 줄이면 과민 반응 완화
	_SpringArm->CameraLagSpeed   = 12.f; // 부드럽게
	_SpringArm->bEnableCameraLag = true;

	//리스폰용버블
	RespawnProtectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RespawnProtectionSphere"));
	if (RespawnProtectionSphere)
	{
		RespawnProtectionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RespawnProtectionSphere->SetVisibility(false); // 기본적으로 숨김
	}

}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();


	InitActionRouterMapFromDa();
	InitActionsFromLoadout(); // 캐릭터가 소유하는 액션들을 케릭터베이스에 등록
	InitStatsFromDA(); //능력치 관련 정보들을 DA를 이용하여 초기화

	// AI 캐릭터인 경우 시야각 체크 타이머 시작
	if (Brain && Brain->Mode == EBrainMode::AI)
	{
		// AimEnemy 위젯 설정
		SetupAimEnemyWidget();
		
		// 초기에는 위젯 숨김
		ShowAimEnemyWidget(false);

		// 주기적으로 플레이어 시야각 체크
		GetWorldTimerManager().SetTimer(WidgetUpdateTimerHandle, this,
			&ACharacterBase::UpdateAimEnemyWidgetVisibility, UpdateFrequency, true);

		UE_LOG(LogTemp, Log, TEXT("AI 캐릭터의 시야각 체크 시작: %s"), *GetName());
	}

	if (ActionStatComp)
	{
		ActionStatComp->OnDeath.AddDynamic(this, &ACharacterBase::HandleDeath);
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->ViewPitchMin = -10.f; //-80
			PC->PlayerCameraManager->ViewPitchMax =  25.f; //80
		}
	}

	// UI 생성을 약간 지연시켜 PlayerController 준비 완료 대기
	FTimerHandle UITimerHandle;
	GetWorld()->GetTimerManager().SetTimer(UITimerHandle, [this]()
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			
			if (crosshairUIFactory)
			{
				FName CurrentLevel = FName(*UGameplayStatics::GetCurrentLevelName(GetWorld()));
				if (CurrentLevel == "INGAME_MAP")
				{
					crosshairUI = CreateWidget<UUserWidget>(PC, crosshairUIFactory); 
					if (crosshairUI != nullptr)
					{
						crosshairUI->AddToViewport(1000); // 높은 Z-Order로 설정
						crosshairUI->SetVisibility(ESlateVisibility::Visible);
						UE_LOG(LogTemp, Warning, TEXT("BP_Aim UI 화면 추가 성공!"));
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("CreateWidget 실패"));
					}
				}
			}
			else
			{
				// 런타임에서 블루프린트 클래스 로딩 시도
				UClass* FoundClass = LoadClass<UUserWidget>(nullptr, TEXT("/UI/Aim/BP_Aim.BP_Aim_C"));
				if (!FoundClass)
				{
					FoundClass = LoadClass<UUserWidget>(nullptr, TEXT("/UI/Aim/BP_Aim.BP_Aim_C"));
				}
				if (!FoundClass)
				{
					FoundClass = LoadClass<UUserWidget>(nullptr, TEXT("/UI/Aim/BP_Aim.BP_Aim_C"));
				}
				
				if (FoundClass)
				{
					UE_LOG(LogTemp, Warning, TEXT("런타임에서 BP_Aim 로딩 성공!"));
					crosshairUI = CreateWidget<UUserWidget>(PC, FoundClass);
					if (crosshairUI)
					{
						crosshairUI->AddToViewport(1000);
						crosshairUI->SetVisibility(ESlateVisibility::Visible);
						UE_LOG(LogTemp, Warning, TEXT("런타임 BP_Aim UI 생성 성공!"));
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("런타임에서도 BP_Aim을 찾을 수 없습니다."));
					UE_LOG(LogTemp, Error, TEXT("Content Browser에서 BP_Aim의 정확한 경로를 확인하세요."));
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerController를 찾을 수 없음"));
		}
	}, 0.1f, false);

}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (auto* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (InputProxy) InputProxy->SetupPlayerBindings(EIC);
	}

}

//두뇌 2개중 무엇을 선택할 것인가? 
void ACharacterBase::RefreshBrainActivation()
{
	
}

void ACharacterBase::InitActionsFromLoadout()
{
	if (!ActionComponent || !ActionLoadOutDA) return;
	
	for (const FActionSlot& Slot : ActionLoadOutDA->Actions)
	{
		UActionBase* Action = ActionComponent->CreateAndRegister(Slot.ActionClass, Slot.ActionTag);
		if (!Action)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to create action for tag %s"), *Slot.ActionTag.ToString());
			continue;
		}
		
		Action->SetActionMontage(Slot.Montage);
		
		if (Slot.CooldownSec > 0.f)
		{
			Action->SetCooldown(Slot.CooldownSec);
		}
	}
}

void ACharacterBase::InitStatsFromDA()//추후 움직임 관련 스텟 추가
{
	if (ActionStatDA)
	{
		ActionStatComp->InitFromDa(ActionStatDA);
	}
}

void ACharacterBase::InitActionRouterMapFromDa()
{
	if (ActonRouterDA)
	{
		ActionRouter->InitFromRouterDA(ActonRouterDA);
	}
	;
}

void ACharacterBase::PlayActionMontage(UAnimMontage* montage)
{
	if (!montage)
	{
		return;
	}

	if (bIsDead && montage != DeathMontage)
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(montage);
	}
}


void ACharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (InputProxy)
	{
		InputProxy->RegisterIMCToLocalPlayer(Cast<APlayerController>(NewController));
	}
}

void ACharacterBase::OnRep_Controller()
{
	Super::OnRep_Controller();
	if (InputProxy)
	{
		InputProxy->RegisterIMCToLocalPlayer(Cast<APlayerController>(GetController()));
	}
}

void ACharacterBase::ExitWallMode()
{
	GetCharacterMovement()->GravityScale = 1.0f;
	ActionStatComp->SetInWall(false);
	SetWallClimbMode(false);
    
	GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Green, "Wall climb exited");
}

bool ACharacterBase::GetIsRunning()
{
	return bIsRunningMode;
}

void ACharacterBase::SetIsRunning(bool setRunMode)
{
	bIsRunningMode = setRunMode;
}

// 벽 붙기 모드 설정 (메인 함수)
void ACharacterBase::SetWallClimbMode(bool bInWall)
{
	bIsInWallMode = bInWall;
	
	if (bInWall)
	{
		UE_LOG(LogTemp, Warning, TEXT("벽 붙기 모드 활성화 - 카메라 회전만 허용"));
		EnableCameraRotationOnly();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("벽 붙기 모드 비활성화 - 일반 회전 복귀"));
		DisableCameraRotationOnly();
	}
}

// 카메라 회전만 허용 (메시는 고정)
void ACharacterBase::EnableCameraRotationOnly()
{
	if (!bIsInWallMode) return;
	
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (MovementComp)
	{
		bOriginalOrientRotationToMovement = MovementComp->bOrientRotationToMovement;
		// 캐릭터가 이동 방향으로 회전하는 것을 비활성화
		//MovementComp->bOrientRotationToMovement = false;
		// 캐릭터 전체 회전도 비활성화
		MovementComp->bUseControllerDesiredRotation = false;
	}

	// Pawn의 회전을 비활성화 (메시가 따라 회전하지 않도록)
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	
	// 메시 회전을 현재 상태로 완전히 고정
	if (GetMesh())
	{
		// 메시의 현재 회전값 저장
		OriginalMeshRotation = GetMesh()->GetComponentRotation();
		// 메시가 부모(Pawn)의 회전을 따라가지 않도록 설정
		GetMesh()->SetAbsolute(false, true, false); // Location, Rotation, Scale
		// 또는 더 확실하게 하려면:
		GetMesh()->SetWorldRotation(OriginalMeshRotation);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("카메라 회전만 허용 모드 설정 완료"));

}

// 일반 회전 모드로 복귀
void ACharacterBase::DisableCameraRotationOnly()
{
	// Movement Component 설정 복구
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (MovementComp)
	{
		bOriginalOrientRotationToMovement = MovementComp->bOrientRotationToMovement;
		MovementComp->bOrientRotationToMovement = true;
		MovementComp->bUseControllerDesiredRotation = true;
	}

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = true;
	bUseControllerRotationRoll = true;
	
	// 메시 회전을 현재 상태로 완전히 고정
	if (GetMesh())
	{
		// 메시의 현재 회전값 저장
		OriginalMeshRotation = GetMesh()->GetComponentRotation();
		// 메시가 부모(Pawn)의 회전을 따라가지 않도록 설정
		GetMesh()->SetAbsolute(false, false, false); // Location, Rotation, Scale
		GetMesh()->SetWorldRotation(OriginalMeshRotation);
	}
	UE_LOG(LogTemp, Warning, TEXT("카메라 회전 모드 해제"));

}
//피격	
void ACharacterBase::ReceiveDamage(float Amount, AActor* Causer)
{
	if (bIsDead)
	{
		return;
	}

	if (ActionStatComp)
	{
		if (DamagedMontage)
		{
			PlayActionMontage(DamagedMontage);
		}
		ActionStatComp->ApplyDamage(Amount, Causer);
		if (Brain-> CehckPlayerMode())
		{
			OnDamaged.Broadcast(ActionStatComp->CalculateHpPercentage());
		}
		
	}
}
//리스폰
void ACharacterBase::HandleDeath(AActor* DeadActor, AActor* KillerActor)
{
	// 캐릭터의 움직임 스톱
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->StopMovementImmediately();
		MovementComp->SetMovementMode(MOVE_None);
	}

	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);

	bIsDead = true;
	bRespawnRequested = false;

	const float PostDeathRespawnDelay = 4.0f;
	float FallbackRespawnDelay = PostDeathRespawnDelay;
	if (DeathMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Stop(0.1f);
			const float PlayResult = AnimInstance->Montage_Play(DeathMontage);
			if (PlayResult > 0.f)
			{
				FOnMontageEnded MontageEndedDelegate;
				MontageEndedDelegate.BindUObject(this, &ACharacterBase::HandleDeathMontageEnded);
				AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, DeathMontage);
				FallbackRespawnDelay = FMath::Max(FallbackRespawnDelay, PlayResult + PostDeathRespawnDelay);
			}
		}
	}

	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ACharacterBase::RequestRespawn, FallbackRespawnDelay, false);

	// 전투 매니저에 킬 이벤트 알림
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<ABattleManager> It(World); It; ++It)
		{
			ABattleManager* BM = *It;
			if (BM)
			{
				// Resolve killer character from projectile/ability ownership chain
				ACharacterBase* KillerChar = Cast<ACharacterBase>(KillerActor);
				if (!KillerChar && KillerActor)
				{
					// Try instigator first
					if (APawn* InstPawn = KillerActor->GetInstigator())
					{
						KillerChar = Cast<ACharacterBase>(InstPawn);
					}
					// Walk owner chain
					if (!KillerChar)
					{
						AActor* OwnerPtr = KillerActor->GetOwner();
						while (OwnerPtr && !KillerChar)
						{
							KillerChar = Cast<ACharacterBase>(OwnerPtr);
							OwnerPtr = OwnerPtr ? OwnerPtr->GetOwner() : nullptr;
						}
					}
				}
				BM->OnCharacterKilled(KillerChar, this);
				break;
			}
		}
	}
}



void ACharacterBase::HandleDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != DeathMontage)
	{
		return;
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			if (FOnMontageEnded* EndedDelegate = AnimInstance->Montage_GetEndedDelegate(DeathMontage))
			{
				EndedDelegate->Unbind();
			}
		}
	}

	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
	// 몽타주 종료 4초 뒤 리스폰 요청
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ACharacterBase::RequestRespawn, 4.0f, false);
}

void ACharacterBase::RequestRespawn()
{
	if (bRespawnRequested)
	{
		GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
		return;
	}

	bRespawnRequested = true;

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			if (DeathMontage && AnimInstance->Montage_IsPlaying(DeathMontage))
			{
				AnimInstance->Montage_Stop(0.1f, DeathMontage);
			}
			if (FOnMontageEnded* EndedDelegate = AnimInstance->Montage_GetEndedDelegate(DeathMontage))
			{
				EndedDelegate->Unbind();
			}
		}
	}

	bIsDead = false;
	// 게임 모드를 가져와 리스폰 
	if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
	{
		if (AInfinityFightersGameModeBase* IFGameMode = Cast<AInfinityFightersGameModeBase>(GameMode))
		{
			IFGameMode->RequestPlayerRespawn(GetController());
		}
	}
	// 리스폰 타이머를 초기화
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
}

void ACharacterBase::SetupAimEnemyWidget()
{
	if (!AimEnemyWidget || !AimEnemyWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("AimEnemyWidget 또는 AimEnemyWidgetClass가 null입니다"));
		return;
	}

	// 위젯 클래스 설정
	AimEnemyWidget->SetWidgetClass(AimEnemyWidgetClass);

	// 위젯 설정
	AimEnemyWidget->SetWidgetSpace(EWidgetSpace::Screen); // 항상 화면을 향함
	AimEnemyWidget->SetWorldScale3D(FVector(0.3f)); // 요청된 스케일
	AimEnemyWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); // 캐릭터 머리 위

	// DrawSize 설정 (위젯 크기)
	AimEnemyWidget->SetDrawSize(FVector2D(200.0f, 100.0f));

	UE_LOG(LogTemp, Log, TEXT("AimEnemy 위젯 설정 완료: %s"), *GetName());
}

void ACharacterBase::ShowAimEnemyWidget(bool bShow)
{
	if (AimEnemyWidget)
	{
		AimEnemyWidget->SetVisibility(bShow);
		// 로그를 너무 자주 출력하지 않도록 조건부로 변경
		/*if (bShow)
		{
			UE_LOG(LogTemp, Log, TEXT("AimEnemy 위젯 표시: %s"), *GetName());
		}*/
	}
}

void ACharacterBase::UpdateAimEnemyWidgetVisibility()
{
	// AI 캐릭터가 아니면 처리하지 않음
	if (!Brain || Brain->Mode != EBrainMode::AI)
	{
		return;
	}

	// 가장 가까운 플레이어 캐릭터 찾기
	ACharacterBase* NearestPlayer = GetNearestPlayerCharacter();
	if (!NearestPlayer)
	{
		ShowAimEnemyWidget(false);
		return;
	}

	// 플레이어 시야각 범위에 있는지 확인
	bool bInFieldOfView = IsInPlayerFieldOfView(NearestPlayer);
	ShowAimEnemyWidget(bInFieldOfView);
}

bool ACharacterBase::IsInPlayerFieldOfView(ACharacterBase* PlayerCharacter) const
{
	if (!PlayerCharacter || !PlayerCharacter->Camera)
	{
		return false;
	}

	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	FVector EnemyLocation = GetActorLocation();
	FVector PlayerForward = PlayerCharacter->Camera->GetForwardVector();

	// 거리 체크
	float Distance = FVector::Dist(PlayerLocation, EnemyLocation);
	if (Distance > MaxDetectionRange)
	{
		return false;
	}

	// 플레이어에서 적으로의 방향 벡터
	FVector DirectionToEnemy = (EnemyLocation - PlayerLocation).GetSafeNormal();

	// 시야각 체크
	float DotProduct = FVector::DotProduct(PlayerForward, DirectionToEnemy);
	float AngleInRadians = FMath::Acos(DotProduct);
	float AngleInDegrees = FMath::RadiansToDegrees(AngleInRadians);

	// 시야각 범위 내에 있는지 확인
	bool bInAngle = AngleInDegrees <= (FieldOfViewAngle * 0.5f);

	if (!bInAngle)
	{
		return false;
	}

	// 장애물 체크 (Line Trace)
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(PlayerCharacter);
	CollisionParams.AddIgnoredActor(const_cast<ACharacterBase*>(this));

	// 플레이어의 눈 높이에서 적의 중심으로 레이캐스트
	FVector TraceStart = PlayerLocation + FVector(0, 0, 80); // 플레이어 눈 높이
	FVector TraceEnd = EnemyLocation + FVector(0, 0, 50);    // 적 중심

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		CollisionParams
	);

	// 히트가 없으면 시야에 보임, 히트가 있으면 장애물에 가려짐
	return !bHit;
}

ACharacterBase* ACharacterBase::GetNearestPlayerCharacter() const
{
	ACharacterBase* NearestPlayer = nullptr;
	float MinDistSq = MaxDetectionRange * MaxDetectionRange;

	// 플레이어 폰에서 직접 획득 (TActorIterator 전체 순회 대신)
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		ACharacterBase* Character = Cast<ACharacterBase>(PC->GetPawn());
		if (!Character || Character == this) continue;

		const float DistSq = FVector::DistSquared(GetActorLocation(), Character->GetActorLocation());
		if (DistSq < MinDistSq)
		{
			MinDistSq = DistSq;
			NearestPlayer = Character;
		}
	}

	return NearestPlayer;
}
