// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once

#include "Core.h"
#include "CoreTypes.h"
//#include <bitset>

namespace Havtorn
{
	template<U64 bits>
    class CORE_API CBitSet
    {
    public:
        constexpr bool Subscript(U64 pos) const
        {
            return (Words[pos / BitsPerWord] & (U64(1) << pos % BitsPerWord)) != 0;
        }

        [[nodiscard]] constexpr bool operator[](U64 pos) const
        {
            return Subscript(pos);
        }

        constexpr CBitSet() noexcept : Words() {} // construct with all false values

        static constexpr bool NeedMask = bits < CHAR_BIT * sizeof(U64);

        static constexpr U64 Mask = (U64(1) << (NeedMask ? bits : 0)) - U64(1);

        constexpr CBitSet(U64 val) noexcept : Words{ static_cast<U64>(NeedMask ? val & Mask : val) } {}

        CBitSet& operator&=(const CBitSet& right) noexcept
        {
            for (U64 wPos = 0; wPos <= (NumberOfWords - U64(1)); ++wPos)
            {
                Words[wPos] &= right.Words[wPos];
            }

            return *this;
        }

        CBitSet& operator|=(const CBitSet& right) noexcept
        {
            for (U64 wPos = 0; wPos <= (NumberOfWords - U64(1)); ++wPos)
            {
                Words[wPos] |= right.Words[wPos];
            }

            return *this;
        }

        CBitSet& operator^=(const CBitSet& right) noexcept
        {
            for (U64 wPos = 0; wPos <= (NumberOfWords - U64(1)); ++wPos)
            {
                Words[wPos] ^= right.Words[wPos];
            }

            return *this;
        }

        // Shift left by pos, first by words then by bits
        CBitSet& operator<<=(U64 pos) noexcept
        {
            const auto wordShift = static_cast<I64>(pos / BitsPerWord);

            if (wordShift != 0)
            {
                for (I64 wPos = (NumberOfWords - U64(1)); 0 <= wPos; --wPos)
                {
                    Words[wPos] = wordShift <= wPos ? Words[wPos - wordShift] : 0;
                }
            }

            // 0 < pos < BitsPerWord, shift by bits
            if ((pos %= BitsPerWord) != 0)
            {
                for (I64 wPos = (NumberOfWords - U64(1)); 0 < wPos; --wPos)
                {
                    Words[wPos] = (Words[wPos] << pos) | (Words[wPos - 1] >> (BitsPerWord - pos));
                }

                Words[0] <<= pos;
            }

            Trim();
            return *this;
        }

        // Shift right by pos, first by words then by bits
        CBitSet& operator>>=(U64 pos) noexcept
        {
            const auto wordShift = static_cast<I64>(pos / BitsPerWord);

            if (wordShift != 0)
            {
                for (I64 wPos = 0; wPos <= (NumberOfWords - U64(1)); ++wPos)
                {
                    Words[wPos] = wordShift <= (NumberOfWords - U64(1)) - wPos ? Words[wPos + wordShift] : 0;
                }
            }

            // 0 < pos < BitsPerWord, shift by bits
            if ((pos %= BitsPerWord) != 0)
            {
                for (I64 wPos = 0; wPos < (NumberOfWords - U64(1)); ++wPos)
                {
                    Words[wPos] = (Words[wPos] >> pos) | (Words[wPos + 1] << (BitsPerWord - pos));
                }

                Words[(NumberOfWords - U64(1))] >>= pos;
            }

            return *this;
        }

        // Set all bits true
        CBitSet& Set() noexcept
        {
            ::memset(&Words, 0xFF, sizeof(Words));
            Trim();
            return *this;
        }

        // Set bit at pos to val
        CBitSet& Set(U64 pos, bool val = true)
        {
            if (bits <= pos)
            {
                HV_LOG_ERROR("CBitSet: Tried to set nonexisting bit of BitSet!");
            }

            return SetUnchecked(pos, val);
        }

        // Set all bits false
        CBitSet& Reset() noexcept
        {
            ::memset(&Words, 0, sizeof(Words));
            return *this;
        }

        // Set bit at pos to false
        CBitSet& Reset(U64 pos)
        {
            return Set(pos, false);
        }

        // Flip all bits
        [[nodiscard]] CBitSet operator~() const noexcept
        {
            return CBitSet(*this).Flip();
        }

        // Flip all bits
        CBitSet& Flip() noexcept
        {
            for (U64 wPos = 0; wPos <= (NumberOfWords - U64(1)); ++wPos)
            {
                Words[wPos] = ~Words[wPos];
            }

            Trim();
            return *this;
        }

        // Flip bit at pos
        CBitSet& Flip(U64 pos)
        {
            if (bits <= pos)
            {
                HV_LOG_ERROR("CBitSet: Tried to flip nonexisting bit of BitSet!");
            }

            return FlipUnchecked(pos);
        }

        [[nodiscard]] U64 ToU64() const
        {
            constexpr bool isSizeZero = bits == 0;
            constexpr bool isSizeLarge = bits > 64;

            if constexpr (isSizeZero)
            {
                return 0;
            }
            else
            {
                if constexpr (isSizeLarge)
                {
                    for (U64 wPos = 1; wPos <= (NumberOfWords - U64(1)); ++wPos)
                    {
                        // Fail if any high-order words are nonzero
                        if (Words[wPos] != 0)
                        {
                            HV_LOG_ERROR("CBitSet: BitSet was too large to fit within a U64!");
                        }
                    }
                }

                return Words[0];
            }
        }

