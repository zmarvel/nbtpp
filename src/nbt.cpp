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


#include "nbt.hpp"


NBTFile::NBTFile(std::string filename)
  : file{filename}
{ }

NBTFile::~NBTFile() {
  // file is automatically closed
}


NBTFile& NBTFile::operator=(NBTFile&& other) {
  file.swap(other.file);
  return *this;
}

NBTFile::NBTFile(NBTFile&& other) {
  std::swap(file, other.file);
}


std::string NBTFile::readName() {
  uint16_t nameSize;
  file.read(reinterpret_cast<char*>(&nameSize), sizeof(uint16_t));
  std::unique_ptr<char[]> name = std::make_unique<char[]>(nameSize);
  file.read(name.get(), nameSize);
  return std::string{name.get()};
}


template <typename T>
T NBTFile::readTag() {
  std::string name = readName();
  typename T::type value;
  file.read(reinterpret_cast<char*>(&value), sizeof(value));
  return T{name, std::move(value)};
}

template <typename T>
T NBTFile::readTagArray() {
  std::string name = readName();
  int32_t size;
  file.read(reinterpret_cast<char*>(&size), sizeof(size));
  //typename T::type value = std::make_unique<typename T::type::element_type>(size);
  T tag{name, std::make_unique<typename T::type::element_type>(size)};
  for (int i = 0; i < size; i++) {
    file.read(reinterpret_cast<char*>(&(*tag.value)[i]),
              sizeof(typename T::type::element_type::value_type));
  }
  return tag;
}

template<typename T>
ListTag<T> NBTFile::readTagList() {
  std::string name = readName();
  TagID id;
  file.read(reinterpret_cast<char*>(&id), sizeof(char)); 
  int32_t size;
  file.read(reinterpret_cast<char*>(&size), sizeof(int32_t)); 
  ListTag<T> list{name, id, size};
  for (int i = 0; i < size; i++) {
    list.push_back(readTag<T>());
  }
  return list;
}

template<typename T>
ListTag<T> NBTFile::readTagList(TagID id, std::string name) {
  int32_t size;
  file.read(reinterpret_cast<char*>(&size), sizeof(int32_t)); 
  ListTag<T> list{name, id, size};
  for (int i = 0; i < size; i++) {
    list.push_back(readTag<T>());
  }
  return list;
}


template <>
CompoundTag NBTFile::readTag() {
  std::string name = readName();
  CompoundTag ct{name};
  bool end = false;
  while (!end) {
    TagID id;
    file.read(reinterpret_cast<char*>(&id), sizeof(char)); 
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
        ct.push_back(id, readTagArray<ByteArrayTag>());
        break;
      case TagID::STRING:
        //ct.push_back(id, readTag<StringTag>());
        break;
      case TagID::LIST:
        {
          //ct.push_back(id, readTag<ListTag>());
          // Read contained TypeID
          std::string listName = readName();
          TagID listID;
          file.read(reinterpret_cast<char*>(&listID), sizeof(char));
          switch (listID) {
            case TagID::END:
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
              //ct.push_back(id, readTag<StringTag>());
              break;
            case TagID::LIST:
              //ct.push_back(id, readTag<ListTag>());
              // Read contained TypeID
              //ct.push_back(id, readTagList());
              break;
            case TagID::COMPOUND:
              // Unexpected
              //ct.push_back(id, readTagList<CompoundTag>());
              break;
            case TagID::INT_ARRAY:
              ct.push_back(id, readTagList<IntArrayTag>(listID, listName));
              break;
            case TagID::LONG_ARRAY:
              ct.push_back(id, readTagList<LongArrayTag>(listID, listName));
              break;
          }
        }
        break;
      case TagID::COMPOUND:
        ct.push_back(id, readTag<CompoundTag>());
        break;
      case TagID::INT_ARRAY:
        ct.push_back(id, readTagArray<IntArrayTag>());
        break;
      case TagID::LONG_ARRAY:
        ct.push_back(id, readTagArray<LongArrayTag>());
        break;
    }
  }
  return ct;
}
