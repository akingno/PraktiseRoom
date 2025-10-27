//
// Created by jacob on 25-10-26.
//

#ifndef SHORTMEMORY_H
#define SHORTMEMORY_H

#include <vector>
#include <string>

struct Memory {
  std::string content;
  double      valence;  // -2, -1, 0, +0.5, +1, +2
};

class ShortMemory {
public:

  void add(std::string content, double valence) {
    if (memories.size() >= cap) {
      memories.erase(memories.begin());
    }
    memories.push_back({std::move(content), valence});
  }

  [[nodiscard]] const std::vector<Memory>& entries() const { return memories; }

private:
  static constexpr size_t cap = 128;
  std::vector<Memory> memories;
};

#endif //SHORTMEMORY_H
