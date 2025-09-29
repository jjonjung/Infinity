#include "UI/GameTimerWidget.h"
#include "Components/TextBlock.h"

void UGameTimerWidget::StartCountdown(float InDurationSec, bool bCountDown)
{
    Duration        = InDurationSec;
    bCountDownMode  = bCountDown;                   
    StartTime       = GetWorld()->GetTimeSeconds();
    bIsCounting     = true;
}

void UGameTimerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    if (!bIsCounting) return;

    const double Now     = GetWorld()->GetTimeSeconds();
    const double Elapsed = Now - StartTime;
    const double Remain  = FMath::Max(0.0, Duration - Elapsed);

    const double Show = bCountDownMode ? Remain : Elapsed;
    UpdateRemainTime(static_cast<float>(Show));

    if (bCountDownMode && Remain <= 0.0)
    {
        bIsCounting = false;
    }
}

void UGameTimerWidget::UpdateRemainTime(float time)
{
    const int32 IntTime = FMath::FloorToInt(time + 1.0f);
    const int32 Min     = IntTime / 60;
    const int32 Sec     = IntTime % 60;

    if (RemainTimeText)
    {
        if (time <= 0.0f)
            RemainTimeText->SetText(FText::FromString(TEXT("0:00")));
        else
            RemainTimeText->SetText(FText::FromString(FString::Printf(TEXT("%d:%02d"), Min, Sec)));
    }
}
