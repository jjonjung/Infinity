// Fill out your copyright notice in the Description page of Project Settings.


#include "TestPlayerBrain.h"
#include "InputCoreTypes.h"
#include "Router/ActionRouter.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"

// Enhanced Input
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

// Sets default values for this component's properties
UTestPlayerBrain::UTestPlayerBrain()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTestPlayerBrain::BeginPlay()
{
	Super::BeginPlay();

	// Owner(CharacterBase에서 액션 라우터 컴포넌트 가져오기) 
	ActionRouter = GetOwner() ? GetOwner()->FindComponentByClass<UActionRouter>() : nullptr;

	if (!ActionRouter)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TestFireBrain] ActionRouter not found on owner."));
	}

	SetupEnhancedInput();
	BindEnhancedActions();
	
}

void UTestPlayerBrain::SetupEnhancedInput()
{
	if (!MappingContext) return;

	APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController());
	if (!PC) return;

	if (ULocalPlayer* LP = PC->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsys->AddMappingContext(MappingContext, MappingPriority);
			
		}
	}
}
	

// Called every frame
void UTestPlayerBrain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTestPlayerBrain::BindEnhancedActions()
{
	APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController());
	if (!PC || !PC->InputComponent) return;

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PC->InputComponent))
	{
		if (IA_Fire)
		{
			EIC->BindAction(IA_Fire, ETriggerEvent::Started, this, &UTestPlayerBrain::OnFireStarted);
			UE_LOG(LogTemp, Log, TEXT("[TestFireBrain] Fire action bound."));
		}
	}
}

void UTestPlayerBrain::OnFireStarted(const struct FInputActionValue& Value)
{
	EmitIntent(FireIntent);
}

void UTestPlayerBrain::EmitIntent(FName IntentTag)
{
	if (!ActionRouter) return;
	 ActionRouter->ExcuteActionByIntent(IntentTag);

}

