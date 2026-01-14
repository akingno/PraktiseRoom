//
// Created by jacob on 25-10-26.
//

#ifndef SHORTMEMORY_H
#define SHORTMEMORY_H

#include <vector>
#include <string>

struct Memory {
  std::string content;
};

class ShortMemory {
public:

  void add(std::string content) {
    if (memories.size() >= cap) {
      memories.erase(memories.begin());
    }
    memories.push_back({std::move(content)});
  }

  [[nodiscard]] const std::vector<Memory>& entries() const {
    return memories;
  }

  [[nodiscard]] std::string to_string() const {
    if(memories.empty()) {
      return "no memory";
    }
    std::string ret;
    for (const auto& mem : memories) {
        ret.append(mem.content+"\n");
    }
    return ret;
  }

 private:
  static constexpr size_t cap = 12;
  std::vector<Memory> memories;
};

#endif //SHORTMEMORY_H
