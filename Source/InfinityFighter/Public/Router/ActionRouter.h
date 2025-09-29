// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionComponent.h"
#include "DA/ActionRouterDA.h"
#include "Components/ActorComponent.h"
#include "ActionRouter.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INFINITYFIGHTER_API UActionRouter : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UActionRouter();
private:
	class UActionComponent* ActionComponent;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere)
	TMap<FName,FName> RouterMap;
	

public:	 //등록/삭제/조회 
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION(BlueprintCallable)
	bool IsValidIntent(FName IntentName);//인텐트 조회
	UFUNCTION(BlueprintCallable)
	bool IsValidAction(FName ActionTag);
	UFUNCTION(BlueprintCallable)
	void RegisterActionRouter(FName IntentName, FName ActionRouterName);
	UFUNCTION(BlueprintCallable)
	void RemoveActionRouterByIntentName(FName IntentName);//
	UFUNCTION(BlueprintCallable)
	void RemoveActionRouterByActionTag(FName ActionTag);//
	UFUNCTION(BlueprintCallable)
	void InitActionRouterMap(TArray<FName> IntentNames,TArray<FName> ActionTags);
	UFUNCTION(BlueprintCallable)
	void ExcuteActionByIntent(FName Intent);
	UFUNCTION(BlueprintCallable)
	void InitFromRouterDA(UActionRouterDA* AcRouterDa);
	
	
	
	
			
};
