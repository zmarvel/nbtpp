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
#include <exception>
#include <iostream>
#include <cstring>


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

    static T ftoh(T unswapped);
    static T htof(T unswapped);

    constexpr TagID getID() {
      return id;
    }
};


template <typename T>
constexpr TagID getTagID();

// don't inherit from Tag<id, T> to avoid unnecessary members
class EndTag : TagBase {
  public:
    typedef void type;
    explicit EndTag() { }
    ~EndTag() = default;
};


using ByteTag = Tag<TagID::BYTE, int8_t>;
using ShortTag = Tag<TagID::SHORT, int16_t>;
using IntTag = Tag<TagID::INT, int32_t>;
using LongTag = Tag<TagID::LONG, int64_t>;
using FloatTag = Tag<TagID::FLOAT, float>;
using DoubleTag = Tag<TagID::DOUBLE, double>;


using ByteArrayTag = Tag<TagID::BYTE_ARRAY, std::unique_ptr<std::vector<int8_t>>>;
using IntArrayTag = Tag<TagID::INT_ARRAY, std::unique_ptr<std::vector<int32_t>>>;
using LongArrayTag = Tag<TagID::LONG_ARRAY, std::unique_ptr<std::vector<int64_t>>>;

using StringTag = Tag<TagID::STRING, std::unique_ptr<std::string>>;



template <typename T>
class ListTag : public Tag<TagID::LIST, std::unique_ptr<std::vector<typename T::type>>> {
  public:
    // Due to unique_ptr attribute, there will be no implicit copy operator
    explicit ListTag(std::string name, int32_t size) :
      Tag<TagID::LIST, std::unique_ptr<std::vector<typename T::type>>>
        (name, std::move(std::make_unique<std::vector<typename T::type>>(size))),
      size{size},
      curr{0}
    { }

    virtual TagID getChildID() {
      return getTagID<T>();
    }

    constexpr TagID getID() {
      return TagID::LIST;
    }

    void push_back(T tag) {
      (this->value)->at(curr) = std::move(tag.value);
      curr++;
    }

    T at(size_t i) {
      return (this->value)->at(i);
    }

    int32_t getSize() const {
      return size;
    }

    int32_t size;

  private:
    int32_t curr;
};

template <>
class ListTag<EndTag> : public TagBase {
  public:
    explicit ListTag(std::string name, int32_t size) :
      name{name},
      size{size} { }

    virtual TagID getChildID() {
      return TagID::END;
    }

    template<typename T>
    ListTag<T> push_back(T tag) {
      ListTag<T> newList(name, size);
      newList.push_back(std::move(tag));
      return newList;
    }

    std::string name;
    int32_t size;
};

class CompoundTag;

template <>
constexpr TagID getTagID<CompoundTag>() {
  return TagID::COMPOUND;
}

template <>
class ListTag<CompoundTag> : public Tag<TagID::LIST, std::unique_ptr<std::vector<CompoundTag>>> {
  public:
    // Due to unique_ptr member, there will be no implicit copy constructor
    explicit ListTag(std::string name, TagID memberID, int32_t size) :
      Tag<TagID::LIST, std::unique_ptr<std::vector<CompoundTag>>>
        (name, std::make_unique<std::vector<CompoundTag>>(size)),
      memberID{memberID},
      size{size},
      tail{0}
      { }

    virtual TagID getChildID() {
      return getTagID<CompoundTag>();
    }

    void push_back(CompoundTag tag);

    CompoundTag& at(size_t i) {
      return this->value->at(i);
    }

    int32_t getSize() const {
      return size;
    }

    TagID memberID;
    int32_t size;

  private:
    size_t tail;
};







class CompoundTag : TagBase {
  public:
    typedef CompoundTag type;
    CompoundTag() :
      name{}
    { }

    CompoundTag(std::string name) :
      name{name}
    { }

    ~CompoundTag();

    template <typename T>
    void push_back(TagID id, T tag) {
      ids.push_back(id);
      T *tagCopy = new T{std::move(tag)};
      value.push_back(tagCopy);
    }

    template <typename T>
    T at(size_t i) {
      return std::move(*reinterpret_cast<T*>(value.at(i)));
    }

    TagID idAt(size_t i) {
      return ids.at(i);
    }

    std::string name;

  private:
    // TODO unique_ptr
    std::vector<TagID> ids;
    std::vector<void*> value;
};





template <>
constexpr TagID getTagID<EndTag>() {
  return TagID::END;
}

template <>
constexpr TagID getTagID<ByteTag>() {
  return TagID::BYTE;
}

template <>
constexpr TagID getTagID<ShortTag>() {
  return TagID::SHORT;
}

template <>
constexpr TagID getTagID<IntTag>() {
  return TagID::INT;
}

template <>
constexpr TagID getTagID<LongTag>() {
  return TagID::LONG;
}

template <>
constexpr TagID getTagID<FloatTag>() {
  return TagID::FLOAT;
}

template <>
constexpr TagID getTagID<DoubleTag>() {
  return TagID::DOUBLE;
}

template <>
constexpr TagID getTagID<ByteArrayTag>() {
  return TagID::BYTE_ARRAY;
}

template <>
constexpr TagID getTagID<IntArrayTag>() {
  return TagID::INT_ARRAY;
}

template <>
constexpr TagID getTagID<LongArrayTag>() {
  return TagID::LONG_ARRAY;
}

// TODO
//template <typename T>
//constexpr TagID getTagID<ListTag<T>>() {
//  return TagID::LIST;
//}

template <>
constexpr TagID getTagID<StringTag>() {
  return TagID::STRING;
}






class NBTTagException : public std::exception {
  public:
    explicit NBTTagException(TagID id, std::string why) :
      id{id}, why{why}
    { }

    virtual const char* what() const noexcept
    {
      std::string expl = why + ": " + std::to_string(static_cast<int>(id));
      return expl.c_str();
    }

    TagID id;
    std::string why;
};


class NBTFile {
  public:
    explicit NBTFile(std::string filename);
    ~NBTFile();

    // no copy
    NBTFile(NBTFile& other) = delete;

    // only move
    NBTFile& operator=(NBTFile&& other) {
      file.swap(other.file);
      return *this;
    }
    NBTFile(NBTFile&& other) {
      std::swap(file, other.file);
    }

    TagID readID();

    template <typename T>
    T readTag();

    template <typename T>
    T readTag(std::string name);

    template <typename T>
    T readTagArray(std::string name, int32_t size);

    template <typename T>
    ListTag<T> readTagList();

    template <typename T>
    ListTag<T> readTagList(TagID id, std::string name);

    CompoundTag readCompoundTag();
    CompoundTag readCompoundTag(std::string name);

  private:
    std::string readName();
    int16_t readListSize();
    int32_t readSize();
    std::ifstream file;
};





#endif // NBT_HPP
