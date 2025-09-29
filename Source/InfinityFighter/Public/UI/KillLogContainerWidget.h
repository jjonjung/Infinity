#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/KillLogEntryWidget.h"
#include "KillLogContainerWidget.generated.h"

class UVerticalBox;
class ABattleManager;

UCLASS()
class INFINITYFIGHTER_API UKillLogContainerWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION(BlueprintCallable, Category="KillLog")
    void AddKillLogEntry(const FKillLogEvent& Event);

protected:
    UPROPERTY(meta=(BindWidget)) UVerticalBox* LogList = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="KillLog")
    TSubclassOf<UKillLogEntryWidget> EntryWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="KillLog")
    int32 MaxEntries = 5;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="KillLog")
    float EntryLifetime = 5.0f;

protected:
    UFUNCTION()
    void HandleKillLogAdded(const FKillLogEvent& Event);

    UFUNCTION()
    void OnEntryExpired(UUserWidget* ExpiredEntry);

private:
    TWeakObjectPtr<ABattleManager> BattleManager;
};

