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


#include <arpa/inet.h>

#include "nbt.hpp"


NBTFile::NBTFile(std::string filename)
  : file{filename, std::ios_base::in | std::ios_base::binary}
{
  if (!file.is_open()) {
    std::cerr << "ERR: NBTFile is not open" << std::endl;
  }
}

NBTFile::~NBTFile() {
  // file is automatically closed
  file.close();
}

static inline uint16_t swap16(uint16_t x) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return __builtin_bswap16(x);
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  return x;
#else
#error "Unsupported host byte order"
#endif
}

static inline uint32_t swap32(uint32_t x) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return __builtin_bswap32(x);
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  return x;
#else
#error "Unsupported host byte order"
#endif
}

static inline uint64_t swap64(uint64_t x) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return __builtin_bswap64(x);
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  return x;
#else
#error "Unsupported host byte order"
#endif
}




template<>
ByteTag::type ByteTag::ftoh(ByteTag::type unswapped) {
  return unswapped;
}

template<>
ByteTag::type ByteTag::htof(ByteTag::type unswapped) {
  return unswapped;
}

template<>
ShortTag::type ShortTag::ftoh(ShortTag::type unswapped) {
  return swap16(unswapped);
}

template<>
ShortTag::type ShortTag::htof(ShortTag::type unswapped) {
  return swap16(unswapped);
}

template<>
IntTag::type IntTag::ftoh(IntTag::type unswapped) {
  return swap32(unswapped);
}

template<>
IntTag::type IntTag::htof(IntTag::type unswapped) {
  return swap32(unswapped);
}

template<>
LongTag::type LongTag::ftoh(LongTag::type unswapped) {
  return swap64(unswapped);
}

template<>
LongTag::type LongTag::htof(LongTag::type unswapped) {
  return swap64(unswapped);
}

template<>
FloatTag::type FloatTag::ftoh(FloatTag::type unswapped) {
  uint32_t swapped = swap32(reinterpret_cast<uint32_t&>(unswapped));
  return reinterpret_cast<float&>(swapped);
}

template<>
FloatTag::type FloatTag::htof(FloatTag::type unswapped) {
  uint32_t swapped = swap32(reinterpret_cast<uint32_t&>(unswapped));
  return reinterpret_cast<float&>(swapped);
}

template<>
DoubleTag::type DoubleTag::ftoh(DoubleTag::type unswapped) {
  uint64_t swapped = swap64(reinterpret_cast<uint64_t&>(unswapped));
  return reinterpret_cast<double&>(swapped);
}

template<>
DoubleTag::type DoubleTag::htof(DoubleTag::type unswapped) {
  uint64_t swapped = swap64(reinterpret_cast<uint64_t&>(unswapped));
  return reinterpret_cast<double&>(swapped);
}

template<>
ByteArrayTag::type ByteArrayTag::ftoh(ByteArrayTag::type unswapped) {
  return unswapped;
}

template<>
ByteArrayTag::type ByteArrayTag::htof(ByteArrayTag::type unswapped) {
  return unswapped;
}

template<>
IntArrayTag::type IntArrayTag::ftoh(IntArrayTag::type unswapped) {
  for (auto it = unswapped->begin(); it != unswapped->end(); it++) {
    *it = swap32(*it);
  }
  return unswapped;
}

template<>
IntArrayTag::type IntArrayTag::htof(IntArrayTag::type unswapped) {
  for (auto it = unswapped->begin(); it != unswapped->end(); it++) {
    *it = swap32(*it);
  }
  return unswapped;
}

template<>
LongArrayTag::type LongArrayTag::ftoh(LongArrayTag::type unswapped) {
  for (auto it = unswapped->begin(); it != unswapped->end(); it++) {
    *it = swap64(*it);
  }
  return unswapped;
}

template<>
LongArrayTag::type LongArrayTag::htof(LongArrayTag::type unswapped) {
  for (auto it = unswapped->begin(); it != unswapped->end(); it++) {
    *it = swap64(*it);
  }
  return unswapped;
}


TagID NBTFile::readID() {
  char rawID;
  file.read(&rawID, sizeof(char)); 
  return static_cast<TagID>(rawID);
}

