#pragma once
#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGameDebug, Log, All);

namespace Debug
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	inline void Screen
		(
			const FString& Msg, //메시지
			float Sec=1.5f, //표시시간
			const FColor& Color=FColor::White,//글자색
			int32 Key=-1 // 고정키, 새줄로 추가.
		)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(Key, Sec, Color, Msg);
	}
	inline void LogInfo(const FString& Msg)  { UE_LOG(LogGameDebug, Log,     TEXT("%s"), *Msg); }
	inline void LogWarn(const FString& Msg)  { UE_LOG(LogGameDebug, Warning, TEXT("%s"), *Msg); }
	inline void LogError(const FString& Msg) { UE_LOG(LogGameDebug, Error,   TEXT("%s"), *Msg); }



	//히트상황 전용 출력
	//  - Attacker: 공격자 액터
	//  - Victim  : 피격자 액터
	//  - Bone    : 맞은 본 이름
	//  - Damage  : 최종 데미지 값
	//  - bHead   : 헤드샷 여부(색상/표시용)
	//  - Sec     : 화면 표시 시간
	
	inline void PrintHit(AActor* Attacker, AActor* Victim, FName Bone, float Damage, bool bHead, float Sec=1.5f)
	{
		//헤드샷이면 빨강, 아니면 초록
		const FColor  Col  = bHead ? FColor::Red : FColor::Cyan;
		const FString AStr = Attacker ? Attacker->GetName() : TEXT("None");
		const FString VStr = Victim   ? Victim->GetName()   : TEXT("None");
		//맞은 본이름 출력
		const FString BStr = Bone.IsNone() ? TEXT("-") : Bone.ToString();
		//헤드샷이면 꼬리표 출력
		const FString Tag  = bHead ? TEXT(" (HEAD)") : TEXT("");

		//최종 한줄 출력
		const FString Line = FString::Printf(TEXT("[HIT] %s -> %s  Bone=%s  Dmg=%.1f%s"),
											 *AStr, *VStr, *BStr, Damage, *Tag);
		Screen(Line, Sec, Col);
	
	}
#else
	inline void Screen(const FString&, float=0, const FColor&=FColor::White, int32=-1) {}
	inline void LogInfo(const FString&) {} inline void LogWarn(const FString&) {}
	inline void LogError(const FString&) {} inline void PrintHit(AActor*,AActor*,FName,float,bool,float=0) {}
#endif
}
