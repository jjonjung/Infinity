// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleManager.h"
#include "Utility/ManagerController.h"
#include "Utility/DebugUtil.h"
#include "Engine/World.h"
#include "Chatacter/MyDoctorStrange.h"
#include "Chatacter/MySpiderMan.h"
#include "Chatacter/MyIronMan.h"


ABattleManager::ABattleManager() {}


void ABattleManager::BeginPlay()
{
    Super::BeginPlay();
}


void ABattleManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void ABattleManager::OnCharacterKilled(ACharacterBase* Killer, ACharacterBase* Victim)
{
    if (!Victim)
    {
        return;
    }

    auto TryAssignDefaultIdentity = [](ACharacterBase* C)
    {
        if (!C) return;
        if (C->MatchNickname == NAME_None)
        {
            if (Cast<AMyDoctorStrange>(C))      { C->MatchNickname = FName(TEXT("나는야경덕왕")); }
            else if (Cast<AMySpiderMan>(C))     { C->MatchNickname = FName(TEXT("나영킴")); }
            else if (Cast<AMyIronMan>(C))       { C->MatchNickname = FName(TEXT("쫀정")); }
        }
        if (C->MatchTeam == ETeam::None)
        {
            if (Cast<AMyDoctorStrange>(C))      { C->MatchTeam = ETeam::Red; }
            else if (Cast<AMySpiderMan>(C))     { C->MatchTeam = ETeam::Blue; }
            else if (Cast<AMyIronMan>(C))       { C->MatchTeam = ETeam::Blue; }
        }
    };

    TryAssignDefaultIdentity(Killer);
    TryAssignDefaultIdentity(Victim);

    ReportKillToGameInstance(Killer, Victim);

    // Prefer match nicknames
    const FString KillerName = (Killer && Cast<ACharacterBase>(Killer) && Cast<ACharacterBase>(Killer)->MatchNickname != NAME_None)
        ? Cast<ACharacterBase>(Killer)->MatchNickname.ToString()
        : (Killer ? Killer->GetName() : TEXT("<Unknown>"));
    const FString VictimName = (Victim && Cast<ACharacterBase>(Victim) && Cast<ACharacterBase>(Victim)->MatchNickname != NAME_None)
        ? Cast<ACharacterBase>(Victim)->MatchNickname.ToString()
        : (Victim ? Victim->GetName() : TEXT("<Unknown>"));

    
    ETeam KillerTeam = ETeam::None;
    int32 KillerKills = 0;
    int32 KillerDeaths = 0;
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (const UManagerController* MC = Cast<UManagerController>(GI))
            {
                KillerTeam = MC->GetTeam(*KillerName);
                KillerKills = MC->GetKills(*KillerName);
                KillerDeaths = MC->GetDeaths(*KillerName);
            }
        }
    }

    Debug::Screen(FString::Printf(TEXT("BM: OnCharacterKilled %s -> %s"), *KillerName, *VictimName), 3.0f, FColor::Yellow);
    AddKillLog(KillerName, VictimName, KillerTeam, KillerKills, KillerDeaths);
    UpdateScoreboard();
}

void ABattleManager::OnCharacterRespawned(ACharacterBase* /*Character*/)
{
   
}

void ABattleManager::AddKillLog(const FString& KillerName, const FString& VictimName, ETeam KillerTeam, int32 KillerKills, int32 KillerDeaths)
{
    const TCHAR* TeamTag = TEXT("?");
    switch (KillerTeam)
    {
    case ETeam::Red:  TeamTag = TEXT("R"); break;
    case ETeam::Blue: TeamTag = TEXT("B"); break;
    default:          TeamTag = TEXT("-"); break;
    }
    const FString Line = FString::Printf(TEXT("[%s] %s -> %s  (K:%d D:%d)"), TeamTag, *KillerName, *VictimName, KillerKills, KillerDeaths);
    KillLogs.Add(Line);

    FColor Color = FColor::White;
    switch (KillerTeam)
    {
    case ETeam::Red:  Color = FColor::Red;  break;
    case ETeam::Blue: Color = FColor::Blue; break;
    default:          Color = FColor::White; break;
    }
    Debug::Screen(Line, 2.5f, Color);

    // Notify UI
    FKillLogEvent Ev;
    Ev.KillerName  = KillerName;
    Ev.VictimName  = VictimName;
    Ev.KillerTeam  = KillerTeam;
    // Look up victim team for UI coloring
    Ev.VictimTeam  = ETeam::None;
    if (UWorld* World = GetWorld())
    {
        if (const UGameInstance* GI = World->GetGameInstance())
        {
            if (const UManagerController* MC = Cast<UManagerController>(GI))
            {
                Ev.VictimTeam = MC->GetTeam(*VictimName);
            }
        }
    }
    Ev.KillerKills = KillerKills;
    Ev.KillerDeaths= KillerDeaths;
    OnKillLogAdded.Broadcast(Ev);
}

void ABattleManager::ClearAllKillLogs()
{
    KillLogs.Reset();
}

void ABattleManager::UpdateScoreboard()
{
    
}

void ABattleManager::ReportKillToGameInstance(ACharacter* Killer, ACharacter* Victim)
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UManagerController* MC = Cast<UManagerController>(GI))
            {
                ACharacterBase* KBase = Cast<ACharacterBase>(Killer);
                ACharacterBase* VBase = Cast<ACharacterBase>(Victim);

                
                auto TryAssignDefaultIdentity = [](ACharacterBase* C)
                {
                    if (!C) return;
                    if (C->MatchNickname == NAME_None)
                    {
                        if (Cast<AMyDoctorStrange>(C))      { C->MatchNickname = FName(TEXT("ykd0638")); }
                        else if (Cast<AMySpiderMan>(C))     { C->MatchNickname = FName(TEXT("nayeong")); }
                        else if (Cast<AMyIronMan>(C))       { C->MatchNickname = FName(TEXT("jjg")); }
                    }
                    if (C->MatchTeam == ETeam::None)
                    {
                        if (Cast<AMyDoctorStrange>(C))      { C->MatchTeam = ETeam::Red; }
                        else if (Cast<AMySpiderMan>(C))     { C->MatchTeam = ETeam::Blue; }
                        else if (Cast<AMyIronMan>(C))       { C->MatchTeam = ETeam::Blue; }
                    }
                };
                TryAssignDefaultIdentity(KBase);
                TryAssignDefaultIdentity(VBase);
                const FName KillerNick = (KBase && KBase->MatchNickname != NAME_None)
                    ? KBase->MatchNickname
                    : (Killer ? FName(*Killer->GetName()) : NAME_None);
                const FName VictimNick = (VBase && VBase->MatchNickname != NAME_None)
                    ? VBase->MatchNickname
                    : (Victim ? FName(*Victim->GetName()) : NAME_None);

                
                if (KillerNick != NAME_None)
                {
                    const ETeam KTeam = KBase ? KBase->MatchTeam : ETeam::None;
                    MC->RegisterPlayer(KillerNick, KTeam);
                }
                if (VictimNick != NAME_None)
                {
                    const ETeam VTeam = VBase ? VBase->MatchTeam : ETeam::None;
                    MC->RegisterPlayer(VictimNick, VTeam);
                }

                if (KillerNick != NAME_None && VictimNick != NAME_None)
                {
                    MC->AddKillFor(KillerNick, VictimNick);
                }
                else if (KillerNick != NAME_None)
                {
                    MC->AddKillOnly(KillerNick);
                }
                else if (VictimNick != NAME_None)
                {
                    MC->AddDeathOnly(VictimNick);
                }
            }
        }
    }
}

