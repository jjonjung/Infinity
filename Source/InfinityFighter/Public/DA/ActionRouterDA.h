// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ActionRouterDA.generated.h"

USTRUCT(BlueprintType)
struct FRoutePair
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Routing")
	FName IntentTag;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Routing")
	FName ActionTag;

	FRoutePair()
		: IntentTag(NAME_None)
		, ActionTag(NAME_None)
	{}

	FRoutePair(const FName& InIntent, const FName& InAction)
		: IntentTag(InIntent)
		, ActionTag(InAction)
	{}
};
UCLASS(BlueprintType)
class INFINITYFIGHTER_API UActionRouterDA : public UDataAsset
{
	GENERATED_BODY()
public:
	/** Intent ↔ ActionTag 라우팅 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Routing")
	TArray<FRoutePair> Routes;

#if WITH_EDITORONLY_DATA
	/** 에디터에서 설명용 */
	UPROPERTY(EditAnywhere, Category="Description")
	FString Description;
#endif
};
