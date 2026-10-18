---
phase: 2
plan: 2
wave: 2
depends_on:
  - 1
---

# Plan 2.2: Multi-Document Matrix GUI Tab (Corpus Analysis)

## Objective
Add a 3rd dedicated tab to the Dear ImGui interface: **"Corpus Similarity Matrix"**. This tab allows users to select multiple `.txt` files or add documents to a corpus list, run pairwise analysis, and render a colored $N \times N$ matrix table with heatmap styling (Green < 20%, Yellow 20-50%, Red > 50%).

## Context
- `.gsd/phases/2/1-PLAN.md`
- `gui_main.cpp`
- `PlagiarismCore.hpp`

## Tasks

<task type="auto">
  <name>Build Corpus Analysis Tab in Dear ImGui</name>
  <files>gui_main.cpp</files>
  <action>
    1. In `gui_main.cpp`, add Tab 3: `"Corpus Matrix Comparison"`.
    2. Provide controls to:
       - Add files via `OpenFileDialog()`.
       - Display the active list of loaded documents with removal buttons.
       - Include default test buttons (e.g., "Load Default Corpus" loading `file1.txt` and `file2.txt`).
       - Slider for $N$-gram size.
       - "Calculate Matrix" execution button.
    3. Render the computed `SimilarityMatrix` using `ImGui::BeginTable`:
       - Column headers showing document basenames.
       - Interactive toggle between Jaccard view and $N$-gram view.
       - Color-coded cells based on threshold percentage (Red for high overlap, Yellow for medium, Green for low).
       - Hover tooltip showing exact comparison details.
  </action>
  <verify>cmake --build build --config Release</verify>
  <done>`PlagiarismCheckerGUI.exe` compiles, links, and displays the interactive Corpus Matrix tab.</done>
</task>

## Success Criteria
- [ ] Users can add multiple documents to an active corpus.
- [ ] Clicking "Calculate Matrix" runs `CorpusComparer::compareCorpus`.
- [ ] Table renders an $N \times N$ color-coded similarity grid.
- [ ] Users can toggle between Jaccard and $N$-gram matrices.
