#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MobileInputWidget.generated.h"

class UInputProxyComponent;
class UImage;
class UCanvasPanel;

// 조이스틱 하나의 상태
USTRUCT()
struct FJoystickState
{
	GENERATED_BODY()

	FVector2D Center    = FVector2D::ZeroVector;
	FVector2D Current   = FVector2D::ZeroVector;
	int32     FingerIdx = -1;
	bool      bActive   = false;
	float     Radius    = 80.f;
};

UCLASS()
class INFINITYFIGHTER_API UMobileInputWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// CharacterBase가 BeginPlay에서 주입
	void Init(UInputProxyComponent* InProxy);

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnTouchStarted (const FGeometry& Geo, const FPointerEvent& Event) override;
	virtual FReply NativeOnTouchMoved  (const FGeometry& Geo, const FPointerEvent& Event) override;
	virtual FReply NativeOnTouchEnded  (const FGeometry& Geo, const FPointerEvent& Event) override;

private:
	UPROPERTY()
	UInputProxyComponent* Proxy = nullptr;

	FJoystickState LeftStick;   // 왼쪽 절반 → 이동
	FJoystickState RightStick;  // 오른쪽 절반 (버튼 영역 제외) → 시점

	// 터치가 오른쪽 버튼 영역인지 판별 (버튼은 별도 처리)
	bool IsButtonZone(const FVector2D& LocalPos, const FGeometry& Geo) const;

	void ProcessStickInput();

	// 스킬 버튼 영역 (오른쪽 하단 고정 위치, 로컬 좌표 비율)
	struct FButtonZone { FName Intent; FVector2D Center; float Radius; };
	TArray<FButtonZone> ButtonZones;

	void BuildButtonZones(const FGeometry& Geo);
	bool bButtonZonesBuilt = false;

	// 버튼 터치 처리
	TMap<int32, FName> ActiveButtonFingers; // fingerIdx → intent
};
