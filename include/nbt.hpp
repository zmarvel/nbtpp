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


#ifndef NBT_HPP
#define NBT_HPP

#include <cinttypes>
#include <string>
#include <vector>
#include <memory>
#include <fstream>

enum class TagID {
  END = 0,
  BYTE = 1,
  SHORT = 2,
  INT = 3,
  LONG = 4,
  FLOAT = 5,
  DOUBLE = 6,
  BYTE_ARRAY = 7,
  STRING = 8,
  LIST = 9,
  COMPOUND = 10,
  INT_ARRAY = 11,
  LONG_ARRAY = 12,
};

class TagBase { };

template <TagID id, typename T>
struct Tag : TagBase {
  public:
    typedef T type;
    Tag(std::string name, T value) : name{name}, value{std::move(value)} { }

    std::string name;
    T value;

    constexpr TagID getID() {
      return id;
    }
};


//using EndTag = Tag<TagID::END, void>;
//template<>
//class Tag<TagID::END, void> { };

// don't inherit from Tag<id, T> to avoid unnecessary members
class EndTag : TagBase {
  constexpr TagID getID() {
    return TagID::END;
  }
};

// can't do this because Tag has a `value` member that cannot inhabit `void`
//class EndTag : Tag<TagID::END, void> { };

using ByteTag = Tag<TagID::BYTE, int8_t>;
using ShortTag = Tag<TagID::SHORT, int16_t>;
using IntTag = Tag<TagID::INT, int32_t>;
using LongTag = Tag<TagID::LONG, int64_t>;
using FloatTag = Tag<TagID::FLOAT, float>;
using DoubleTag = Tag<TagID::DOUBLE, double>;


// can't do this because of the additional `size` field
//template <TagID id, typename T>
//using ArrayTag = Tag<id, std::unique_ptr<std::vector<T>>>;

//template <TagID id, typename T>
//struct ArrayTag : public Tag<id, std::unique_ptr<std::vector<T>>> {
//  typedef memberType T;
//  ArrayTag(std::string name, std::unique_ptr<std::vector<T>>);
//  int32_t size;
//};

//using ByteArrayTag = ArrayTag<TagID::BYTE_ARRAY, int8_t>;
//using IntArrayTag = ArrayTag<TagID::INT_ARRAY, int32_t>;
//using LongArrayTag = ArrayTag<TagID::LONG_ARRAY, int64_t>;
using ByteArrayTag = Tag<TagID::BYTE_ARRAY, std::unique_ptr<std::vector<int8_t>>>;
using IntArrayTag = Tag<TagID::INT_ARRAY, std::unique_ptr<std::vector<int32_t>>>;
using LongArrayTag = Tag<TagID::LONG_ARRAY, std::unique_ptr<std::vector<int64_t>>>;

// Could I constrain T to Tag<id, T>??
//template <typename T>
//class ArrayTag {
//  std::string name;
//  std::vector<T> values;
//};


template <typename T>
class ListTag : public Tag<TagID::LIST, std::unique_ptr<std::vector<typename T::type>>> {
  public:
    explicit ListTag(std::string name, TagID memberID, int32_t size) :
      Tag<TagID::LIST, std::unique_ptr<std::vector<typename T::type>>>
        (name, std::move(std::make_unique<std::vector<typename T::type>>(size))),
      memberID{memberID},
      size{size} { }

    void push_back(T tag) {
      (this->value)->push_back(std::move(tag.value));
    }

    TagID memberID;
    int32_t size;
};



// How can we store different types of values without using void*?
class CompoundTag : TagBase {
  // Append `_v` because a lot of names collide with keywords...
  //union {
  //  ByteTag::type vByte;
  //  ShortTag::type vShort;
  //  IntTag::type vInt;
  //  LongTag::type vLong;
  //  FloatTag::type vFloat;
  //  DoubleTag::type vDouble;

  //  ByteArrayTag::type vByteArray;
  //  IntArrayTag::type vIntArray;
  //  LongArrayTag::type vLongArray;
  //} Value;
  public:
    explicit CompoundTag(std::string name) : name{name} { }

    template <typename T>
      void push_back(TagID id, T tag) {
        ids.push_back(id);
        value.push_back(tag);
      }

    std::string name;

  private:
    std::vector<TagID> ids;
    std::vector<TagBase> value;
};





class NBTFile {
  public:
    explicit NBTFile(std::string filename);
    ~NBTFile();

    // no copy
    NBTFile(NBTFile& other) = delete;

    // only move
    NBTFile& operator=(NBTFile&& other);
    NBTFile(NBTFile&& other);

    template <typename T>
    T readTag();

    template <typename T>
    T readTagArray();

    template <typename T>
    ListTag<T> readTagList();

    template <typename T>
    ListTag<T> readTagList(TagID id, std::string name);

    CompoundTag readCompoundTag();

  private:
    std::string readName();
    std::ifstream file;
};





#endif // NBT_HPP
