

#include <iostream>

#include "nbt.hpp"


const char *USAGE = " input_file [-o output_file]\n"
"\n"
"    input_file                  NBT file\n"
"\n"
"    -o, --output output_file    File to which dump NBT structure and\n"
"                                contents should be dumped (default=stdout)\n";


int main(int argc, char* argv[]) {

  if (argc < 2) {
    std::cerr << "Not enough arguments" << std::endl << argv[0] << USAGE;
    return 1;
  }

  NBTFile input{argv[1]};
  // Assume the first ID identifies a Compound tag
  //TagID id = input.readID();

  try {
    ListTag<CompoundTag> root{input.readTagList<CompoundTag>()};
  }
  catch (NBTTagException& e) {
    std::cerr << "NBTTagException: " << e.what() << std::endl;
  }
  /*
  try {
    CompoundTag root = input.readCompoundTag("");
    StringTag child = root.at<StringTag>(0);
    std::cout << *child.value << std::endl;
  }
  catch (NBTTagException& e) {
    std::cerr << "NBTTagException: " << e.what() << std::endl;
  }
  */

  return 0;
}
