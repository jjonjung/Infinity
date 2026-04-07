#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "GameUIWidget.generated.h"

class UGameTimerWidget;
class UMinimapWidget;

UCLASS()
class INFINITYFIGHTER_API UGameUIWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta=(BindWidgetOptional))
    TObjectPtr<UGameTimerWidget> TimerWidget;

    UPROPERTY(meta=(BindWidgetOptional))
    TObjectPtr<UMinimapWidget> MinimapWidget;

    // 타이머 업데이트 함수
    void UpdateTimer(float NewTime);

};
