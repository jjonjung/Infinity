#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Utility/BattleTypes.h"
#include "KillLogEntryWidget.generated.h"

class UTextBlock;

UCLASS()
class INFINITYFIGHTER_API UKillLogEntryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="KillLog")
    void SetupFromEvent(const FKillLogEvent& Event);

protected:
    UPROPERTY(meta=(BindWidget)) UTextBlock* KillerText = nullptr;
    UPROPERTY(meta=(BindWidget)) UTextBlock* VictimText = nullptr;
    UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* StatText = nullptr; // optional: K/D
};

