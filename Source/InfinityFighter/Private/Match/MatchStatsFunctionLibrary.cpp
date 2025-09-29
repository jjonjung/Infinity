// MatchStatsFunctionLibrary.cpp

#include "Match/MatchStatsFunctionLibrary.h"
#include "Utility/ManagerController.h"
#include "Engine/World.h"

namespace
{
    static float ComputeKD(int32 Kills, int32 Deaths)
    {
        if (Deaths <= 0)
        {
            return static_cast<float>(Kills);
        }
        return static_cast<float>(Kills) / static_cast<float>(Deaths);
    }
}

FMatchResult UMatchStatsFunctionLibrary::BuildMatchResultForCeremony(const TArray<FPlayerMatchStat>& InStats)
{
    FMatchResult Result;

    
    Result.PlayerStats = InStats;
    for (FPlayerMatchStat& Stat : Result.PlayerStats)
    {
        Stat.KD = ComputeKD(Stat.Kills, Stat.Deaths);
    }

    
    Result.PlayerStats.StableSort([](const FPlayerMatchStat& A, const FPlayerMatchStat& B)
    {
        if (A.Kills != B.Kills)
        {
            return A.Kills > B.Kills;
        }
        if (A.Deaths != B.Deaths)
        {
            return A.Deaths < B.Deaths;
        }
        return A.DisplayName.ToString() < B.DisplayName.ToString();
    });

   
    for (int32 Index = 0; Index < Result.PlayerStats.Num(); ++Index)
    {
        Result.PlayerStats[Index].Rank = Index + 1;
    }

   
    if (Result.PlayerStats.Num() > 0)
    {
        Result.bValid = true;
        Result.TopKillerIndex = 0;
        Result.TopKills = Result.PlayerStats[0].Kills;
        Result.TopKillerName = Result.PlayerStats[0].DisplayName;
    }
    else
    {
        Result.bValid = false;
        Result.TopKillerIndex = -1;
        Result.TopKills = 0;
        Result.TopKillerName = FText();
    }

    return Result;
}


FMatchResult UMatchStatsFunctionLibrary::BuildMatchResultFromManager(const UObject* WorldContextObject)
{
    TArray<FPlayerMatchStat> Stats;

    if (WorldContextObject)
    {
        if (const UWorld* World = WorldContextObject->GetWorld())
        {
            if (const UGameInstance* GI = World->GetGameInstance())
            {
                if (const UManagerController* Manager = Cast<UManagerController>(GI))
                {
                    const TArray<FPlayerMatchInfo> All = Manager->GetAllPlayers();
                    Stats.Reserve(All.Num());
                    for (const FPlayerMatchInfo& Info : All)
                    {
                        FPlayerMatchStat S;
                        S.PlayerId = Info.Nickname.ToString();
                        S.DisplayName = FText::FromName(Info.Nickname);
                        S.Kills = Info.Kills;
                        S.Deaths = Info.Deaths;
                        Stats.Add(MoveTemp(S));
                    }
                }
            }
        }
    }

    return BuildMatchResultForCeremony(Stats);
}
