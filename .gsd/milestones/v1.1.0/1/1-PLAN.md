---
phase: 1
plan: 1
wave: 1
---

# Plan 1.1: Native Windows File Picker Integration

## Objective
Enhance the user experience of `gui_main.cpp` by adding native Windows File Open dialogs (`GetOpenFileNameA` from `comdlg32`) so users can click a "Browse..." button next to each document input field instead of manually entering file paths.

## Context
- `.gsd/SPEC.md`
- `.gsd/ARCHITECTURE.md`
- `gui_main.cpp`
- `CMakeLists.txt`

## Tasks

<task type="auto">
  <name>Link Windows Common Dialogs in CMake</name>
  <files>CMakeLists.txt</files>
  <action>
    Add `comdlg32` to the `target_link_libraries` list of `PlagiarismCheckerGUI` in CMakeLists.txt so that `GetOpenFileName` is available at link time.
    Avoid adding external package dependencies; keep it purely native to Windows.
  </action>
  <verify>cmake --build build --config Release</verify>
  <done>CMake successfully links `comdlg32` to `PlagiarismCheckerGUI` without errors.</done>
</task>

<task type="auto">
  <name>Implement Native File Open Dialog in Dear ImGui UI</name>
  <files>gui_main.cpp</files>
  <action>
    1. Implement a helper function `std::string OpenFileDialog(const char *filter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0");` using Windows `OPENFILENAMEA`.
    2. Add "Browse..." buttons next to Document 1 Path, Document 2 Path, and Target Document Path.
    3. When selected, automatically update the file buffer and load the document content into the preview.
  </action>
  <verify>cmake --build build --config Release</verify>
  <done>The executable builds cleanly and the "Browse..." buttons open the native OS file picker.</done>
</task>

## Success Criteria
- [ ] `PlagiarismCheckerGUI.exe` builds cleanly with `comdlg32` linked.
- [ ] Clicking "Browse..." opens a native Windows file selection dialog.
- [ ] Selected file paths populate the input fields and preview panes automatically.
