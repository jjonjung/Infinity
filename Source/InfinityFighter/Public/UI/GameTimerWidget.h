#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameTimerWidget.generated.h"

class UTextBlock;

UCLASS()
class INFINITYFIGHTER_API UGameTimerWidget : public UUserWidget
{
    GENERATED_BODY()

public:
   	UPROPERTY(meta = (BindWidget))
   	TObjectPtr<UTextBlock> RemainTimeText;

	UFUNCTION(BlueprintCallable)
	void StartCountdown(float InDurationSec, bool bCountDown = true);

	UFUNCTION(BlueprintCallable)
	void UpdateRemainTime(float InSeconds);
   
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
private:
	double StartTime = 0.0;
	double Duration = 0.0;
	bool bIsCounting = false;
	bool bCountDownMode = true;
};