std::string NBTFile::readName() {
  // NOTE: Names are null-terminated, except when they're empty.
  uint16_t nameSize;
  file.read(reinterpret_cast<char*>(&nameSize), sizeof(uint16_t));
  nameSize = swap16(nameSize);
  std::unique_ptr<char[]> name = std::make_unique<char[]>(nameSize);
  file.read(name.get(), nameSize);
  return std::string{name.get(), nameSize};
}

int32_t NBTFile::readSize() {
  int32_t size;
  file.read(reinterpret_cast<char*>(&size), sizeof(size));
  return swap32(size);
}

int16_t NBTFile::readListSize() {
  int16_t size;
  file.read(reinterpret_cast<char*>(&size), sizeof(size));
  return swap16(size);
}

template <typename T>
T NBTFile::readTag(std::string name) {
  typename T::type value;
  file.read(reinterpret_cast<char*>(&value), sizeof(value));
  return T{name, T::ftoh(std::move(value))};
}

template <typename T>
T NBTFile::readTag() {
  std::string name = readName();
  return readTag<T>(name);
}

/**
 * Specialization for EndTag, which has no name or value.
 */
template <>
EndTag NBTFile::readTag<EndTag>() {
  return EndTag{};
}

/**
 * Specialization for StringTag.
 */
template <>
StringTag NBTFile::readTag<StringTag>(std::string name) {
  int16_t length;
  file.read(reinterpret_cast<char*>(&length), sizeof(length));
  length = swap16(length);
  std::string str(static_cast<size_t>(length), static_cast<char>('\0'));
  file.read(&str[0], length*sizeof(char));
  return StringTag{name, std::unique_ptr<std::string>{new std::string{str}}};
}

template <>
StringTag NBTFile::readTag<StringTag>() {
  std::string name = readName();
  return readTag<StringTag>(name);
}

/**
 * Specializations for TagArray types.
 */
template <>
ByteArrayTag NBTFile::readTag<ByteArrayTag>(std::string name) {
  return readTagArray<ByteArrayTag>(name, readSize());
}

template <>
ByteArrayTag NBTFile::readTag<ByteArrayTag>() {
  return readTag<ByteArrayTag>(readName());;
}

template <>
IntArrayTag NBTFile::readTag<IntArrayTag>(std::string name) {
  return readTagArray<IntArrayTag>(name, readSize());
}

template <>
IntArrayTag NBTFile::readTag<IntArrayTag>() {
  return readTag<IntArrayTag>(readName());;
}

template <>
LongArrayTag NBTFile::readTag<LongArrayTag>(std::string name) {
  return readTagArray<LongArrayTag>(name, readSize());
}

template <>
LongArrayTag NBTFile::readTag<LongArrayTag>() {
  return readTag<LongArrayTag>(readName());;
}

/**
 * Helper for reading arrays of fixed-size values.
 */
template <typename T>
T NBTFile::readTagArray(std::string name, int32_t size) {
  T tag{name, std::make_unique<typename T::type::element_type>(size)};
  for (int i = 0; i < size; i++) {
    file.read(reinterpret_cast<char*>(&(*tag.value)[i]),
              sizeof(typename T::type::element_type::value_type));
  }
  return tag;
}


template <typename T>
ListTag<T> NBTFile::readTagList(TagID id, std::string name) {
  int16_t size = readListSize();
  ListTag<T> list{name, size};
  for (int i = 0; i < size; i++) {
    list.push_back(readTag<T>(""));
  }
  return list;
}

template <typename T>
ListTag<T> NBTFile::readTagList() {
  std::string name = readName();
  TagID id = readID();
  return readTagList<T>(id, name);
}


