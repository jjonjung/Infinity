#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "MapBoundsActor.generated.h"

/**
 * 레벨에 배치하는 AABB 바운딩 박스 Actor.
 * BoundsBox의 위치 + Extent 로 맵 XY 범위를 정의하고
 * UV(0~1) ↔ 월드 XY 좌표 변환을 제공한다.
 * 미니맵 시스템의 좌표 변환 기반.
 */
UCLASS()
class INFINITYFIGHTER_API AMapBoundsActor : public AActor
{
    GENERATED_BODY()

public:
    AMapBoundsActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="MapBounds")
    TObjectPtr<UBoxComponent> BoundsBox;

    /** AABB 바운딩 박스 계산 (XY 평면) */
    UFUNCTION(BlueprintCallable, Category="MapBounds")
    FBox2D GetXYBounds() const;

    /** UV (0~1) → 월드 XY 좌표 (선형 보간) */
    UFUNCTION(BlueprintCallable, Category="MapBounds")
    FVector2D UVToWorldXY(const FVector2D& UV) const;

    /** 월드 XY → UV (0~1) (범위 매핑 + 클램프) */
    UFUNCTION(BlueprintCallable, Category="MapBounds")
    FVector2D WorldXYToUV(const FVector2D& WorldXY) const;
};
