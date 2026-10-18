# PRD: Digital Plagiarism Checker & GUI

## Project Overview
The Digital Plagiarism Checker is a high-performance C++ program implementing string searching algorithms (Naive, KMP, Rabin-Karp) and document similarity metrics (Jaccard Token Similarity, N-gram overlap). This project adds an interactive Dear ImGui GUI (with OpenGL3/GLFW), a comprehensive technical README with Big-O complexity analysis and Mermaid diagrams, and an MIT License.

## Target Output Files
- `PRD.md`: Master specification of tasks (Read-only during loop iterations).
- `progress.txt`: Append-only execution journal tracked across Ralph Loop iterations.
- `LICENSE`: Standard MIT License attributing copyright to Nofil Ansari (2026).
- `README.md`: Professional Markdown documentation with architecture diagrams, algorithmic deep dives, Big-O tables, and build guides.
- `CMakeLists.txt`: Build specification for compiling the project and fetching/configuring Dear ImGui + GLFW/OpenGL3.
- `gui_main.cpp`: Interactive Dear ImGui desktop application with Dual-Mode workflow (Document Comparison + Pattern Search).

---

## Tasks Breakdown

### Task 1: Open Source Licensing
- **Description**: Add standard MIT License attributing copyright to Nofil Ansari (2026).
- **Deliverable**: `LICENSE` file in the workspace root.
- **Verification**: Ensure the file contains standard MIT terms, current year (2026), and "Nofil Ansari".

### Task 2: Comprehensive Technical README & Big-O Documentation
- **Description**: Replace the text-based README with an in-depth, professional `README.md`.
- **Key Sections**:
  - Architectural overview (Strategy Pattern, Tokenizer, TextIndexer, NGramGenerator, PlagiarismChecker).
  - Algorithmic deep-dive for Naive String Matching, Knuth-Morris-Pratt (Prefix function / LPS array), and Rabin-Karp (Rolling hash with polynomial base).
  - Plagiarism metrics deep-dive for Token-level Jaccard Similarity and N-gram Overlap.
  - Complete Big-O Complexity Table (Best, Average, Worst Time Complexity + Auxiliary Space Complexity).
  - Mermaid architecture & flow diagrams.
  - Windows MinGW / CMake compilation and usage guide.
- **Deliverable**: `README.md`
- **Verification**: Markdown parses cleanly, all algorithms and complexity metrics are accurately covered.

### Task 3: Refactor C++ Core Logic for Headless & GUI Interoperability
- **Description**: Ensure the algorithm implementations, classes, and helper structures in `Digital_Plagiarism_checker.cpp` can be cleanly linked or included by the GUI without duplicate `main()` collisions or CLI blocking.
- **Deliverable**: Separate core logic into clean header/source modules (e.g., `PlagiarismChecker.hpp` or modularized structure) while preserving existing CLI entry point.
- **Verification**: The core logic compiles cleanly with `g++` and passes standalone tests.

### Task 4: CMake Configuration for Dear ImGui & Desktop App
- **Description**: Create `CMakeLists.txt` configured to build the executable with MinGW-W64 (`g++`), fetching Dear ImGui and GLFW/OpenGL3 backends.
- **Deliverable**: `CMakeLists.txt`
- **Verification**: Running `cmake -B build -G "MinGW Makefiles"` configures without syntax errors.

### Task 5: Implement Dual-Mode Dear ImGui Interface
- **Description**: Implement `gui_main.cpp` using Dear ImGui featuring:
  - **Dark Premium Aesthetic**: Styled fonts, clean padding, modern dark theme.
  - **Tab 1 - Document Similarity**: Compare two loaded files (`file1.txt` vs `file2.txt`), calculate Jaccard & N-gram overlap %, display side-by-side text with overlap statistics.
  - **Tab 2 - Pattern Search & Benchmark**: Load document, input query string, pick algorithm (Naive / KMP / Rabin-Karp), toggle case-sensitivity and whole-word matching, display match count, positions, and execution time in microseconds.
  - **Export Report**: Export search & similarity findings to file.
- **Deliverable**: `gui_main.cpp`
- **Verification**: Source builds and runs, rendering the interactive ImGui window.

### Task 6: End-to-End Build, Run & Verification
- **Description**: Compile both CLI and GUI targets, test document comparison between `file1.txt` and `file2.txt`, verify search algorithm outputs match, and ensure reports are generated correctly.
- **Deliverable**: Compiled binaries in `build/`.
- **Verification**: Executable runs smoothly, results match expected plagiarism metrics, and clean progress report is committed.
