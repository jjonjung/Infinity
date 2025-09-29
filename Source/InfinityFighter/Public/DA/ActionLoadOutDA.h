
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimMontage.h"
#include "Sound/SoundBase.h"
#include "ActionLoadOutDA.generated.h"

class UActionBase;
class UAnimMontage;
class USoundBase;
class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FActionSlot
{
	GENERATED_BODY()

	// 액션 호출 키(태그)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Action")
	FName ActionTag;

	// 실행할 액션 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Action")
	TSubclassOf<UActionBase> ActionClass;

	// 기본 쿨다운(초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Action")
	float CooldownSec ;

	// 코스트(탄약/마나/스태미나 등). 의미는 게임에서 정의
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Action")
	int32 Cost;

	//그 액션이 소유하는 애니메이션 몽타주, SF , VFX 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Action|FX")
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Action|FX")
	TObjectPtr<USoundBase> SFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Action|FX")
	TObjectPtr<UNiagaraSystem> VFX;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Action|Rules")
	TArray<FName> BlockOtherActionTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Action|Rules")
	TArray<FName> BlockedByActionTags;
};
UCLASS()
class INFINITYFIGHTER_API UActionLoadOutDA : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Action")
    TArray<FActionSlot> Actions;
	
	
};
