#include "UI/MinimapDataActor.h"
#include "UI/MapBoundsActor.h"
#include "Base/CharacterBase.h"
#include "Utility/BattleTypes.h"
#include "EngineUtils.h"

AMinimapDataActor::AMinimapDataActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

// ============================================================
// GetMapBounds — 첫 호출 시 레벨에서 탐색 후 캐시
// ============================================================
AMapBoundsActor* AMinimapDataActor::GetMapBounds() const
{
    if (!CachedBounds.IsValid())
    {
        if (UWorld* World = GetWorld())
        {
            for (TActorIterator<AMapBoundsActor> It(World); It; ++It)
            {
                CachedBounds = *It;
                break;
            }
        }
    }
    return CachedBounds.Get();
}

// ============================================================
// GetMinimapMarkers — 게임 로직이 집중되는 유일한 지점
//   ACharacterBase / ETeam 은 이 파일만 알고 있다.
// ============================================================
TArray<FMinimapMarker> AMinimapDataActor::GetMinimapMarkers(const APawn* ObserverPawn) const
{
    TArray<FMinimapMarker> Markers;

    AMapBoundsActor* Bounds = GetMapBounds();
    if (!Bounds) return Markers;

    UWorld* World = GetWorld();
    if (!World) return Markers;

    const ACharacterBase* Observer    = Cast<ACharacterBase>(ObserverPawn);
    const ETeam           ObserverTeam = Observer ? Observer->MatchTeam : ETeam::None;

    for (TActorIterator<ACharacterBase> It(World); It; ++It)
    {
        const ACharacterBase* Char = *It;
        if (!Char || Char->IsActorBeingDestroyed()) continue;

        FMinimapMarker Marker;
        Marker.UV = Bounds->WorldXYToUV(FVector2D(Char->GetActorLocation().X,
                                                   Char->GetActorLocation().Y));

        if (Char == Observer)
        {
            Marker.Type = EMinimapMarkerType::Self;
        }
        else if (ObserverTeam != ETeam::None && Char->MatchTeam == ObserverTeam)
        {
            Marker.Type = EMinimapMarkerType::Friendly;
        }
        else if (Char->MatchTeam != ETeam::None)
        {
            Marker.Type = EMinimapMarkerType::Enemy;
        }
        else
        {
            Marker.Type = EMinimapMarkerType::Neutral;
        }

        Markers.Add(Marker);
    }

    return Markers;
}
