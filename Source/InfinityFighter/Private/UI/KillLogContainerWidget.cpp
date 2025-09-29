#include "UI/KillLogContainerWidget.h"

#include "BattleManager.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Utility/DebugUtil.h"

void UKillLogContainerWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!EntryWidgetClass)
    {
        EntryWidgetClass = UKillLogEntryWidget::StaticClass();
    }

    if (UWorld* World = GetWorld())
    {
        Debug::Screen(TEXT("KillLogContainer: Searching BattleManager"), 2.f, FColor::Silver);
        for (TActorIterator<ABattleManager> It(World); It; ++It)
        {
            BattleManager = *It;
            break;
        }

        if (BattleManager.IsValid())
        {
            BattleManager->OnKillLogAdded.AddDynamic(this, &UKillLogContainerWidget::HandleKillLogAdded);
            Debug::Screen(TEXT("KillLogContainer: Bound to BattleManager"), 2.5f, FColor::Green);
        }
        else
        {
            Debug::Screen(TEXT("KillLogContainer: BattleManager NOT found"), 3.0f, FColor::Red);
        }
    }
}

void UKillLogContainerWidget::NativeDestruct()
{
    if (BattleManager.IsValid())
    {
        BattleManager->OnKillLogAdded.RemoveDynamic(this, &UKillLogContainerWidget::HandleKillLogAdded);
    }
    Super::NativeDestruct();
}

void UKillLogContainerWidget::HandleKillLogAdded(const FKillLogEvent& Event)
{
    Debug::Screen(FString::Printf(TEXT("KillLogContainer: Event %s -> %s"), *Event.KillerName, *Event.VictimName), 2.5f, FColor::Cyan);
    AddKillLogEntry(Event);
}

void UKillLogContainerWidget::AddKillLogEntry(const FKillLogEvent& Event)
{
    if (!LogList || !EntryWidgetClass) return;

    UKillLogEntryWidget* Entry = CreateWidget<UKillLogEntryWidget>(GetWorld(), EntryWidgetClass);
    if (!Entry) return;

    Entry->SetupFromEvent(Event);

    // Insert newest at the top (index 0)
    LogList->InsertChildAt(0, Entry);
    Debug::Screen(TEXT("KillLogContainer: Inserted new entry at top"), 2.f, FColor::White);

    // Trim overflow
    while (MaxEntries > 0 && LogList->GetChildrenCount() > MaxEntries)
    {
        UWidget* Last = LogList->GetChildAt(LogList->GetChildrenCount() - 1);
        LogList->RemoveChild(Last);
    }

    // Auto-remove after lifetime
    if (EntryLifetime > 0.f)
    {
        FTimerDelegate Del;
        Del.BindUFunction(this, FName("OnEntryExpired"), Entry);
        FTimerHandle Handle;
        GetWorld()->GetTimerManager().SetTimer(Handle, Del, EntryLifetime, false);
    }
}

void UKillLogContainerWidget::OnEntryExpired(UUserWidget* ExpiredEntry)
{
    if (!LogList || !ExpiredEntry) return;
    LogList->RemoveChild(ExpiredEntry);
}
