// Fill out your copyright notice in the Description page of Project Settings.


#include "EndingGameModeBase.h"
#include "Utility/ManagerController.h"
#include "Blueprint/UserWidget.h"
#include "Match/MatchResults.h"


void AEndingGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	const UManagerController* GI = GetGameInstance<UManagerController>();
	if (!GI) return;

	const FMatchResult MatchResult = GI->GetMatchResult();
	if (!MatchResult.bValid) {return;} 

	// 1) 엔딩 위젯 생성/표시
	if (CeremonyWidgetClass)
	{
		if (UUserWidget* W = CreateWidget<UUserWidget>(GetWorld(), CeremonyWidgetClass))
		{
			W->AddToViewport();
			
		}
	}

	
}
