//
// Created by ssj5v on 08-03-2026.
//

#ifndef GUID_H
#define GUID_H

#include <cstdint>
#include <random>
#include <sstream>
#include <iomanip>
#include <string>
#include <functional>

struct Guid
{
    uint64_t high = 0;
    uint64_t low  = 0;

    Guid() = default;

    Guid(uint64_t h, uint64_t l)
        : high(h), low(l) {}

    static Guid New()
    {
        static std::random_device rd;
        static std::mt19937_64 eng(rd());
        static std::uniform_int_distribution<uint64_t> dist;

        return Guid(dist(eng), dist(eng));
    }

    bool IsValid() const
    {
        return high != 0 || low != 0;
    }

    bool operator==(const Guid& other) const
    {
        return high == other.high && low == other.low;
    }

    bool operator!=(const Guid& other) const
    {
        return !(*this == other);
    }

    bool operator<(const Guid& other) const
    {
        if (high == other.high)
            return low < other.low;

        return high < other.high;
    }

    std::string ToString() const
    {
        std::stringstream ss;

        ss << std::hex << std::setfill('0')
           << std::setw(16) << high
           << std::setw(16) << low;

        return ss.str();
    }

    static Guid FromString(const std::string& str)
    {
        if (str.size() != 32)
            return Guid();

        Guid id;

        std::stringstream ss;
        ss << std::hex << str.substr(0,16) << " " << str.substr(16,16);
        ss >> id.high >> id.low;

        return id;
    }
};

namespace std
{
    template<>
    struct hash<Guid>
    {
        size_t operator()(const Guid& g) const noexcept
        {
            return hash<uint64_t>()(g.high) ^ (hash<uint64_t>()(g.low) << 1);
        }
    };
}
#endif //GUID_H
