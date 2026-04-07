#pragma once

#include "CoreMinimal.h"
#include "MinimapTypes.generated.h"

/**
 * 미니맵에 표시되는 dot 의 분류.
 * 게임 로직(팀, 캐릭터 종류)과 렌더링을 분리하기 위한 추상 타입.
 * 위젯은 이 타입만 알고, 실제 분류 로직은 IMinimapDataProvider 구현체가 담당한다.
 */
UENUM(BlueprintType)
enum class EMinimapMarkerType : uint8
{
    Self,       // 로컬 플레이어 자신
    Friendly,   // 같은 팀
    Enemy,      // 적 팀
    Neutral,    // 팀 미분류
};

/**
 * 미니맵 dot 하나를 표현하는 순수 데이터 구조체.
 * UV 는 0~1 범위 (MapBoundsActor 변환 결과).
 * 위젯이 렌더링에 필요한 정보만 담는다.
 */
USTRUCT(BlueprintType)
struct FMinimapMarker
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category="Minimap")
    FVector2D UV = FVector2D::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category="Minimap")
    EMinimapMarkerType Type = EMinimapMarkerType::Neutral;
};
