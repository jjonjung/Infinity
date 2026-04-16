#pragma once

#include <cstdint>
#include <cstring>
#include <vector>

// ─────────────────────────────────────────────────────
//  [게임 서버] Serialization — 패킷 직렬화/역직렬화
//
//  설계 원칙:
//    1. #pragma pack(push, 1) 구조체는 memcpy로 직렬화 가능
//       → WriteStruct / ReadStruct 으로 처리
//    2. 동적 길이 데이터(가변 플레이어 수 등)는
//       WriteBytes / ReadBytes 로 필드 단위 처리
//    3. 모든 수치형은 Little-Endian 그대로 전송
//       (서버/클라이언트 모두 x86 기준)
//
//  사용 예:
//    // 직렬화
//    PacketWriter w;
//    w.Write<uint32_t>(serverTick);
//    w.Write<float>(pos.x);
//    w.Write<float>(pos.y);
//    session.SendPacket(OP_GAME_MOVE_BROADCAST, w.Data(), w.Size());
//
//    // 역직렬화
//    PacketReader r(body, bodySize);
//    uint32_t tick = r.Read<uint32_t>();
//    float px      = r.Read<float>();
// ─────────────────────────────────────────────────────

// ── 직렬화 ────────────────────────────────────────────
class PacketWriter
{
public:
    PacketWriter() { m_buf.reserve(256); }

    template<typename T>
    void Write(T value)
    {
        const char* src = reinterpret_cast<const char*>(&value);
        m_buf.insert(m_buf.end(), src, src + sizeof(T));
    }

    // packed 구조체 전체를 한 번에 직렬화
    template<typename T>
    void WriteStruct(const T& s)
    {
        const char* src = reinterpret_cast<const char*>(&s);
        m_buf.insert(m_buf.end(), src, src + sizeof(T));
    }

    void WriteBytes(const char* data, uint16_t size)
    {
        m_buf.insert(m_buf.end(), data, data + size);
    }

    const char* Data() const { return m_buf.data(); }
    uint16_t    Size() const { return static_cast<uint16_t>(m_buf.size()); }
    void        Clear()      { m_buf.clear(); }

private:
    std::vector<char> m_buf;
};

// ── 역직렬화 ──────────────────────────────────────────
class PacketReader
{
public:
    PacketReader(const char* data, uint16_t size)
        : m_data(data), m_size(size), m_pos(0) {}

    template<typename T>
    T Read()
    {
        T value{};
        if (m_pos + sizeof(T) <= m_size)
        {
            std::memcpy(&value, m_data + m_pos, sizeof(T));
            m_pos += static_cast<uint16_t>(sizeof(T));
        }
        return value;
    }

    // packed 구조체 전체를 한 번에 역직렬화
    template<typename T>
    bool ReadStruct(T& out)
    {
        if (m_pos + sizeof(T) > m_size) return false;
        std::memcpy(&out, m_data + m_pos, sizeof(T));
        m_pos += static_cast<uint16_t>(sizeof(T));
        return true;
    }

    bool ReadBytes(char* dest, uint16_t size)
    {
        if (m_pos + size > m_size) return false;
        std::memcpy(dest, m_data + m_pos, size);
        m_pos += size;
        return true;
    }

    bool IsValid() const   { return m_pos <= m_size; }
    bool HasMore() const   { return m_pos < m_size; }
    uint16_t Remaining() const { return m_size - m_pos; }

private:
    const char* m_data;
    uint16_t    m_size;
    uint16_t    m_pos;
};
