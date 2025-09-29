// TDequeUE.h
#pragma once
#include "CoreMinimal.h"

/**
 * Unreal-style lightweight deque (double-ended queue)
 * - push_back / push_front
 * - pop_back / pop_front  (bool 반환)
 * - operator[] 임의 접근 (0 = 앞(front))
 * - Front()/Back(), Num(), IsEmpty(), Clear()
 * 내부는 TArray 를 링버퍼처럼 사용.
 */
template<typename T>
class TDeque
{
public:
    TDeque(int32 InCapacity = 8)
    {
        Capacity = FMath::Max(8, InCapacity);
        Data.SetNum(Capacity);   // default construct
        Head = 0;
        Count = 0;
    }

    // 요소 개수
    FORCEINLINE int32 Num() const { return Count; }
    FORCEINLINE bool  IsEmpty() const { return Count == 0; }
    FORCEINLINE int32 CapacitySize() const { return Capacity; }

    // 임의 접근 (0 = front, Num-1 = back)
    FORCEINLINE T& operator[](int32 Index)
    {
        checkf(Index >= 0 && Index < Count, TEXT("TDequeUE::operator[] index out of range"));
        return Data[PhysIndex(Index)];
    }
    FORCEINLINE const T& operator[](int32 Index) const
    {
        checkf(Index >= 0 && Index < Count, TEXT("TDequeUE::operator[] index out of range"));
        return Data[PhysIndex(Index)];
    }

    // 맨 앞/뒤 참조
    FORCEINLINE T& Front()             { check(!IsEmpty()); return (*this)[0]; }
    FORCEINLINE const T& Front() const { check(!IsEmpty()); return (*this)[0]; }
    FORCEINLINE T& Back()              { check(!IsEmpty()); return (*this)[Count-1]; }
    FORCEINLINE const T& Back() const  { check(!IsEmpty()); return (*this)[Count-1]; }

    // 뒤에 삽입 (push back)
    void PushBack(const T& Value)
    {
        EnsureCapacity(Count + 1);
        const int32 Tail = PhysIndex(Count); // write at back
        Data[Tail] = Value;
        ++Count;
    }
    void PushBack(T&& Value)
    {
        EnsureCapacity(Count + 1);
        const int32 Tail = PhysIndex(Count);
        Data[Tail] = MoveTemp(Value);
        ++Count;
    }

    // 앞에 삽입 (push front)
    void PushFront(const T& Value)
    {
        EnsureCapacity(Count + 1);
        Head = PrevIndex(Head);
        Data[Head] = Value;
        ++Count;
    }
    void PushFront(T&& Value)
    {
        EnsureCapacity(Count + 1);
        Head = PrevIndex(Head);
        Data[Head] = MoveTemp(Value);
        ++Count;
    }

    // 앞에서 제거 (pop front) — Out에 담고 true, 비어있으면 false
    bool PopFront(T& OutItem)
    {
        if (IsEmpty()) return false;
        OutItem = MoveTemp(Data[Head]);
        Head = NextIndex(Head);
        --Count;
        return true;
    }

    // 뒤에서 제거 (pop back)
    bool PopBack(T& OutItem)
    {
        if (IsEmpty()) return false;
        const int32 Tail = PhysIndex(Count - 1);
        OutItem = MoveTemp(Data[Tail]);
        --Count;
        return true;
    }

    // 싹 비우기 (메모리는 유지)
    void Clear()
    {
        Head = 0;
        Count = 0;
    }

    // 최신(뒤) → 과거(앞) 순회 콜백
    template<typename FuncType>
    void ForEachFromBack(FuncType&& Func) const
    {
        for (int32 i = Count - 1; i >= 0; --i)
        {
            Func((*this)[i]);
        }
    }

private:
    TArray<T> Data;
    int32 Capacity = 0;
    int32 Head = 0;     // 논리적 0번(Front)이 놓이는 물리 인덱스
    int32 Count = 0;

    FORCEINLINE int32 NextIndex(int32 I) const { return (I + 1) >= Capacity ? 0 : (I + 1); }
    FORCEINLINE int32 PrevIndex(int32 I) const { return (I - 1) < 0 ? (Capacity - 1) : (I - 1); }
    FORCEINLINE int32 PhysIndex(int32 LogicalIndex) const
    {
        // LogicalIndex: 0..Count-1
        const int32 I = Head + LogicalIndex;
        return (I >= Capacity) ? (I - Capacity) : I;
    }

    void EnsureCapacity(int32 Needed)
    {
        if (Needed <= Capacity) return;

        int32 NewCap = FMath::Max(8, Capacity * 2);
        while (NewCap < Needed) NewCap *= 2;

        TArray<T> NewData;
        NewData.SetNum(NewCap);

        // 0..Count-1 순서로 재배치
        for (int32 i = 0; i < Count; ++i)
        {
            NewData[i] = MoveTemp((*this)[i]);
        }
        Data = MoveTemp(NewData);
        Capacity = NewCap;
        Head = 0;
    }
};
