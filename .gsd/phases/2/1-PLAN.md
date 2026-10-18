---
phase: 2
plan: 1
wave: 1
---

# Plan 2.1: Multi-Document Matrix Domain Logic & Data Structures

## Objective
Implement core domain structures in `PlagiarismCore.hpp` to load multiple documents simultaneously and calculate an $N \times N$ pairwise similarity matrix for both Jaccard token overlap and $N$-gram sequence overlap.

## Context
- `.gsd/SPEC.md`
- `.gsd/ARCHITECTURE.md`
- `PlagiarismCore.hpp`

## Tasks

<task type="auto">
  <name>Implement Multi-Document Corpus & Matrix Comparator</name>
  <files>PlagiarismCore.hpp</files>
  <action>
    1. Define `struct SimilarityMatrix` containing:
       - `std::vector<std::string> documentNames;`
       - `std::vector<std::vector<double>> jaccardMatrix;`
       - `std::vector<std::vector<double>> ngramMatrix;`
    2. Define `class CorpusComparer` with method:
       `static SimilarityMatrix compareCorpus(const std::vector<TextDocument> &docs, size_t ngramSize = 3);`
    3. Ensure symmetric computation optimization ($M[i][j] = M[j][i]$ and $M[i][i] = 100.0\%$).
    4. Maintain pure header-only C++17 compliance with zero warnings under `-Wall -Wextra -Wconversion`.
  </action>
  <verify>g++ -std=c++17 -Wall -Wextra -Wconversion -c Digital_Plagiarism_checker.cpp -o build/cli_test.o</verify>
  <done>`PlagiarismCore.hpp` compiles cleanly and provides `CorpusComparer::compareCorpus`.</done>
</task>

## Success Criteria
- [ ] `SimilarityMatrix` data structure models pairwise comparisons.
- [ ] `CorpusComparer` computes $N \times N$ matrices in $O(N^2 \times (\text{words}))$ without redundant duplicate calculations.
- [ ] Compiles with zero warnings.
