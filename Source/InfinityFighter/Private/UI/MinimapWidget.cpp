#include "UI/MinimapWidget.h"
#include "UI/MinimapDataProvider.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Brushes/SlateColorBrush.h"
#include "Rendering/DrawElements.h"

// ============================================================
// NativeConstruct — IMinimapDataProvider 구현체를 인터페이스로만 탐색
// ============================================================
void UMinimapWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            if (It->GetClass()->ImplementsInterface(UMinimapDataProvider::StaticClass()))
            {
                DataProviderActor = *It;
                break;
            }
        }
    }
}

// ============================================================
// NativeTick — Provider 유실 복구
// ============================================================
void UMinimapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!DataProviderActor.IsValid())
    {
        if (UWorld* World = GetWorld())
        {
            for (TActorIterator<AActor> It(World); It; ++It)
            {
                if (It->GetClass()->ImplementsInterface(UMinimapDataProvider::StaticClass()))
                {
                    DataProviderActor = *It;
                    break;
                }
            }
        }
    }
}

// ============================================================
// NativePaint — 렌더링만. 게임 로직 전혀 없음.
// ============================================================
int32 UMinimapWidget::NativePaint(
    const FPaintArgs& Args,
    const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements,
    int32 LayerId,
    const FWidgetStyle& InWidgetStyle,
    bool bParentEnabled) const
{
    LayerId = Super::NativePaint(Args, AllottedGeometry, MyCullingRect,
        OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

    if (!DataProviderActor.IsValid()) return LayerId;

    const FVector2D WidgetSize = AllottedGeometry.GetLocalSize();

    // 1. 배경
    {
        static const FSlateColorBrush FallbackBrush(FLinearColor::White);
        const FSlateBrush* BgBrush = BackgroundBrush.HasUObject()
            ? &BackgroundBrush
            : static_cast<const FSlateBrush*>(&FallbackBrush);

        FSlateDrawElement::MakeBox(OutDrawElements, LayerId,
            AllottedGeometry.ToPaintGeometry(),
            BgBrush, ESlateDrawEffect::None, BackgroundColor);
    }
    ++LayerId;

    // 2. ObserverPawn 획득
    const APawn* Observer = nullptr;
    if (APlayerController* PC = GetOwningPlayer())
    {
        Observer = PC->GetPawn();
    }

    // 3. 마커 목록 수신 — 인터페이스 호출, 구체 타입 불필요
    const IMinimapDataProvider* Provider =
        Cast<IMinimapDataProvider>(DataProviderActor.Get());
    if (!Provider) return LayerId;

    const TArray<FMinimapMarker> Markers = Provider->GetMinimapMarkers(Observer);

    // 4. dot 렌더링 — 스타일 결정은 ResolveMarkerStyle 이 집중 처리
    for (const FMinimapMarker& Marker : Markers)
    {
        FLinearColor Color;
        float        Size;
        ResolveMarkerStyle(Marker.Type, Color, Size);

        const FVector2D Center(Marker.UV.X * WidgetSize.X,
                               Marker.UV.Y * WidgetSize.Y);
        DrawDot(AllottedGeometry, OutDrawElements, LayerId, Center, Size, Color);
    }

    return LayerId;
}

// ============================================================
// ResolveMarkerStyle — MarkerType → 시각 스타일 변환 정책
//   색상·크기 변경이 필요하면 이 함수만 수정하면 된다.
// ============================================================
void UMinimapWidget::ResolveMarkerStyle(EMinimapMarkerType Type,
                                        FLinearColor& OutColor,
                                        float& OutSize) const
{
    switch (Type)
    {
    case EMinimapMarkerType::Self:
        OutColor = SelfColor;
        OutSize  = SelfDotSize;
        break;
    case EMinimapMarkerType::Friendly:
        OutColor = FriendlyColor;
        OutSize  = DotSize;
        break;
    case EMinimapMarkerType::Enemy:
        OutColor = EnemyColor;
        OutSize  = DotSize;
        break;
    default:
        OutColor = NeutralColor;
        OutSize  = DotSize;
        break;
    }
}

// ============================================================
// DrawDot — Center 기준 Size×Size 사각형 dot
// ============================================================
void UMinimapWidget::DrawDot(
    const FGeometry& Geometry,
    FSlateWindowElementList& OutDrawElements,
    int32 LayerId,
    FVector2D Center,
    float Size,
    FLinearColor Color) const
{
    static const FSlateColorBrush DotBrush(FLinearColor::White);

    const FVector2D TopLeft = Center - FVector2D(Size * 0.5f, Size * 0.5f);

    FSlateDrawElement::MakeBox(
        OutDrawElements, LayerId,
        Geometry.ToPaintGeometry(FVector2D(Size, Size), FSlateLayoutTransform(TopLeft)),
        &DotBrush, ESlateDrawEffect::None, Color);
}
