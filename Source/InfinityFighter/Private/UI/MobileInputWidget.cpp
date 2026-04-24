#include "UI/MobileInputWidget.h"
#include "Component/InputProxyComponent.h"
#include "Component/CBrainComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"

void UMobileInputWidget::Init(UInputProxyComponent* InProxy)
{
	Proxy = InProxy;
}

void UMobileInputWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// 터치 이벤트 수신 활성화
	SetIsFocusable(true);
}

// 버튼 존 비율 기반으로 초기화 (화면 크기 확정된 후 첫 터치 시 빌드)
void UMobileInputWidget::BuildButtonZones(const FGeometry& Geo)
{
	FVector2D Size = Geo.GetLocalSize();

	// 오른쪽 하단: Jump / Fire / Skill1 / Skill2 / Dodge
	// 비율 기준 (x: 오른쪽에서, y: 아래에서)
	ButtonZones = {
		{ "Input.Jump",   FVector2D(Size.X * 0.82f, Size.Y * 0.75f), 50.f },
		{ "Input.Fire",   FVector2D(Size.X * 0.92f, Size.Y * 0.60f), 50.f },
		{ "Input.Skill1", FVector2D(Size.X * 0.72f, Size.Y * 0.60f), 50.f },
		{ "Input.Skill2", FVector2D(Size.X * 0.92f, Size.Y * 0.45f), 50.f },
		{ "Input.Dodge",  FVector2D(Size.X * 0.72f, Size.Y * 0.45f), 50.f },
	};
	bButtonZonesBuilt = true;
}

bool UMobileInputWidget::IsButtonZone(const FVector2D& LocalPos, const FGeometry& Geo) const
{
	for (const FButtonZone& BZ : ButtonZones)
	{
		if (FVector2D::Distance(LocalPos, BZ.Center) <= BZ.Radius)
			return true;
	}
	return false;
}

FReply UMobileInputWidget::NativeOnTouchStarted(const FGeometry& Geo, const FPointerEvent& Event)
{
	if (!bButtonZonesBuilt) BuildButtonZones(Geo);

	FVector2D LocalPos = Geo.AbsoluteToLocal(Event.GetScreenSpacePosition());
	int32 Finger = (int32)Event.GetPointerIndex();
	FVector2D Size = Geo.GetLocalSize();

	// 버튼 존 먼저 체크
	for (const FButtonZone& BZ : ButtonZones)
	{
		if (FVector2D::Distance(LocalPos, BZ.Center) <= BZ.Radius)
		{
			if (Proxy) Proxy->EmitIntent(BZ.Intent);
			ActiveButtonFingers.Add(Finger, BZ.Intent);
			return FReply::Handled();
		}
	}

	// 왼쪽 절반 → 이동 조이스틱
	if (LocalPos.X < Size.X * 0.5f && !LeftStick.bActive)
	{
		LeftStick.bActive   = true;
		LeftStick.FingerIdx = Finger;
		LeftStick.Center    = LocalPos;
		LeftStick.Current   = LocalPos;
	}
	// 오른쪽 절반 → 시점 조이스틱
	else if (LocalPos.X >= Size.X * 0.5f && !RightStick.bActive)
	{
		RightStick.bActive   = true;
		RightStick.FingerIdx = Finger;
		RightStick.Center    = LocalPos;
		RightStick.Current   = LocalPos;
	}

	ProcessStickInput();
	return FReply::Handled();
}

FReply UMobileInputWidget::NativeOnTouchMoved(const FGeometry& Geo, const FPointerEvent& Event)
{
	FVector2D LocalPos = Geo.AbsoluteToLocal(Event.GetScreenSpacePosition());
	int32 Finger = (int32)Event.GetPointerIndex();

	if (LeftStick.bActive && LeftStick.FingerIdx == Finger)
		LeftStick.Current = LocalPos;
	else if (RightStick.bActive && RightStick.FingerIdx == Finger)
		RightStick.Current = LocalPos;

	ProcessStickInput();
	return FReply::Handled();
}

FReply UMobileInputWidget::NativeOnTouchEnded(const FGeometry& Geo, const FPointerEvent& Event)
{
	int32 Finger = (int32)Event.GetPointerIndex();

	// 버튼 손가락 해제
	ActiveButtonFingers.Remove(Finger);

	if (LeftStick.bActive && LeftStick.FingerIdx == Finger)
	{
		LeftStick = FJoystickState();
		if (Proxy)
		{
			Proxy->CachedForward = 0.f;
			Proxy->CachedRight   = 0.f;
			// Brain에 0 입력 전달해서 이동 정지
			if (Proxy->Brain)
				Proxy->Brain->OnMoveInput(FVector2D::ZeroVector);
		}
	}
	else if (RightStick.bActive && RightStick.FingerIdx == Finger)
	{
		RightStick = FJoystickState();
		// 시점은 0 전달 불필요 (Look은 델타값)
	}

	ProcessStickInput();
	return FReply::Handled();
}

void UMobileInputWidget::ProcessStickInput()
{
	if (!Proxy) return;

	// 왼쪽 조이스틱 → 이동
	if (LeftStick.bActive)
	{
		FVector2D Delta = LeftStick.Current - LeftStick.Center;
		float Len = Delta.Size();
		if (Len > LeftStick.Radius)
			Delta = Delta / Len * LeftStick.Radius;

		FVector2D Normalized = Delta / LeftStick.Radius;
		Proxy->CachedForward = Normalized.Y * -1.f; // Y축 반전 (위=전진)
		Proxy->CachedRight   = Normalized.X;

		if (Proxy->Brain)
			Proxy->Brain->OnMoveInput(FVector2D(Proxy->CachedRight, Proxy->CachedForward));
	}

	// 오른쪽 조이스틱 → 시점
	if (RightStick.bActive)
	{
		FVector2D Delta = RightStick.Current - RightStick.Center;
		float Len = Delta.Size();
		if (Len > RightStick.Radius)
			Delta = Delta / Len * RightStick.Radius;

		FVector2D Normalized = Delta / RightStick.Radius;
		// 시점은 델타 누적이 아닌 매 프레임 전달
		if (Proxy->Brain)
			Proxy->Brain->OnLookInput(FVector2D(Normalized.X * 3.f, Normalized.Y * -3.f));

		// 다음 프레임 비교를 위해 현재를 센터로 업데이트 (상대 이동)
		RightStick.Center = RightStick.Current;
	}
}
