#include "UI/MapBoundsActor.h"

AMapBoundsActor::AMapBoundsActor()
{
    PrimaryActorTick.bCanEverTick = false;

    BoundsBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BoundsBox"));
    BoundsBox->SetBoxExtent(FVector(5000.f, 5000.f, 500.f));
    BoundsBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RootComponent = BoundsBox;
}

FBox2D AMapBoundsActor::GetXYBounds() const
{
    const FVector Origin = BoundsBox->GetComponentLocation();
    const FVector Extent = BoundsBox->GetScaledBoxExtent();

    const FVector2D Min(Origin.X - Extent.X, Origin.Y - Extent.Y);
    const FVector2D Max(Origin.X + Extent.X, Origin.Y + Extent.Y);
    return FBox2D(Min, Max);
}

FVector2D AMapBoundsActor::UVToWorldXY(const FVector2D& UV) const
{
    const FBox2D  Bounds    = GetXYBounds();
    const FVector2D ClampedUV(
        FMath::Clamp(UV.X, 0.f, 1.f),
        FMath::Clamp(UV.Y, 0.f, 1.f)
    );

    return FVector2D(
        FMath::Lerp(Bounds.Min.X, Bounds.Max.X, ClampedUV.X),
        FMath::Lerp(Bounds.Min.Y, Bounds.Max.Y, ClampedUV.Y)
    );
}

FVector2D AMapBoundsActor::WorldXYToUV(const FVector2D& WorldXY) const
{
    const FBox2D Bounds = GetXYBounds();
    return FVector2D(
        FMath::GetMappedRangeValueClamped(
            FVector2D(Bounds.Min.X, Bounds.Max.X), FVector2D(0.f, 1.f), WorldXY.X),
        FMath::GetMappedRangeValueClamped(
            FVector2D(Bounds.Min.Y, Bounds.Max.Y), FVector2D(0.f, 1.f), WorldXY.Y)
    );
}
