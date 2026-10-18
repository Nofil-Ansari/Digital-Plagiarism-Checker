# SPEC.md — Project Specification

> **Status**: `FINALIZED`
>
> ⚠️ **Planning Lock**: Marked `FINALIZED` on 2026-10-18.

## Vision
The Digital Plagiarism Checker is a modern, cross-platform C++ application engineered to compare documents for plagiarism (using Jaccard similarity and $N$-gram analysis) and perform rapid pattern searching with customizable algorithmic strategies (Naive, KMP, Rabin-Karp) via both a high-performance CLI and a hardware-accelerated Dear ImGui desktop interface.

## Goals
1. **Interactive Dual-Mode Desktop GUI** — Provide visual document similarity comparison with side-by-side preview and pattern search with microsecond execution benchmarking.
2. **Polymorphic Algorithmic Search** — Implement interchangeable search strategies (Naive, Knuth-Morris-Pratt, Rabin-Karp) adhering to the Strategy pattern.
3. **Robust Plagiarism Metrics** — Deliver both lexical (token Jaccard) and syntactic ($N$-gram set overlap) metrics.
4. **Production-Ready Documentation & Open-Source Packaging** — Maintain comprehensive architectural documentation, Big-O complexity analyses, and MIT licensing.

## Non-Goals (Out of Scope)
- Cloud-hosted web SaaS platform (desktop native focus).
- Deep learning semantic embeddings / vector database infrastructure (deterministic algorithmic focus).
- PDF/Word binary parsing (standard UTF-8 / plain-text focus).

## Constraints
- **Language**: Modern C++ (C++17 or C++20 standard).
- **GUI Engine**: Dear ImGui with GLFW and OpenGL3 backends.
- **Build System**: CMake with MinGW-W64 GCC on Windows.
- **Platform**: Windows 10/11 native support.

## Success Criteria
- [x] Core algorithm logic decoupled into reusable header `PlagiarismCore.hpp`.
- [x] Dear ImGui desktop interface running with dual tabs (Document Comparison + Pattern Search).
- [x] Zero-warning compilation with strict `-Wall -Wextra -Wconversion` flags.
- [x] MIT License and comprehensive `README.md` with Big-O table and Mermaid charts.
- [ ] Native OS file dialog integration (open file dialog instead of manual text paths).
- [ ] Multi-document batch plagiarism comparison matrix.

---
*Last updated: 2026-10-18*
