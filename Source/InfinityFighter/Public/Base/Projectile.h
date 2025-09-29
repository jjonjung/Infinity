// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DA/ActionLoadOutDA.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class INFINITYFIGHTER_API AProjectile : public AActor
{
    GENERATED_BODY()
    
public: 
    // Sets default values for this actor's properties
    AProjectile();
	UPROPERTY(EditDefaultsOnly)
	float InitialSpeed;
	UPROPERTY(EditDefaultsOnly)
	float MaxSpeed;
	UPROPERTY(EditDefaultsOnly)
	float LifeTime;//총알 수명
	UPROPERTY(EditDefaultsOnly)
	float Damaged;//총알 데미지

	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* TrajectoryVFX;//궤적 VFX
	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* CollisionVFX;//충돌시 VFX
	UPROPERTY(EditDefaultsOnly)
	class USoundBase* ProjectileSFX;//사운드

	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* ProjectileMeshComp;//총알 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile")
	class UProjectileMovementComponent* projectiole;
    UPROPERTY(EditDefaultsOnly)
    class UBoxComponent* BoxComp;
    
    // Damage settings (shared by all projectiles)
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
    bool bUseInstigatorAttackDamage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float BaseDamage = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
    float DamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float HeadshotMultiplier = 1.5f;

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
    // Called every frame
    virtual void Tick(float DeltaTime) override;
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Projectile")
    void ProjectileMove();
    virtual void ProjectileMove_Implementation()PURE_VIRTUAL(AProjectile::ProjectileMove);; // 이동부 구현
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Projectile")
    void OnProjectileHit(const FHitResult& Hit);
    
    class ACharacterBase* ProjectileOwner= nullptr;

    void SetProjectileOwner(ACharacterBase* MyOwner);
protected:
    UFUNCTION()
    void HandleBoxBeginOverlap(class UPrimitiveComponent* OverlappedComp,
                               AActor* OtherActor,
                               class UPrimitiveComponent* OtherComp,
                               int32 OtherBodyIndex,
                               bool bFromSweep,
                               const FHitResult& SweepResult);

    void ApplyDamageCommon(AActor* Victim, const FHitResult* HitOpt);
    

};
