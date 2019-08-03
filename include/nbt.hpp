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
    Tag(std::string name, T value) : mName{name}, mValue{value} { }
    virtual ~Tag() { }

    static T ftoh(T unswapped);
    static T htof(T unswapped);

    virtual TagID id() const {
      return tagID;
    }

    std::string name() const {
      return mName;
    }

    T& value() {
      return mValue;
    }

  protected:
    std::string mName;
    T mValue;
};

template <TagID tagID, typename T>
struct ArrayTag : TagBase {
  public:
    typedef std::vector<T> type;
    ArrayTag(std::string name, std::vector<T> value) :
      mName{name}, mValue{value} { }
    virtual ~ArrayTag() { }

    static type ftoh(type unswapped);
    static type htof(type unswapped);

    virtual TagID id() const {
      return tagID;
    }

    std::string name() const {
      return mName;
    }

    std::vector<T>& value() {
      return mValue;
    }

    size_t size() const {
      return mValue.size();
    }

    void push_back(T val) {
      value().push_back(val);
    }

    const T& at(size_t i) const {
      return mValue.at(i);
    }

    T& at(size_t i) {
      return mValue.at(i);
    }

  private:
    std::string mName;
    std::vector<T> mValue;
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


using ByteArrayTag = ArrayTag<TagID::BYTE_ARRAY, int8_t>;
using IntArrayTag = ArrayTag<TagID::INT_ARRAY, int32_t>;
using LongArrayTag = ArrayTag<TagID::LONG_ARRAY, int64_t>;

using StringTag = Tag<TagID::STRING, std::string>;



template <typename T>
class ListTag : public TagBase {
  public:
    typedef std::vector<typename T::type> type;

    // Due to unique_ptr attribute, there will be no implicit copy operator
    explicit ListTag(std::string name, int32_t size) :
      mName{name},
      mValue{std::vector<typename T::type>()},
      mSize{size}
    {
      mValue.reserve(size);
    }

    ListTag(ListTag&& other) :
      mName{other.name()},
      mValue{std::move(other.value())},
      mSize{other.size()}
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

    std::string name() const {
      return mName;
    }

    std::vector<typename T::type>& value() {
      return mValue;
    }

    void push_back(T tag) {
      value().push_back(tag.value());
    }

    T at(size_t i) {
      return value().at(i);
    }

    int32_t size() const {
      return mSize;
    }

  private:
    std::string mName;
    std::vector<typename T::type> mValue;
    int32_t mSize;
};

template <>
class ListTag<EndTag> : public TagBase {
  public:
    explicit ListTag(std::string name, int32_t size) :
      mName{name},
      mSize{size} { }

    virtual ~ListTag() { }

    virtual TagID id() const override {
      return TagID::LIST;
    }

    virtual TagID childID() {
      return TagID::END;
    }

    template<typename T>
    ListTag<T> push_back(T tag) {
      ListTag<T> newList(name(), size());
      newList.push_back(std::move(tag));
      return newList;
    }

    std::string name() const {
      return mName;
    }

    int32_t size() const {
      return mSize;
    }

  private:
    std::string mName;
    int32_t mSize;
};

class CompoundTag;

template <>
constexpr TagID getTagID<CompoundTag>() {
  return TagID::COMPOUND;
}

template <>
class ListTag<CompoundTag> : public TagBase {
  public:
    typedef std::vector<CompoundTag> type;

    // Due to unique_ptr member, there will be no implicit copy constructor
    explicit ListTag(std::string name, TagID memberID, int32_t size) :
      mSize{size},
      mName{name},
      mValue{},
      mMemberID{memberID}
      {
        mValue.reserve(size);
      }

    ListTag(ListTag&& other) :
      mSize{other.size()},
      mName{other.name()},
      mValue{other.value()}
    { }

    virtual ~ListTag() { }

    virtual TagID id() const {
      return TagID::LIST;
    }

    virtual TagID getChildID() {
      return getTagID<CompoundTag>();
    }

    std::string name() const {
      return mName;
    }

    std::vector<CompoundTag>& value() {
      return mValue;
    }

    void push_back(CompoundTag tag);

    CompoundTag& at(size_t i) {
      return value().at(i);
    }

    int32_t size() const {
      return mSize;
    }

  private:
    int32_t mSize;
    std::string mName;
    std::vector<CompoundTag> mValue;
    TagID mMemberID;
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

    virtual ~CompoundTag() { }

    template<class T>
    void push_back(T tag) {
      std::shared_ptr<TagBase> tagCopy = std::make_shared<T>(std::move(tag));
      value().push_back(tagCopy);
    }

    std::shared_ptr<TagBase> at(size_t i) {
      return value().at(i);
    }

    size_t size() const {
      return mValue.size();
    }
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
      static std::string expl = why + ": " + std::to_string(static_cast<unsigned int>(id));
      return expl.c_str();
    }

    TagID id;
    std::string why;
};

class NBTException : public std::exception {
  public:
    explicit NBTException(const char* why) :
      why{why}
    { }

    virtual const char* what() const noexcept
    {
      return why;
    }

  private:
    const char* why;
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
    std::string readName();

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
    int32_t readListSize();
    int32_t readSize();
    std::ifstream file;
};





#endif // NBT_HPP
