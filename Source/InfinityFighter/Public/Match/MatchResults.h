// MatchResults.h
// Minimal single-player match result DTOs for ceremony UI

#pragma once

#include "CoreMinimal.h"
#include "MatchResults.generated.h"

USTRUCT(BlueprintType)
struct INFINITYFIGHTER_API FPlayerMatchStat
{
    GENERATED_BODY()

public:
    // Unique id (optional in single-player). Can be name if no GUID.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Match|Stats")
    FString PlayerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Match|Stats")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Match|Stats")
    int32 Kills = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Match|Stats")
    int32 Deaths = 0;

    // Calculated fields (filled by result builder)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Match|Stats")
    int32 Rank = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Match|Stats")
    float KD = 0.f;
};

USTRUCT(BlueprintType)
struct INFINITYFIGHTER_API FMatchResult
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Match|Stats")
    bool bValid = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Match|Stats")
    int32 TopKills = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Match|Stats")
    FText TopKillerName;

    // Index in PlayerStats array after sorting; -1 if none
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Match|Stats")
    int32 TopKillerIndex = -1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Match|Stats")
    TArray<FPlayerMatchStat> PlayerStats;
};

