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
      REQUIRE(tag.name() == "byte tag");
      REQUIRE(tag.value() == 0x40);
    }
    {
      NBTFile file{"./test/data/short_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::SHORT);
      ShortTag tag = file.readTag<ShortTag>();
      REQUIRE(tag.name() == "short tag");
      REQUIRE(tag.value() == 0x40);
    }
    {
      NBTFile file{"./test/data/int_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::INT);
      IntTag tag = file.readTag<IntTag>();
      REQUIRE(tag.name() == "int tag");
      REQUIRE(tag.value() == 0x40000000);
    }
    {
      NBTFile file{"./test/data/long_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::LONG);
      LongTag tag = file.readTag<LongTag>();
      REQUIRE(tag.name() == "long tag");
      REQUIRE(tag.value() == 0x4000000030000000);
    }
    {
      NBTFile file{"./test/data/float_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::FLOAT);
      FloatTag tag = file.readTag<FloatTag>();
      REQUIRE(tag.name() == "float tag");
      REQUIRE(tag.value() == 64.0f);
    }
    {
      NBTFile file{"./test/data/double_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::DOUBLE);
      DoubleTag tag = file.readTag<DoubleTag>();
      REQUIRE(tag.name() == "double tag");
      REQUIRE(tag.value() == 64.0);
    }
  }

  SECTION("Files with complex tags") {
    SECTION("ByteArrayTag") {
      NBTFile file{"./test/data/byte_array_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::BYTE_ARRAY);
      ByteArrayTag tag = file.readTag<ByteArrayTag>();
      REQUIRE(tag.name() == "byte array tag");
      std::vector<int8_t> expected{
        0x12, 0x23, 0x34, 0x45
      };
      REQUIRE(tag.size() == expected.size());
      REQUIRE(tag.value() == expected);
    }
    SECTION("IntArrayTag") {
      NBTFile file{"./test/data/int_array_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::INT_ARRAY);
      IntArrayTag tag = file.readTag<IntArrayTag>();
      REQUIRE(tag.name() == "int array tag");
      std::vector<int32_t> expected{
        0x11223344, 0x22334455, 0x33445566, 0x44556677
      };
      REQUIRE(tag.value() == expected);
    }
    SECTION("LongArrayTag") {
      NBTFile file{"./test/data/long_array_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::LONG_ARRAY);
      LongArrayTag tag = file.readTag<LongArrayTag>();
      REQUIRE(tag.name() == "long array tag");
      std::vector<int64_t> expected{
        0x1122334455667708, 0x2233445566778809,
        0x334455667788990a, 0x445566778899aa0b
      };
      REQUIRE(tag.value() == expected);
    }
    SECTION("StringTag") {
      NBTFile file{"./test/data/string_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::STRING);
      StringTag tag = file.readTag<StringTag>();
      REQUIRE(tag.name() == "string tag");
      std::string expected{"The quick brown fox jumped over the lazy dog"};
      REQUIRE(tag.value() == expected);
    }
    SECTION("ListTag<ByteTag>") {
      NBTFile file{"./test/data/list_byte_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::LIST);
      ListTag<ByteTag> tag = file.readTagList<ByteTag>();
      REQUIRE(tag.name() == "listof byte tag");
      std::vector<int8_t> expected{
        0x7f, 0x6e, 0x5d, 0x4c
      };
      REQUIRE(tag.size() == expected.size());
      REQUIRE(tag.value() == expected);
    }
    SECTION("ListTag<StringTag>") {
      NBTFile file{"./test/data/list_string_tag.dat"};
      TagID id = file.readID();
      REQUIRE(id == TagID::LIST);
      ListTag<StringTag> tag = file.readTagList<StringTag>();
      REQUIRE(tag.name() == "list tag");
      std::vector<std::string> expected{
        "Roses are red",
        "Violets are blue",
        "C++ is a language for me and you"
      };
      REQUIRE(tag.size() == expected.size());
      for (size_t i = 0; i < expected.size(); i++) {
        REQUIRE(tag.value().at(i) == expected.at(i));
      }
    }
    SECTION("CompoundTag") {
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
      CompoundTag tag{file.readCompoundTag("")};
      REQUIRE(tag.size() == 4);
      { // StringTag
        REQUIRE(tag.at(0)->id() == TagID::STRING);
        StringTag child = std::move(*std::dynamic_pointer_cast<StringTag>(tag.at(0)));
        REQUIRE(child.name()  == "string child");
        REQUIRE(child.value() == "Hello world");
      }

      { // LongTag
        REQUIRE(tag.at(1)->id() == TagID::LONG);
        LongTag child = *std::dynamic_pointer_cast<LongTag>(tag.at(1));
        REQUIRE(child.name() == "long child");
        REQUIRE(child.value() == 0x7766554433221100);
      }

      { // IntArrayTag
        REQUIRE(tag.at(2)->id() == TagID::INT_ARRAY);
        IntArrayTag child = *std::dynamic_pointer_cast<IntArrayTag>(tag.at(2));
        REQUIRE(child.name() == "int array child");
        std::vector<int32_t> expected{0x33221100, 0x00112233};
        REQUIRE(child.value() == expected);
      }
    }
    SECTION("ListTag") {
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
      REQUIRE(tag.name() == "listof compound");
      REQUIRE(tag.size() == 2);

      {
        //CompoundTag child0 = tag.at(0);
        //REQUIRE(child0.at(0)->id() == TagID::STRING);
        //StringTag string = *std::dynamic_pointer_cast<StringTag>(child0.at(0));
        //REQUIRE(string.name() == "string child");
        //REQUIRE(string.value() == "asdfsdfg");
        //REQUIRE(child0.at(1)->id() == TagID::LONG_ARRAY);
      }
    }
  }
    // TODO
    // Test bad paths:
    // - File doesn't exist
    // - File ends unexpectedly (e.g. list with length 2 and 0 real members)
    // - CompoundTag with no EndTag (falls into "file ends unexpectedly")
}
