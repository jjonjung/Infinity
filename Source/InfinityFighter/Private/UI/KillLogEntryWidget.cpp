#include "UI/KillLogEntryWidget.h"

#include "Components/TextBlock.h"
#include "Utility/ManagerController.h"
#include "Engine/World.h"

void UKillLogEntryWidget::SetupFromEvent(const FKillLogEvent& Event)
{
    // Resolve team colors (fallback to simple red/blue/white if GI not available)
    FLinearColor killerColor = FLinearColor::White;
    FLinearColor victimColor = FLinearColor::White;
    if (UWorld* World = GetWorld())
    {
        if (UManagerController* MC = World->GetGameInstance<UManagerController>())
        {
            killerColor = MC->GetTeamColor(Event.KillerTeam);
            victimColor = MC->GetTeamColor(Event.VictimTeam);
        }
        else
        {
            switch (Event.KillerTeam)
            {
            case ETeam::Red:  killerColor = FLinearColor(1.f, 0.1f, 0.1f); break;
            case ETeam::Blue: killerColor = FLinearColor(0.1f, 0.2f, 1.f); break;
            default:          killerColor = FLinearColor::White;            break;
            }
            switch (Event.VictimTeam)
            {
            case ETeam::Red:  victimColor = FLinearColor(1.f, 0.1f, 0.1f); break;
            case ETeam::Blue: victimColor = FLinearColor(0.1f, 0.2f, 1.f); break;
            default:          victimColor = FLinearColor::White;            break;
            }
        }
    }

    if (KillerText)
    {
        KillerText->SetText(FText::FromString(Event.KillerName));
        KillerText->SetColorAndOpacity(FSlateColor(killerColor));
    }
    if (VictimText)
    {
        VictimText->SetText(FText::FromString(Event.VictimName));
        VictimText->SetColorAndOpacity(FSlateColor(victimColor));
    }
    if (StatText)
    {
        StatText->SetText(FText::FromString(FString::Printf(TEXT("K:%d D:%d"), Event.KillerKills, Event.KillerDeaths)));
        StatText->SetColorAndOpacity(FSlateColor(killerColor));
    }
}