template ListTag<ByteTag> NBTFile::readTagList<ByteTag>();
template ListTag<ShortTag> NBTFile::readTagList<ShortTag>();
template ListTag<IntTag> NBTFile::readTagList<IntTag>();
template ListTag<LongTag> NBTFile::readTagList<LongTag>();
template ListTag<FloatTag> NBTFile::readTagList<FloatTag>();
template ListTag<DoubleTag> NBTFile::readTagList<DoubleTag>();
template ListTag<ByteArrayTag> NBTFile::readTagList<ByteArrayTag>();
template ListTag<IntArrayTag> NBTFile::readTagList<IntArrayTag>();
template ListTag<LongArrayTag> NBTFile::readTagList<LongArrayTag>();
template ListTag<StringTag> NBTFile::readTagList<StringTag>();

template<>
ListTag<CompoundTag> NBTFile::readTagList<CompoundTag>(TagID id, std::string name) {
  int32_t size = readSize();
  ListTag<CompoundTag> list{name, id, size};
  for (int i = 0; i < size; i++) {
    list.push_back(readCompoundTag(""));
  }
  return std::move(list);
}

template ListTag<CompoundTag> NBTFile::readTagList<CompoundTag>();

template<>
ListTag<EndTag> NBTFile::readTagList<EndTag>(TagID id, std::string name) {
  int32_t size = readSize();
  ListTag<EndTag> list{name, size};
  for (int i = 0; i < size; i++) {
    list.push_back(readTag<EndTag>());
  }
  return std::move(list);
}

void ListTag<CompoundTag>::push_back(CompoundTag tag) {
  (*this->value)[tail] = std::move(tag);
}


static void deleteListTag(void *pTag) {
  ListTag<ByteTag> *pFakeList = reinterpret_cast<ListTag<ByteTag>*>(pTag);
  TagID realID = pFakeList->getChildID();
  switch (realID) {
    case TagID::COMPOUND:
      delete reinterpret_cast<ListTag<CompoundTag>*>(pTag);
      break;
    case TagID::END:
      delete reinterpret_cast<ListTag<EndTag>*>(pTag);
      break;
    case TagID::BYTE:
      delete reinterpret_cast<ListTag<ByteTag>*>(pTag);
      break;
    case TagID::SHORT:
      delete reinterpret_cast<ListTag<ShortTag>*>(pTag);
      break;
    case TagID::INT:
      delete reinterpret_cast<ListTag<IntTag>*>(pTag);
      break;
    case TagID::LONG:
      delete reinterpret_cast<ListTag<LongTag>*>(pTag);
      break;
    case TagID::FLOAT:
      delete reinterpret_cast<ListTag<FloatTag>*>(pTag);
      break;
    case TagID::DOUBLE:
      delete reinterpret_cast<ListTag<DoubleTag>*>(pTag);
      break;
    case TagID::BYTE_ARRAY:
      delete reinterpret_cast<ListTag<ByteArrayTag>*>(pTag);
      break;
    case TagID::STRING:
      delete reinterpret_cast<ListTag<StringTag>*>(pTag);
      break;
    default:
      delete reinterpret_cast<ListTag<StringTag>*>(pTag);
    case TagID::LIST:
      // TODO
      break;
    case TagID::INT_ARRAY:
      delete reinterpret_cast<ListTag<IntArrayTag>*>(pTag);
      break;
    case TagID::LONG_ARRAY:
      delete reinterpret_cast<ListTag<LongArrayTag>*>(pTag);
      break;
  }
}

CompoundTag::~CompoundTag() {
  for (size_t i = 0; i < ids.size(); i++) {
    void *pTag = value.at(i);
    switch (ids.at(i)) {
      case TagID::END:
        delete reinterpret_cast<EndTag*>(pTag);
        break;
      case TagID::BYTE:
        delete reinterpret_cast<ByteTag*>(pTag);
        break;
      case TagID::SHORT:
        delete reinterpret_cast<ShortTag*>(pTag);
        break;
      case TagID::INT:
        delete reinterpret_cast<IntTag*>(pTag);
        break;
      case TagID::LONG:
        delete reinterpret_cast<LongTag*>(pTag);
        break;
      case TagID::FLOAT:
        delete reinterpret_cast<FloatTag*>(pTag);
        break;
      case TagID::DOUBLE:
        delete reinterpret_cast<DoubleTag*>(pTag);
        break;
      case TagID::BYTE_ARRAY:
        delete reinterpret_cast<ByteArrayTag*>(pTag);
        break;
      case TagID::STRING:
        delete reinterpret_cast<StringTag*>(pTag);
        break;
      case TagID::LIST:
        // TODO
        deleteListTag(reinterpret_cast<ListTag<ByteTag>*>(pTag));
        break;
      case TagID::COMPOUND:
        delete reinterpret_cast<CompoundTag*>(pTag);
        break;
      case TagID::INT_ARRAY:
        delete reinterpret_cast<IntArrayTag*>(pTag);
        break;
      case TagID::LONG_ARRAY:
        delete reinterpret_cast<LongArrayTag*>(pTag);
        break;
    }
  }
}


