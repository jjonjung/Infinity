// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TestPlayerBrain.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INFINITYFIGHTER_API UTestPlayerBrain : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UTestPlayerBrain();
	UPROPERTY(Transient, VisibleAnywhere, Category="Brain")
	class UActionRouter* ActionRouter = nullptr;

	
	UPROPERTY(EditAnywhere, Category="Input")
	class UInputMappingContext* MappingContext = nullptr;

	
	UPROPERTY(EditAnywhere, Category="Input")
	int32 MappingPriority = 0;

	
	UPROPERTY(EditAnywhere, Category="Input")
	class UInputAction* IA_Fire = nullptr;


	UPROPERTY(EditAnywhere, Category="Intent")
	FName FireIntent = "Input.Fire";
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	void SetupEnhancedInput(); // 인풋 설정
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void BindEnhancedActions();// 
	void OnFireStarted(const struct FInputActionValue& Value); //파이어 인풋이 들어오면  실행될 함수
	void EmitIntent(FName IntentTag); //인텐트에 따른 실행
		
};