        [[nodiscard]] std::string ToString() const 
        {
            // convert bitset to string
            std::string str;
            str.reserve(bits);

            for (auto pos = bits; 0 < pos;) 
            {
                str.push_back(Subscript(--pos) ? '1' : '0');
            }

            return str;
        }

        // Count number of set bits
        [[nodiscard]] U64 Count() const noexcept
        {
            const char* const bitsPerByte =
                "\0\1\1\2\1\2\2\3\1\2\2\3\2\3\3\4"
                "\1\2\2\3\2\3\3\4\2\3\3\4\3\4\4\5"
                "\1\2\2\3\2\3\3\4\2\3\3\4\3\4\4\5"
                "\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
                "\1\2\2\3\2\3\3\4\2\3\3\4\3\4\4\5"
                "\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
                "\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
                "\3\4\4\5\4\5\5\6\4\5\5\6\5\6\6\7"
                "\1\2\2\3\2\3\3\4\2\3\3\4\3\4\4\5"
                "\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
                "\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
                "\3\4\4\5\4\5\5\6\4\5\5\6\5\6\6\7"
                "\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
                "\3\4\4\5\4\5\5\6\4\5\5\6\5\6\6\7"
                "\3\4\4\5\4\5\5\6\4\5\5\6\5\6\6\7"
                "\4\5\5\6\5\6\6\7\5\6\6\7\6\7\7\x8";

            const unsigned char* ptr = &reinterpret_cast<const unsigned char&>(Words);
            const unsigned char* const end = ptr + sizeof(Words);

            U64 val = 0;
            for (; ptr != end; ++ptr)
            {
                val += bitsPerByte[*ptr];
            }

            return val;
        }

        [[nodiscard]] constexpr U64 Size() const noexcept
        {
            return bits;
        }

        [[nodiscard]] bool operator==(const CBitSet& right) const noexcept
        {
            return ::memcmp(&Words[0], &right.Words[0], sizeof(Words)) == 0;
        }

        [[nodiscard]] bool operator!=(const CBitSet& right) const noexcept
        {
            return !(*this == right);
        }

        [[nodiscard]] bool Test(U64 pos) const
        {
            if (bits <= pos)
            {
                HV_LOG_ERROR("CBitSet: Tried to test nonexisting bit in BitSet!");
            }

            return Subscript(pos);
        }

        [[nodiscard]] bool Any() const noexcept
        {
            for (U64 wPos = 0; wPos <= (NumberOfWords - U64(1)); ++wPos)
            {
                if (Words[wPos] != 0)
                {
                    return true;
                }
            }

            return false;
        }

        [[nodiscard]] bool None() const noexcept 
        {
            return !Any();
        }

        [[nodiscard]] bool All() const noexcept 
        {
            constexpr bool isZeroLength = bits == 0;
            
            // Must test for this, otherwise would count one full word
            if constexpr (isZeroLength) 
            { 
                return true;
            }

            constexpr bool hasNoPadding = bits % BitsPerWord == 0;
            for (U64 wPos = 0; wPos < (NumberOfWords - U64(1)) + U64(hasNoPadding); ++wPos)
            {
                if (Words[wPos] != ~U64(0)) 
                {
                    return false;
                }
            }

            return hasNoPadding || Words[(NumberOfWords - U64(1))] == (U64(1) << (bits % BitsPerWord)) - U64(1);
        }

        [[nodiscard]] CBitSet operator<<(U64 pos) const noexcept 
        {
            return CBitSet(*this) <<= pos;
        }

        [[nodiscard]] CBitSet operator>>(U64 pos) const noexcept 
        {
            return CBitSet(*this) >>= pos;
        }

	private:
        // Clear any trailing bits in last word
        void Trim() noexcept 
        { 
            constexpr bool hasWorkToDo = bits == 0 || bits % BitsPerWord != 0;
            if constexpr (hasWorkToDo) 
            {
                Words[(NumberOfWords - U64(1))] &= (U64(1) << bits % BitsPerWord) - 1;
            }
        }

        // Set bit at pos to val, no checking
        CBitSet& SetUnchecked(U64 pos, bool val) noexcept 
        { 
            auto& selectedWord = Words[pos / BitsPerWord];
            const auto bit = U64(1) << pos % BitsPerWord;

            if (val) 
            {
                selectedWord |= bit;
            }
            else 
            {
                selectedWord &= ~bit;
            }

            return *this;
        }

        // Flip bit at pos, no checking
        CBitSet& FlipUnchecked(U64 pos) noexcept 
        { 
            Words[pos / BitsPerWord] ^= U64(1) << pos % BitsPerWord;
            return *this;
        }


    private:
		static constexpr I64 BitsPerWord = CHAR_BIT * sizeof(U64);
		static constexpr I64 NumberOfWords = bits == 0 ? 0 : ((bits - 1) / BitsPerWord) + 1;
		U64 Words[NumberOfWords];
	};
}