CompoundTag NBTFile::readCompoundTag() {
  std::string name = readName();
  return readCompoundTag(name);
}

CompoundTag NBTFile::readCompoundTag(std::string name) {
  CompoundTag ct{name};
  bool end = false;
  while (!end) {
    TagID id = readID();
    switch (id) {
      case TagID::END:
        end = true;
        break;
      case TagID::BYTE:
        ct.push_back(id, readTag<ByteTag>());
        break;
      case TagID::SHORT:
        ct.push_back(id, readTag<ShortTag>());
        break;
      case TagID::INT:
        ct.push_back(id, readTag<IntTag>());
        break;
      case TagID::LONG:
        ct.push_back(id, readTag<LongTag>());
        break;
      case TagID::FLOAT:
        ct.push_back(id, readTag<FloatTag>());
        break;
      case TagID::DOUBLE:
        ct.push_back(id, readTag<DoubleTag>());
        break;
      case TagID::BYTE_ARRAY:
        ct.push_back(id, readTag<ByteArrayTag>());
        break;
      case TagID::STRING:
        ct.push_back(id, readTag<StringTag>());
        break;
      case TagID::LIST:
        {
          //ct.push_back(id, readTag<ListTag>());
          // Read contained TypeID
          std::string listName = readName();
          TagID listID = readID();
          switch (listID) {
            case TagID::END:
              ct.push_back(id, readTagList<EndTag>(listID, listName));
              break;
            case TagID::BYTE:
              ct.push_back(id, readTagList<ByteTag>(listID, listName));
              break;
            case TagID::SHORT:
              ct.push_back(id, readTagList<ShortTag>(listID, listName));
              break;
            case TagID::INT:
              ct.push_back(id, readTagList<IntTag>(listID, listName));
              break;
            case TagID::LONG:
              ct.push_back(id, readTagList<LongTag>(listID, listName));
              break;
            case TagID::FLOAT:
              ct.push_back(id, readTagList<FloatTag>(listID, listName));
              break;
            case TagID::DOUBLE:
              ct.push_back(id, readTagList<DoubleTag>(listID, listName));
              break;
            case TagID::BYTE_ARRAY:
              ct.push_back(id, readTagList<ByteArrayTag>(listID, listName));
              break;
            case TagID::STRING:
              ct.push_back(id, readTagList<StringTag>(listID, listName));
              break;
            //case TagID::LIST:
            //  //ct.push_back(id, readTag<ListTag>());
            //  // Read contained TypeID
            //  //ct.push_back(id, readTagList());
            //  break;
            case TagID::COMPOUND:
              // Unexpected
              ct.push_back(id, readTagList<CompoundTag>(listID, listName));
              break;
            case TagID::INT_ARRAY:
              ct.push_back(id, readTagList<IntArrayTag>(listID, listName));
              break;
            case TagID::LONG_ARRAY:
              ct.push_back(id, readTagList<LongArrayTag>(listID, listName));
              break;
            default:
              throw NBTTagException(listID, "Unrecognized tag");
              break;
          }
        }
        break;
      case TagID::COMPOUND:
        ct.push_back(id, readCompoundTag());
        break;
      case TagID::INT_ARRAY:
        ct.push_back(id, readTag<IntArrayTag>());
        break;
      case TagID::LONG_ARRAY:
        ct.push_back(id, readTag<LongArrayTag>());
        break;
      default:
        throw NBTTagException(id, "Unrecognized tag");
        break;
    }
  }
  return ct;
}
