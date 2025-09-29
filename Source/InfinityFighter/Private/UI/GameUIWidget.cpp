#include "UI/GameUIWidget.h"

#include "Components/HorizontalBoxSlot.h"
#include "UI/GameTimerWidget.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBoxSlot.h"

void UGameUIWidget::UpdateTimer(float NewTime)
{
    TimerWidget->UpdateRemainTime(NewTime);
}
