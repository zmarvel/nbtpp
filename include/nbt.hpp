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

struct TagBase {
  TagBase() { }
  virtual ~TagBase() { }
  virtual TagID id() const = 0;
};


template <TagID tagID, typename T>
struct Tag : TagBase {
  public:
    typedef T type;
    Tag(std::string name, T value) : name{name}, value{std::move(value)} { }
    Tag(Tag&& other) : name{other.name}, value{std::move(other.value)} { }
    Tag& operator=(Tag&& other) {
      name = other.name;
      value = std::move(other.value);
      return *this;
    }
    virtual ~Tag() { }

    std::string name;
    T value;

    static T ftoh(T unswapped);
    static T htof(T unswapped);

    virtual TagID id() const {
      return tagID;
    }
};


template <typename T>
constexpr TagID getTagID();

// don't inherit from Tag<id, T> to avoid unnecessary members
class EndTag : TagBase {
  public:
    typedef void type;

    explicit EndTag() { }

    virtual ~EndTag() { }

    virtual TagID id() const override {
      return TagID::END;
    }
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

    ListTag(ListTag&& other) :
      Tag<TagID::LIST, std::unique_ptr<std::vector<typename T::type>>>
        (other.name, std::move(other.value)),
      size{other.size},
      curr{other.curr}
    { }

    virtual ~ListTag() {
      // As long as T::type is a value type, it will be freed automatically when
      // the ListTag<T> is destructed
    }

    virtual TagID getChildID() {
      return getTagID<T>();
    }

    virtual TagID id() const {
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

    virtual ~ListTag() { }

    virtual TagID id() const override {
      return TagID::LIST;
    }

    virtual TagID childID() {
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

    ListTag(ListTag&& other) :
      Tag<TagID::LIST, std::unique_ptr<std::vector<CompoundTag>>>
        (other.name, std::move(other.value)),
        size{other.size},
        tail{other.tail}
    { }

    virtual ~ListTag() { }

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







class CompoundTag : public Tag<TagID::COMPOUND, std::vector<std::shared_ptr<TagBase>>> {
  public:
    typedef CompoundTag type;
    CompoundTag() :
      Tag<TagID::COMPOUND, std::vector<std::shared_ptr<TagBase>>>{
        "", std::vector<std::shared_ptr<TagBase>>{}}
    { }

    CompoundTag(std::string name) :
      Tag<TagID::COMPOUND, std::vector<std::shared_ptr<TagBase>>>{
        name, std::vector<std::shared_ptr<TagBase>>{}}
    { }

    CompoundTag(CompoundTag&& other) :
      Tag<TagID::COMPOUND, std::vector<std::shared_ptr<TagBase>>>{
        other.name, std::move(other.value)}
    { }

    virtual ~CompoundTag() { }

    template<class T>
    void push_back(T tag) {
      std::shared_ptr<TagBase> tagCopy = std::make_shared<T>(std::move(tag));
      value.push_back(tagCopy);
    }

    std::shared_ptr<TagBase> at(size_t i) {
      return value.at(i);
    }

    std::string name;
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
