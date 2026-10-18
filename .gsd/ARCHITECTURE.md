# Architecture

> Updated on 2026-10-18 post-Milestone v1.1.0

## Overview

The **Digital Plagiarism Checker & String Search Workbench** is a high-performance C++ system designed to solve three core information retrieval tasks:
1. **Exact Pattern Matching**: Fast searching of single queries across text using polymorphic algorithmic strategies (Naive, Knuth-Morris-Pratt, and Rabin-Karp) with sub-microsecond precision benchmarking.
2. **Lexical & Syntactic Plagiarism Detection**: Quantifying document similarity between two texts using token-level Jaccard similarity and $N$-gram sequence overlap.
3. **Corpus Matrix Comparison**: Multi-document pairwise comparison computing an $N \times N$ similarity matrix rendered as a color-coded heatmap with interactive tooltips.

The system provides dual interfaces: a headless Command-Line Interface (CLI) and an interactive **Dear ImGui** Desktop GUI (OpenGL3 / GLFW / Win32).

```
┌─────────────────────────────────────────────────────────────┐
│                 User Presentation Layer                     │
│  [Digital_Plagiarism_checker.cpp]   [gui_main.cpp]          │
│            (CLI Mode)               (3-Tab Dear ImGui GUI)  │
├─────────────────────────────────────────────────────────────┤
│                 Core Domain & Engine Layer                  │
│                   (PlagiarismCore.hpp)                      │
│                                                             │
│   ┌────────────────────┐          ┌──────────────────────┐  │
│   │   Search Engine    │          │  Plagiarism Checker  │  │
│   │  (Strategy Pattern)│          │  (Jaccard & N-Grams) │  │
│   └─────────┬──────────┘          └──────────┬───────────┘  │
│             │                                │              │
│   ┌─────────▼──────────┐          ┌──────────▼───────────┐  │
│   │   IHashStrategy    │          │   CorpusComparer     │  │
│   │(Naive / KMP / RK)  │          │ ($N \times N$ Matrix)│  │
│   └────────────────────┘          └──────────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│                  Data Access & I/O Layer                    │
│      [TextDocument]      [TextIndexer]     [Win32 Dialogs]  │
│       (File Streams)    (Inverted Index)  (GetOpenFileName) │
└─────────────────────────────────────────────────────────────┘
```

---

## Components

### 1. Presentation Layer
- **CLI Target (`Digital_Plagiarism_checker.cpp`)**
  - Interactive terminal interface for file selection, algorithm choosing, benchmark timing, and report output generation.
- **GUI Target (`gui_main.cpp`)**
  - Dear ImGui hardware-accelerated GUI structured across three tabs:
    1. **Document Comparison**: Pairwise Jaccard and N-Gram similarity comparison with side-by-side text viewers and native file pickers.
    2. **Pattern Search Workbench**: Exact pattern searching with Naive, KMP, and Rabin-Karp algorithm options and microsecond execution timer.
    3. **Corpus Matrix Comparison**: Dynamic multi-file list loader, pairwise $N \times N$ similarity matrix calculation, metric toggle, and color-coded heatmap table with tooltips.

### 2. Algorithmic Search Engine (Strategy Pattern)
- **`IHashStrategy` & Implementations (`PlagiarismCore.hpp:143-234`)**
  - **`NaiveStrategy`**: Brute-force sliding window comparison ($O(n \cdot m)$ worst case).
  - **`KMPStrategy`**: Linear matching using a precomputed prefix function / LPS array ($O(n + m)$).
  - **`RabinKarpStrategy`**: Rolling hash matching using a 64-bit polynomial base with exact character verification on match ($O(n + m)$ average).
  - **`HashStrategyFactory`**: Factory returning `std::unique_ptr<IHashStrategy>` based on string identifier (`"rabin"`, `"kmp"`, `"naive"`).
  - **`SearchEngine`**: Orchestrator encapsulating the strategy execution.

### 3. Plagiarism & Corpus Engine
- **`Tokenizer` (`PlagiarismCore.hpp:33-49`)**: Normalizes raw document text, strips punctuation, downcases characters, and extracts lexical word tokens.
- **`NGramGenerator` (`PlagiarismCore.hpp:51-71`)**: Builds consecutive $N$-gram sequence tokens delimited by `\x1F`.
- **`PlagiarismChecker` & `DocumentComparer` (`PlagiarismCore.hpp:264-293`)**: Calculates Jaccard token overlap percentage and $N$-gram set overlap percentage between two documents.
- **`CorpusComparer` & `SimilarityMatrix` (`PlagiarismCore.hpp:349-399`)**: Executes pairwise all-to-all comparisons across a list of loaded `TextDocument` instances, populating an $N \times N$ matrix of similarity values.

### 4. Storage & Platform I/O
- **`TextDocument` (`PlagiarismCore.hpp:73-98`)**: In-memory document model loading binary/text files and tracking token counts.
- **`TextIndexer` (`PlagiarismCore.hpp:100-141`)**: Inverted index mapping word tokens to character positions.
- **`OutputWriter` (`PlagiarismCore.hpp:295-321`)**: Formats and exports search and plagiarism reports to `search_output.txt`.
- **Native File Dialogs (`gui_main.cpp:25-42`)**: Win32 `GetOpenFileNameA` integration wrapped in `OpenFileDialog()`.

---

## Technical Debt & Status

- [x] **Compiler Warning Cleanup**: Refactored `KMPStrategy` to use `size_t` rather than mixed signed `int` arithmetic, passing `-Wconversion` and `-Wsign-conversion` cleanly.
- [x] **Decoupled Architecture**: Logic extracted into `PlagiarismCore.hpp` to prevent duplicate implementations between CLI and GUI.
- [x] **Multi-Document Batch Comparison**: Implemented via `CorpusComparer` and GUI Heatmap Table.
- [x] **Native File Picker Dialog**: Implemented via Win32 `GetOpenFileNameA`.
- [ ] **Cross-Platform Dialog Support**: On non-Windows platforms (Linux/macOS), extend `OpenFileDialog()` with zenity/kdialog or tinyfiledialogs.
