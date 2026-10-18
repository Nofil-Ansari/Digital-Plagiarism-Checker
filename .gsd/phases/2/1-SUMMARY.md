---
phase: 2
plan: 1
status: complete
---

# Summary: Plan 2.1 — Multi-Document Matrix Domain Logic

## What Was Done
- Added `SimilarityMatrix` struct to `PlagiarismCore.hpp` containing:
  - `documentNames` — vector of basename strings for display
  - `jaccardMatrix` — N×N `vector<vector<double>>` for Jaccard overlap %
  - `ngramMatrix` — N×N `vector<vector<double>>` for N-gram overlap %
- Added `pathBasename()` utility function to extract filenames from paths
- Added `CorpusComparer` class with static `compareCorpus(docs, ngramSize)` method:
  - Symmetric optimization: computes only `N*(N-1)/2` pairs
  - Diagonal self-similarity set to 100.0%
  - Pure header-only C++17 compliant

## Verification
- Compiled with `g++ -std=c++17 -Wall -Wextra -Wconversion` — **zero warnings**

## Commit
- `f4a343c` — feat(phase-2): add SimilarityMatrix and CorpusComparer to PlagiarismCore.hpp
