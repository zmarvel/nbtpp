/*
 * Copyright (C) 2019  Zack Marvel
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "catch2/catch.hpp"

#include "nbt.hpp"


TEST_CASE("Primitive type byte-swaps", "[primitive]") {
  SECTION("File-to-host") {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    REQUIRE(ByteTag::ftoh(0x7e) == 0x7e);
    REQUIRE(ShortTag::ftoh(0x7e) == 0x7e00);
    REQUIRE(IntTag::ftoh(0x7e) == 0x7e000000);
    REQUIRE(LongTag::ftoh(0x7e6d5c4b) == 0x4b5c6d7e00000000);

    {
      union expected_u {
        unsigned char raw[4];
        uint32_t uint;
        float flt;
      } expected;
      expected.raw[0] = 0x42;
      expected.raw[1] = 0x80;
      expected.raw[2] = 0x00;
      expected.raw[3] = 0x00;

      REQUIRE(FloatTag::ftoh(64.0f) == expected.flt);
    }
    {
      union expected_u {
        unsigned char raw[8];
        uint64_t uint;
        double flt;
      } expected;
      expected.raw[0] = 0x40;
      expected.raw[1] = 0x50;
      expected.raw[2] = 0x00;
      expected.raw[3] = 0x00;
      expected.raw[4] = 0x00;
      expected.raw[5] = 0x00;
      expected.raw[6] = 0x00;
      expected.raw[7] = 0x00;

      REQUIRE(DoubleTag::ftoh(64.0) == expected.flt);
    }

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // These should be no-ops
    REQUIRE(ByteTag::ftoh(0x7e) == 0x7e);
    REQUIRE(ShortTag::ftoh(0x7e) == 0x7e);
    REQUIRE(IntTag::ftoh(0x7e) == 0x7e);
    REQUIRE(LongTag::ftoh(0x7e6d5c4b) == 0x7e6d5c4b);

    REQUIRE(FloatTag::ftoh(64.0f) == 64.0f);
    REQUIRE(DoubleTag::ftoh(64.0) == 64.0);
#endif
  }
  SECTION("Host-to-file") {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    REQUIRE(ByteTag::htof(0x7e) == 0x7e);
    REQUIRE(ShortTag::htof(0x7e00) == 0x7e);
    REQUIRE(IntTag::htof(0x7e000000) == 0x7e);
    REQUIRE(LongTag::htof(0x4b5c6d7e00000000) == 0x7e6d5c4b);

    { // ['0x0', '0x24', '0x74', '0x49']
      union expected_u {
        unsigned char raw[4];
        uint32_t uint;
        float flt;
      } expected;
      expected.raw[0] = 0x49;
      expected.raw[1] = 0x74;
      expected.raw[2] = 0x24;
      expected.raw[3] = 0x00;

      REQUIRE(FloatTag::htof(1e6f) == expected.flt);
    }
    { // ['0x0', '0x0', '0x0', '0x0', '0x65', '0xcd', '0xcd', '0x41']
      union expected_u {
        unsigned char raw[8];
        uint64_t uint;
        double flt;
      } expected;
      expected.raw[0] = 0x41;
      expected.raw[1] = 0xcd;
      expected.raw[2] = 0xcd;
      expected.raw[3] = 0x65;
      expected.raw[4] = 0x00;
      expected.raw[5] = 0x00;
      expected.raw[6] = 0x00;
      expected.raw[7] = 0x00;

      REQUIRE(DoubleTag::htof(1e9) == expected.flt);
    }
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // These should be no-ops
    REQUIRE(ByteTag::htof(0x7e) == 0x7e);
    REQUIRE(ShortTag::htof(0x7e) == 0x7e);
    REQUIRE(IntTag::htof(0x7e) == 0x7e);
    REQUIRE(LongTag::htof(0x7e6d5c4b) == 0x7e6d5c4b);

    REQUIRE(FloatTag::htof(1e6f) == 1e6f);
    REQUIRE(DoubleTag::htof(1e9) == 1e9);
#endif
  }
}


TEST_CASE("Array type byte-swaps", "[array]") {
  SECTION("File-to-host") {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    {
      std::vector<int8_t> vec = {
        0x12, 0x13, 0x14, 0x15
      };
      std::vector<int8_t> expVec{vec};
      std::vector<int8_t> outVec = ByteArrayTag::ftoh(vec);
      REQUIRE(outVec == expVec);
    }
    {
      std::vector<int32_t> vec = {
        0x12, 0x13, 0x14, 0x15
      };
      std::vector<int32_t> expVec = {
        0x12000000, 0x13000000, 0x14000000, 0x15000000
      };
      std::vector<int32_t> outVec = IntArrayTag::ftoh(vec);
      REQUIRE(outVec == expVec);
    }
    {
      std::vector<int64_t> vec = {
        0x12, 0x13, 0x14, 0x15
      };
      std::vector<int64_t> expVec = {
        0x1200000000000000, 0x1300000000000000,
        0x1400000000000000, 0x1500000000000000
      };
      std::vector<int64_t> outVec = LongArrayTag::ftoh(vec);
      REQUIRE(outVec == expVec);
    }
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    {
      std::vector<int8_t> vec = {
        0x12, 0x13, 0x14, 0x15
      };
      std::vector<int8_t> outVec = ByteArrayTag::ftoh(vec);
      REQUIRE(outVec == vec);
    }
    {
      std::vector<int32_t> vec = {
        0x12, 0x13, 0x14, 0x15
      };
      std::vector<int32_t> outVec = IntArrayTag::ftoh(vec);
      REQUIRE(outVec == vec);
    }
    {
      std::vector<int64_t> vec = {
        0x12, 0x13, 0x14, 0x15
      };
      std::vector<int64_t> outVec = LongArrayTag::ftoh(vec);
      REQUIRE(outVec == vec);
    }
#endif
  }

  SECTION("Host-to-file") {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    {
      std::vector<int8_t> vec = {
        0x12, 0x13, 0x14, 0x15
      };
      std::vector<int8_t> expVec{vec};
      std::vector<int8_t> outVec = ByteArrayTag::htof(vec);
      REQUIRE(outVec == expVec);
    }
    {
      std::vector<int32_t> vec = {
        0x12000000, 0x13000000, 0x14000000, 0x15000000
      };
      std::vector<int32_t> expVec = {
        0x12, 0x13, 0x14, 0x15
      };
      std::vector<int32_t> outVec = IntArrayTag::htof(vec);
      REQUIRE(outVec == expVec);
    }
    {
      std::vector<int64_t> vec = {
        0x1200000000000000, 0x1300000000000000,
        0x1400000000000000, 0x1500000000000000
      };
      std::vector<int64_t> expVec = {
        0x12, 0x13, 0x14, 0x15
      };
      std::vector<int64_t> outVec = LongArrayTag::htof(vec);
      REQUIRE(outVec == expVec);
    }
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    {
      std::vector<int8_t> vec = {
        0x12, 0x13, 0x14, 0x15
      };
      std::vector<int8_t> outVec = ByteArrayTag::htof(vec);
      REQUIRE(outVec == vec);
    }
    {
      std::vector<int32_t> vec = {
        0x12000000, 0x13000000, 0x14000000, 0x15000000
      };
      std::vector<int32_t> outVec = IntArrayTag::htof(vec);
      REQUIRE(outVec == vec);
    }
    {
      std::vector<int64_t> vec = {
        0x1200000000000000, 0x1300000000000000,
        0x1400000000000000, 0x1500000000000000
      };
      std::vector<int64_t> outVec = LongArrayTag::htof(vec);
      REQUIRE(outVec == vec);
    }
#endif
  }
}
