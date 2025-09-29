// Shared battle/match types

#pragma once

#include "CoreMinimal.h"
#include "BattleTypes.generated.h"

UENUM(BlueprintType)
enum class ETeam : uint8
{
    None  UMETA(DisplayName = "None"),
    Red   UMETA(DisplayName = "Red"),
    Blue  UMETA(DisplayName = "Blue"),
};

USTRUCT(BlueprintType)
struct FPlayerMatchInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName  Nickname;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32  Kills  = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32  Deaths = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) ETeam  Team   = ETeam::None;
};

// Kill log event payload for UI/analytics
USTRUCT(BlueprintType)
struct FKillLogEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="KillLog") FString KillerName;
    UPROPERTY(BlueprintReadOnly, Category="KillLog") FString VictimName;
    UPROPERTY(BlueprintReadOnly, Category="KillLog") ETeam   KillerTeam = ETeam::None;
    UPROPERTY(BlueprintReadOnly, Category="KillLog") ETeam   VictimTeam = ETeam::None;
    UPROPERTY(BlueprintReadOnly, Category="KillLog") int32   KillerKills = 0;
    UPROPERTY(BlueprintReadOnly, Category="KillLog") int32   KillerDeaths = 0;
};

