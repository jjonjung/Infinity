#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UI/MinimapDataProvider.h"
#include "MinimapDataActor.generated.h"

class AMapBoundsActor;

/**
 * 레벨에 배치하는 미니맵 데이터 공급 Actor.
 * IMinimapDataProvider 의 유일한 구현체.
 *
 * 책임:
 *   - 레벨의 모든 ACharacterBase 를 탐색
 *   - ObserverPawn 기준으로 Self / Friendly / Enemy / Neutral 분류
 *   - AMapBoundsActor 를 통해 월드 XY → UV 변환
 *   - FMinimapMarker 배열 반환
 *
 * UMinimapWidget 은 이 클래스를 전혀 알지 못한다 (DIP).
 * 새로운 마커(아이템 등)는 이 클래스만 수정하면 된다 (OCP).
 */
UCLASS()
class INFINITYFIGHTER_API AMinimapDataActor : public AActor, public IMinimapDataProvider
{
    GENERATED_BODY()

public:
    AMinimapDataActor();

    virtual TArray<FMinimapMarker> GetMinimapMarkers(const APawn* ObserverPawn) const override;

private:
    /** MapBoundsActor 지연 탐색 캐시 */
    mutable TWeakObjectPtr<AMapBoundsActor> CachedBounds;
    AMapBoundsActor* GetMapBounds() const;
};
