// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "CharacterActionStatComponent.h"
#include "Base/CharacterBase.h"
#include "Utility/DebugUtil.h"
#include "CharacterActionStatComponent.h"
#include "Base/CharacterBase.h"
#include "Utility/DebugUtil.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	SetRootComponent(BoxComp);
	ProjectileMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMeshComp"));
	ProjectileMeshComp->SetupAttachment(BoxComp);
	
    LifeTime = 5.0f; // 기본 수명 설정
    AActor::SetLifeSpan(LifeTime);
	

}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
    Super::BeginPlay();

    if (BoxComp)
    {
        BoxComp->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::HandleBoxBeginOverlap);
    }
	
	
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::SetProjectileOwner(ACharacterBase* myOwner)
{
    if (myOwner)
    {
        ProjectileOwner = myOwner;
        SetOwner(myOwner);
        if (APawn* PawnOwner = Cast<APawn>(myOwner))
        {
            SetInstigator(PawnOwner);
        }
        if (BoxComp)
        {
            BoxComp->IgnoreActorWhenMoving(myOwner, /*bShouldIgnore*/true);
        }
    }
}

void AProjectile::OnProjectileHit_Implementation(const FHitResult& Hit)
{
    ApplyDamageCommon(Hit.GetActor(), &Hit);
}

void AProjectile::HandleBoxBeginOverlap(UPrimitiveComponent* OverlappedComp,
                                        AActor* OtherActor,
                                        UPrimitiveComponent* OtherComp,
                                        int32 OtherBodyIndex,
                                        bool bFromSweep,
                                        const FHitResult& SweepResult)
{
    if (!OtherActor)
    {
        return;
    }

    // Ignore self/owner/instigator overlaps entirely
    if (OtherActor == this || OtherActor == GetOwner() || OtherActor == GetInstigator() || OtherActor == ProjectileOwner)
    {
        return;
    }

    // Only treat overlaps with damageable actors as hits
    const bool bIsDamageable = (Cast<ACharacterBase>(OtherActor) != nullptr) || (OtherActor->FindComponentByClass<UCharacterActionStatComponent>() != nullptr);
    if (!bIsDamageable)
    {
        return;
    }

    // Prefer the sweep result if available (has bone info)
    if (bFromSweep)
    {
        OnProjectileHit(SweepResult);
        return;
    }

    // Otherwise apply with minimal info
    ApplyDamageCommon(OtherActor, nullptr);
}

void AProjectile::ApplyDamageCommon(AActor* Victim, const FHitResult* HitOpt)
{
    // Only server applies damage; do not destroy on clients
    if (!HasAuthority())
    {
        return;
    }

    // Null or hitting itself -> destroy; hitting owner/instigator -> ignore (no destroy)
    if (!Victim || Victim == this)
    {
        Destroy();
        return;
    }
    if (Victim == GetInstigator() || Victim == ProjectileOwner || Victim == GetOwner())
    {
        return; // ignore self-owner hit and keep flying
    }

    float RawDamage = 0.f;
    if (bUseInstigatorAttackDamage)
    {
        if (AActor* Attacker = GetInstigator())
        {
            if (auto* Stat = Attacker->FindComponentByClass<UCharacterActionStatComponent>())
            {
                RawDamage = Stat->GetAttackDamage();
            }
        }
        if (RawDamage <= 0.f)
        {
            RawDamage = BaseDamage;
        }
    }
    else
    {
        RawDamage = BaseDamage;
    }

    float Damage = RawDamage * DamageMultiplier;

    FName BoneName;
    if (HitOpt)
    {
        BoneName = HitOpt->BoneName;
    }
    const bool bHead = !BoneName.IsNone() && BoneName.ToString().Contains(TEXT("head"), ESearchCase::IgnoreCase);
    if (bHead)
    {
        Damage *= HeadshotMultiplier;
    }

    Debug::PrintHit(GetInstigator(), Victim, BoneName, Damage, bHead);

    if (Damage > 0.f)
    {
        if (ACharacterBase* VictimChar = Cast<ACharacterBase>(Victim))
        {
            VictimChar->ReceiveDamage(Damage, this);
        }
        else if (auto* VictimStat = Victim->FindComponentByClass<UCharacterActionStatComponent>())
        {
            VictimStat->ApplyDamage(Damage, this);
        }
    }

    Destroy();
}
