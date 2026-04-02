#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Character.h"
#include "Skill1_SpiderManSwing.generated.h"

class USkeletalMeshComponent;

UCLASS(Blueprintable, BlueprintType)
class INFINITYFIGHTER_API USkill1_SpiderManSwing : public UActionBase
{
	GENERATED_BODY()

public:
	USkill1_SpiderManSwing();

	virtual bool CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const override;
	virtual void Activate_Implementation(ACharacterBase* Owner) override;
	virtual void End_Implementation(ACharacterBase* Owner) override;
	virtual void Cancel_Implementation(ACharacterBase* Owner) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Settings")
	float MaxSwingDistance = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Settings")
	float SwingForce = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Settings")
	float ReleaseVelocityMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Settings")
	float SwingDamping = 0.98f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Settings")
	float MinHeightForSwing = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Attack")
	float SwingDamage = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Attack")
	float AttackRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Attack")
	float KnockbackForce = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Attack")
	float MinAttackVelocity = 500.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Swing|State")
	bool bIsSwinging = false;

	UPROPERTY(BlueprintReadOnly, Category = "Swing|State")
	FVector SwingAnchorPoint;

	UPROPERTY(BlueprintReadOnly, Category = "Swing|State")
	float CurrentRopeLength = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Swing|State")
	TWeakObjectPtr<ACharacterBase> TargetEnemy;

	UPROPERTY(BlueprintReadOnly, Category = "Swing|State")
	bool bIsWallCrawling = false;

	FVector SwingVelocity;
	float SwingAngle = 0.0f;
	float SwingAngularVelocity = 0.0f;

	UPROPERTY()
	UStaticMeshComponent* WebLineComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Visuals")
	int32 MeshSegmentCount = 10;

	FTimerHandle SwingUpdateHandle;

	// Faster than TArray::Contains for repeated attack deduping during swing.
	TSet<TWeakObjectPtr<AActor>> HitActors;

private:
	UPROPERTY()
	USkeletalMesh* WebLineMesh = nullptr;

	UPROPERTY()
	TArray<USkeletalMeshComponent*> WebLineMeshComponents;

	// Reused collision buffer to avoid reallocating every swing tick.
	TArray<FOverlapResult> CachedOverlapResults;

private:
	bool FindSwingTarget(ACharacterBase* Owner, FVector& OutAnchorPoint);
	bool FindEnemyTarget(ACharacterBase* Owner, FVector& OutAnchorPoint);
	void StartSwinging(ACharacterBase* Owner);
	void UpdateSwing(ACharacterBase* Owner);
	void EndSwinging(ACharacterBase* Owner);
	void CreateWebLine(ACharacterBase* Owner);
	void UpdateWebLineVisual(ACharacterBase* Owner);
	void DestroyWebLine();

	void CreateWebLineMeshes(ACharacterBase* Owner);
	void UpdateWebLineMeshes(ACharacterBase* Owner);
	void DestroyWebLineMeshes();

	void StartWallCrawling(ACharacterBase* Owner);
	void UpdateWallCrawling(ACharacterBase* Owner);
	void StartEnemyPull(ACharacterBase* Owner);
	void UpdateEnemyPull(ACharacterBase* Owner);

	void CalculateSwingPhysics(ACharacterBase* Owner, float DeltaTime);
	FVector GetSwingDirection(const FVector& PlayerPos, const FVector& AnchorPos) const;

	void CheckSwingAttack(ACharacterBase* Owner);
	void PerformSwingAttack(ACharacterBase* Owner, ACharacterBase* Target);
	bool CanAttackTarget(ACharacterBase* Owner, ACharacterBase* Target) const;
};
