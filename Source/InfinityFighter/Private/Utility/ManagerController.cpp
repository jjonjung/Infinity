// Copyright

#include "Utility/ManagerController.h"

#include "Match/MatchResults.h"

void UManagerController::Init()
{
    Super::Init();
    PlayerTable.Empty();
    GAreScreenMessagesEnabled = false; 
}

void UManagerController::Shutdown()
{
    PlayerTable.Empty();
    Super::Shutdown();
}

void UManagerController::RegisterPlayer(FName Nick, ETeam Team)
{
    if (Nick.IsNone()) return;
    FPlayerMatchInfo* Found = PlayerTable.Find(Nick);
    if (!Found)
    {
        FPlayerMatchInfo Info;
        Info.Nickname = Nick;
        Info.Team = Team;
        PlayerTable.Add(Nick, MoveTemp(Info));
    }
    else
    {
        Found->Team = Team;
    }
}

void UManagerController::UnregisterPlayer(FName Nick)
{
    PlayerTable.Remove(Nick);
}

void UManagerController::ResetAllPlayers()
{
    PlayerTable.Empty();
}

void UManagerController::SetTeam(FName Nick, ETeam NewTeam)
{
    if (FPlayerMatchInfo* Info = PlayerTable.Find(Nick))
    {
        Info->Team = NewTeam;
    }
}

void UManagerController::RenamePlayer(FName OldName, FName NewName)
{
    if (OldName == NewName || OldName.IsNone() || NewName.IsNone()) return;
    if (FPlayerMatchInfo* Info = PlayerTable.Find(OldName))
    {
        FPlayerMatchInfo Copy = *Info;
        Copy.Nickname = NewName;
        PlayerTable.Remove(OldName);
        PlayerTable.Add(NewName, MoveTemp(Copy));
    }
}

void UManagerController::AddKillFor(FName Killer, FName Victim)
{
    if (!Killer.IsNone())
    {
        FPlayerMatchInfo& K = PlayerTable.FindOrAdd(Killer);
        if (K.Nickname.IsNone()) K.Nickname = Killer;
        ++K.Kills;
    }
    if (!Victim.IsNone())
    {
        FPlayerMatchInfo& V = PlayerTable.FindOrAdd(Victim);
        if (V.Nickname.IsNone()) V.Nickname = Victim;
        ++V.Deaths;
    }
}

void UManagerController::AddKillOnly(FName Nick)
{
    if (Nick.IsNone()) return;
    FPlayerMatchInfo& Info = PlayerTable.FindOrAdd(Nick);
    if (Info.Nickname.IsNone()) Info.Nickname = Nick;
    ++Info.Kills;
}

void UManagerController::AddDeathOnly(FName Nick)
{
    if (Nick.IsNone()) return;
    FPlayerMatchInfo& Info = PlayerTable.FindOrAdd(Nick);
    if (Info.Nickname.IsNone()) Info.Nickname = Nick;
    ++Info.Deaths;
}

void UManagerController::ResetStats(FName Nick)
{
    if (FPlayerMatchInfo* Info = PlayerTable.Find(Nick))
    {
        Info->Kills = 0;
        Info->Deaths = 0;
    }
}

bool UManagerController::GetPlayerInfo(FName Nick, FPlayerMatchInfo& OutInfo) const
{
    if (const FPlayerMatchInfo* Found = PlayerTable.Find(Nick))
    {
        OutInfo = *Found;
        return true;
    }
    return false;
}

TArray<FPlayerMatchInfo> UManagerController::GetAllPlayers() const
{
    TArray<FPlayerMatchInfo> Out;
    Out.Reserve(PlayerTable.Num());
    for (const TPair<FName, FPlayerMatchInfo>& It : PlayerTable)
    {
        Out.Add(It.Value);
    }
    return Out;
}

int32 UManagerController::GetKills(FName Nick) const
{
    if (const FPlayerMatchInfo* Found = PlayerTable.Find(Nick))
    {
        return Found->Kills;
    }
    return 0;
}

int32 UManagerController::GetDeaths(FName Nick) const
{
    if (const FPlayerMatchInfo* Found = PlayerTable.Find(Nick))
    {
        return Found->Deaths;
    }
    return 0;
}

ETeam UManagerController::GetTeam(FName Nick) const
{
    if (const FPlayerMatchInfo* Found = PlayerTable.Find(Nick))
    {
        return Found->Team;
    }
    return ETeam::None;
}

FLinearColor UManagerController::GetTeamColor(ETeam Team) const
{
    switch (Team)
    {
    case ETeam::Red:  return FLinearColor(1.f, 0.1f, 0.1f);
    case ETeam::Blue: return FLinearColor(0.1f, 0.2f, 1.f);
    default:          return FLinearColor(0.6f, 0.6f, 0.6f);
    }
}

FMatchResult UManagerController::GetMatchResult() const
{
    FMatchResult Result;
    if (PlayerTable.Num() == 0)
    {
        return Result;
    }

    // Convert FPlayerMatchInfo to FPlayerMatchStat and populate PlayerStats
    for (const TPair<FName, FPlayerMatchInfo>& It : PlayerTable)
    {
        FPlayerMatchStat Stat;
        Stat.PlayerId = It.Value.Nickname.ToString();
        Stat.DisplayName = FText::FromName(It.Value.Nickname);
        Stat.Kills = It.Value.Kills;
        Stat.Deaths = It.Value.Deaths;
        // KD calculation
        Stat.KD = (Stat.Deaths == 0) ? (float)Stat.Kills : (float)Stat.Kills / Stat.Deaths;
        Result.PlayerStats.Add(Stat);
    }

    // Sort PlayerStats by Kills in descending order
    Result.PlayerStats.Sort([](const FPlayerMatchStat& A, const FPlayerMatchStat& B)
    {
        return A.Kills > B.Kills;
    });

    // Assign Ranks and identify Top Killer
    int32 CurrentRank = 1;
    if (Result.PlayerStats.Num() > 0)
    {
        Result.TopKills = Result.PlayerStats[0].Kills;
        Result.TopKillerName = Result.PlayerStats[0].DisplayName;
        Result.TopKillerIndex = 0;
        Result.PlayerStats[0].Rank = CurrentRank;

        for (int32 i = 1; i < Result.PlayerStats.Num(); ++i)
        {
            if (Result.PlayerStats[i].Kills < Result.PlayerStats[i-1].Kills)
            {
                CurrentRank++;
            }
            Result.PlayerStats[i].Rank = CurrentRank;
        }
    }

    Result.bValid = true;
    return Result;
}

void UManagerController::SetMatchResult(const FMatchResult& InResult)
{
    CachedMatchResult = InResult;
}

