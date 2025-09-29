// Fill out your copyright notice in the Description page of Project Settings.

#include "DA/EnemyAIData.h"


UEnemyAIData::UEnemyAIData()
{
	// 기본값 설정
	MaxHP = 3;
	AIMoveScale = 1.0f;
	AttackRange = 200.0f;
	AttackDelayTime = 1.0f;
	IdleDelayTime = 0.5f;
	KnockbackPower = 300.0f;
	DamageDelayTime = 1.5f;
}