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


TEST_CASE("Reading from test files", "[nbtfile]") {
  SECTION("Single, primitive-tag files") {
    {
      NBTFile file{"./test/data/end_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::END);
      file.readTag<EndTag>();
    }
    {
      NBTFile file{"./test/data/byte_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::BYTE);
      ByteTag tag = file.readTag<ByteTag>();
      REQUIRE(tag.name == "byte tag");
      REQUIRE(tag.value == 0x40);
    }
    {
      NBTFile file{"./test/data/short_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::SHORT);
      ShortTag tag = file.readTag<ShortTag>();
      REQUIRE(tag.name == "short tag");
      REQUIRE(tag.value == 0x40);
    }
    {
      NBTFile file{"./test/data/int_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::INT);
      IntTag tag = file.readTag<IntTag>();
      REQUIRE(tag.name == "int tag");
      REQUIRE(tag.value == 0x40000000);
    }
    {
      NBTFile file{"./test/data/long_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::LONG);
      LongTag tag = file.readTag<LongTag>();
      REQUIRE(tag.name == "long tag");
      REQUIRE(tag.value == 0x4000000030000000);
    }
    {
      NBTFile file{"./test/data/float_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::FLOAT);
      FloatTag tag = file.readTag<FloatTag>();
      REQUIRE(tag.name == "float tag");
      REQUIRE(tag.value == 64.0f);
    }
    {
      NBTFile file{"./test/data/double_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::DOUBLE);
      DoubleTag tag = file.readTag<DoubleTag>();
      REQUIRE(tag.name == "double tag");
      REQUIRE(tag.value == 64.0);
    }
  }

  SECTION("Files with complex tags") {
    { // ByteArrayTag
      NBTFile file{"./test/data/byte_array_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::BYTE_ARRAY);
      ByteArrayTag tag = file.readTag<ByteArrayTag>();
      REQUIRE(tag.name == "byte array tag");
      std::vector<int8_t> expected{
        0x12, 0x23, 0x34, 0x45
      };
      REQUIRE(*tag.value == expected);
    }
    { // IntArrayTag
      NBTFile file{"./test/data/int_array_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::INT_ARRAY);
      IntArrayTag tag = file.readTag<IntArrayTag>();
      REQUIRE(tag.name == "int array tag");
      std::vector<int32_t> expected{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        0x44332211, 0x55443322, 0x66554433, 0x77665544
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        0x11223344, 0x22334455, 0x33445566, 0x44556677
#endif
      };
      REQUIRE(*tag.value == expected);
    }
    { // LongArrayTag
      NBTFile file{"./test/data/long_array_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::LONG_ARRAY);
      LongArrayTag tag = file.readTag<LongArrayTag>();
      REQUIRE(tag.name == "long array tag");
      std::vector<int64_t> expected{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        0x0877665544332211, 0x0988776655443322,
        0x0a99887766554433, 0x0baa998877665544
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        0x1122334455667708, 0x2233445566778809,
        0x334455667788990a, 0x445566778899aa0b
#endif
      };
      REQUIRE(*tag.value == expected);
    }
    { // StringTag
      NBTFile file{"./test/data/string_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::STRING);
      StringTag tag = file.readTag<StringTag>();
      REQUIRE(tag.name == "string tag");
      std::string expected{"The quick brown fox jumped over the lazy dog"};
      REQUIRE(*tag.value == expected);
    }
    { // ListTag<ByteTag>
      NBTFile file{"./test/data/list_byte_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::LIST);
      ListTag<ByteTag> tag = file.readTagList<ByteTag>();
      REQUIRE(tag.name == "listof byte tag");
      std::vector<int8_t> expected{
        0x7f, 0x6e, 0x5d, 0x4c
      };
      REQUIRE(*tag.value == expected);
    }
    { // ListTag<StringTag>
      NBTFile file{"./test/data/list_string_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::LIST);
      ListTag<StringTag> tag = file.readTagList<StringTag>();
      REQUIRE(tag.name == "list tag");
      std::vector<std::string> expected{
        "Roses are red",
        "Violets are blue",
        "C++ is a language for me and you"
      };
      for (size_t i = 0; i < expected.size(); i++) {
        REQUIRE(*(tag.value->at(i)) == expected.at(i));
      }
    }
    { // CompoundTag
      /*
       * CompoundTag
       * |
       * |-- StringTag
       * |-- LongTag
       * |-- IntArrayTag
       * |-- ListTag<DoubleTag> (2)
       */
      NBTFile file{"./test/data/compound_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::COMPOUND);
      CompoundTag tag = file.readCompoundTag("");
      REQUIRE(tag.idAt(0) == TagID::STRING);
      StringTag child = tag.at<StringTag>(0);
      REQUIRE(child.name  == "string child");
      std::cout << *child.value << std::endl;
      REQUIRE(*child.value == "Hello world");
    }
    if (false) {
      /*
       * ListTag
       * |
       * |-- CompoundTag
       * |   |-- StringTag
       * |   |-- LongArrayTag
       * |
       * |-- CompoundTag
       * |   |-- IntTag
       * |   |-- ShortTag
       * |   |-- ShortTag
       *
       */
      NBTFile file{"./test/data/list_compound_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::LIST);
      ListTag<CompoundTag> tag = file.readTagList<CompoundTag>();
    }
  }
    // TODO
    // Test bad paths:
    // - File doesn't exist
    // - File ends unexpectedly (e.g. list with length 2 and 0 real members)
    // - CompoundTag with no EndTag (falls into "file ends unexpectedly")
}
