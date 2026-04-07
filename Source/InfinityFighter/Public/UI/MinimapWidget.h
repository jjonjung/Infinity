#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/MinimapTypes.h"
#include "MinimapWidget.generated.h"

class IMinimapDataProvider;

/**
 * 미니맵 렌더링 전용 위젯.
 *
 * 이 클래스가 아는 것:
 *   - IMinimapDataProvider  (추상 인터페이스)
 *   - FMinimapMarker / EMinimapMarkerType  (순수 데이터)
 *   - 각 MarkerType 의 시각적 표현 (색상, 크기) — 스타일 소유권은 위젯에 있다
 *
 * 이 클래스가 모르는 것:
 *   - ACharacterBase, ETeam, AMapBoundsActor, 게임 규칙 일체
 *
 * [에디터 설정]
 *   - 레벨에 AMinimapDataActor, AMapBoundsActor 를 배치할 것.
 *   - BackgroundBrush 에 맵 텍스처를 할당하면 배경 이미지로 사용된다.
 */
UCLASS()
class INFINITYFIGHTER_API UMinimapWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    virtual int32 NativePaint(
        const FPaintArgs& Args,
        const FGeometry& AllottedGeometry,
        const FSlateRect& MyCullingRect,
        FSlateWindowElementList& OutDrawElements,
        int32 LayerId,
        const FWidgetStyle& InWidgetStyle,
        bool bParentEnabled) const override;

    // ── 스타일: MarkerType 별 시각 표현 ──────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Minimap|Style")
    FSlateBrush BackgroundBrush;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Minimap|Style")
    FLinearColor BackgroundColor = FLinearColor(0.f, 0.f, 0.f, 0.65f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Minimap|Style")
    float DotSize = 8.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Minimap|Style")
    float SelfDotSize = 12.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Minimap|Style")
    FLinearColor SelfColor     = FLinearColor(1.f, 1.f, 0.f,   1.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Minimap|Style")
    FLinearColor FriendlyColor = FLinearColor(0.f, 0.85f, 0.f, 1.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Minimap|Style")
    FLinearColor EnemyColor    = FLinearColor(1.f, 0.15f, 0.15f, 1.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Minimap|Style")
    FLinearColor NeutralColor  = FLinearColor(0.7f, 0.7f, 0.7f, 1.f);

private:
    /** IMinimapDataProvider 를 구현하는 Actor 캐시 (구체 타입 불필요) */
    TWeakObjectPtr<AActor> DataProviderActor;

    /** MarkerType → (Color, Size) 변환 — 스타일 정책 한 곳에 집중 */
    void ResolveMarkerStyle(EMinimapMarkerType Type,
                            FLinearColor& OutColor, float& OutSize) const;

    void DrawDot(const FGeometry& Geometry,
                 FSlateWindowElementList& OutDrawElements,
                 int32 LayerId,
                 FVector2D Center,
                 float Size,
                 FLinearColor Color) const;
};
