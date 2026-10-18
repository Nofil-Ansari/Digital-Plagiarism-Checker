---
milestone: Plagiarism Checker GUI & Enhanced Workbench
version: 1.1.0
updated: 2026-10-18
---

# Roadmap

> **Current Phase:** All Phases Complete
> **Status:** ✅ Complete

## Must-Haves (from SPEC)
- [x] Dual-Mode Desktop GUI (Dear ImGui + OpenGL3)
- [x] Strategy-based Pattern Searching (Naive, KMP, Rabin-Karp)
- [x] Plagiarism Metrics (Jaccard + N-Gram Overlap)
- [x] Native OS File Open Dialogs
- [x] Multi-Document Pairwise Comparison Matrix

---

## Phases

### Phase 1: Native File Dialog Integration
**Status:** ✅ Complete
**Objective:** Replace manual file path typing in the Dear ImGui interface with native Windows file picker dialogs (`GetOpenFileName` / Win32 API).
**Requirements:** Native Windows dialog integration without external heavyweight dependencies.

**Plans:**
- [x] Plan 1.1: Native Windows File Picker integration for Document Comparison and Pattern Search.

---

### Phase 2: Multi-Document Matrix Comparison
**Status:** ✅ Complete
**Objective:** Add a new tab allowing users to add/select multiple documents and compute an $N \times N$ similarity matrix.
**Depends on:** Phase 1

**Plans:**
- [x] Plan 2.1: Multi-Document Matrix Domain Logic & Data Structures (PlagiarismCore.hpp)
- [x] Plan 2.2: Multi-Document Matrix GUI Tab & Heatmap Table (gui_main.cpp)

---

## Progress Summary

| Phase | Status | Plans | Complete |
|---|---|---|---|
| 1 | ✅ Complete | 1/1 | 100% |
| 2 | ✅ Complete | 2/2 | 100% |
