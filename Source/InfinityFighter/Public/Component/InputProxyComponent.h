

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"
#include "InputProxyComponent.generated.h"

class UInputAction;
class UEnhancedInputComponent;
class UInputMappingContext;
class UCBrainComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntent, FName, IntentTag);

// OOP: 입력 수집/바인딩 책임을 별도 컴포넌트로 분리(SRP)
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INFINITYFIGHTER_API UInputProxyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInputProxyComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* IC_Player;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_MoveForward;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_MoveRight;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_Look;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_Reload;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_Dodge;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_Fire;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_Jump;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_Skill1;
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_Skill2;

	UPROPERTY(BlueprintAssignable, Category="Input|Intent")
	FOnIntent OnIntent;

	FName FireIntent ="Input.Fire";
	FName DodgeIntent = "Input.Dodge";
	FName ReloadIntent = "Input.Reload";
	FName JumpIntent = "Input.Jump";
	FName Skill1Intent = "Input.Skill1";
	FName Skill2Intent = "Input.Skill2";
	FName DoublePressed = "Input.Double";
	// Space double-press (Jump double) intent for BaseFly
	FName FlyIntent = "Input.Fly";

	
	void SetupPlayerBindings(class UEnhancedInputComponent* EIC);
	void RegisterIMCToLocalPlayer(class APlayerController* PC);

	UPROPERTY()
	UCBrainComponent* Brain = nullptr;

	// 누적 입력값
	float CachedForward = 0.f;
	float CachedRight   = 0.f;
	float CachedLookX = 0.0f;  // Yaw (좌우 회전)
	float CachedLookY = 0.0f;  // Pitch (상하 회전)

	float CurrenTime_Press = 0.f;
	float DoubleThreshold =0.2f;
	FTimerHandle DoubleHandle;
	// Separate timer for jump double-press detection
	FTimerHandle JumpDoubleHandle;
	UPROPERTY()
	class ACharacterBase* OwnerBase;
	bool isFoward = false;
	bool isRight =false;

	UFUNCTION()
	void OnMoveForward(const struct FInputActionValue& Value);
	UFUNCTION()
	void OnMoveRight(const struct FInputActionValue& Value);
	UFUNCTION()
	void CheckDoubleFoward(const struct FInputActionValue& Value);
	void IncreseDoublePressed();
	UFUNCTION()
	void CheckDoubleJump(const struct FInputActionValue& Value);
	UFUNCTION()
	void OnLook(const struct FInputActionValue& Value);

	UFUNCTION()
	void OnFire(const struct FInputActionValue& Value);
	UFUNCTION()
	void OnSkill1(const struct FInputActionValue& Value);
	UFUNCTION()
	void OnSkill2(const struct FInputActionValue& Value);
	UFUNCTION()
	void OnReload(const struct FInputActionValue& Value);
	UFUNCTION()
	void OnJump(const struct FInputActionValue& Value);
    // Removed hold-to-ascend; double-space toggles flight instead
	UFUNCTION()
	void OnDodge(const struct FInputActionValue& Value);
	UFUNCTION()
	void EmitIntent(FName IntenSignal);

	UPROPERTY(EditAnywhere, Category="Dodge")
	TObjectPtr<UStaticMeshComponent> ArcComponent = nullptr;

	UPROPERTY(EditAnywhere, Category="Dodge")
	float DodgeHideDuration = 3.0f;

	FTimerHandle VisibilityRestoreTimerHandle;

	void RestoreArcVisibility();
};
