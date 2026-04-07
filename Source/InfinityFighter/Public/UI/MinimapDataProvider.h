#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UI/MinimapTypes.h"
#include "MinimapDataProvider.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UMinimapDataProvider : public UInterface
{
    GENERATED_BODY()
};

/**
 * 미니맵에 표시할 마커 목록을 공급하는 인터페이스.
 *
 * UMinimapWidget 은 이 인터페이스만 알고, 실제 구현(캐릭터 탐색·팀 분류)은
 * AMinimapDataActor 또는 다른 구현체가 담당한다.
 * 새로운 마커 소스(아이템, 목표 지점 등)를 추가할 때 Widget 을 수정하지 않아도 된다.
 *
 * @param ObserverPawn  마커 타입 분류 기준이 되는 관찰자 폰 (아군/적 판단용)
 * @return              UV(0~1) + MarkerType 배열
 */
class INFINITYFIGHTER_API IMinimapDataProvider
{
    GENERATED_BODY()

public:
    virtual TArray<FMinimapMarker> GetMinimapMarkers(const APawn* ObserverPawn) const = 0;
};
