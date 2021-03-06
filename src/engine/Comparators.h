// Copyright 2015, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold (buchhold@informatik.uni-freiburg.de)
#pragma once

#include <utility>
#include <vector>

using std::pair;
using std::vector;

template <typename E>
class OBComp {
 public:
  OBComp(const vector<pair<size_t, bool>>& sortIndices)
      : _sortIndices(sortIndices) {}

  bool operator()(const E& a, const E& b) const {
    for (auto& entry : _sortIndices) {
      if (a[entry.first] < b[entry.first]) {
        return !entry.second;
      }
      if (a[entry.first] > b[entry.first]) {
        return entry.second;
      }
    }
    return a[0] < b[0];
  }

 private:
  vector<pair<size_t, bool>> _sortIndices;
};