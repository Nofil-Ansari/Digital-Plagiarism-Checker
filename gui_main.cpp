#include "PlagiarismCore.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#endif

static std::string OpenFileDialog(const char *filter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0") {
#ifdef _WIN32
    char szFileName[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "txt";

    if (GetOpenFileNameA(&ofn)) {
        return std::string(szFileName);
    }
#endif
    return "";
}

static void ApplyModernDarkTheme() {
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;

    style.WindowRounding = 8.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 5.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 5.0f;
    style.WindowPadding = ImVec2(14, 14);
    style.FramePadding = ImVec2(10, 6);
    style.ItemSpacing = ImVec2(10, 8);
    style.ItemInnerSpacing = ImVec2(6, 6);

    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.55f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.15f, 0.19f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.14f, 0.18f, 0.96f);
    colors[ImGuiCol_Border] = ImVec4(0.24f, 0.27f, 0.33f, 0.60f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.20f, 0.26f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.28f, 0.36f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.28f, 0.33f, 0.44f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.11f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.14f, 0.16f, 0.21f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.15f, 0.19f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.11f, 0.14f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.26f, 0.30f, 0.39f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.33f, 0.38f, 0.49f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.40f, 0.46f, 0.60f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.38f, 0.72f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.38f, 0.72f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.55f, 0.82f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.22f, 0.45f, 0.75f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.54f, 0.88f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.62f, 0.98f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.34f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.33f, 0.45f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.32f, 0.40f, 0.54f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.18f, 0.24f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.28f, 0.36f, 0.50f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.22f, 0.45f, 0.75f, 1.00f);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return 1;
    }

    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow *window = glfwCreateWindow(1100, 760, "Digital Plagiarism Checker & String Search Workbench", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    ApplyModernDarkTheme();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // State for Tab 1: Plagiarism Comparison
    char file1Path[260] = "file1.txt";
    char file2Path[260] = "file2.txt";
    std::string file1Content, file2Content;
    int ngramSize = 3;
    bool compared = false;
    double jaccardResult = 0.0;
    double ngramResult = 0.0;
    std::string comparisonStatusMsg = "Enter file paths and click 'Compare Documents'.";

    // State for Tab 2: Pattern Search
    char searchFilePath[260] = "file1.txt";
    char patternBuf[256] = "plagiarism";
    int selectedAlgo = 0; // 0: Rabin-Karp, 1: KMP, 2: Naive
    const char *algoNames[] = { "Rabin-Karp", "KMP", "Naive Brute Force" };
    bool ignoreCase = true;
    bool wholeWord = false;
    bool verifyMatches = true;
    SearchResult searchResult;
    double searchDurationUs = 0.0;
    double singleJaccard = 0.0;
    double singleNgram = 0.0;
    bool searchExecuted = false;
    std::string searchStatusMsg = "Load a file and search a pattern.";
    std::string loadedSearchDocContent;

    // State for Tab 3: Corpus Matrix Comparison
    std::vector<TextDocument> corpusDocs;
    std::vector<std::string> corpusPaths;
    SimilarityMatrix corpusMatrix;
    bool matrixComputed = false;
    bool showNgramMatrix = false;   // false = Jaccard, true = N-gram
    int corpusNgramSize = 3;
    std::string corpusStatusMsg = "Add documents to the corpus, then click 'Calculate Matrix'.";

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(display_w), static_cast<float>(display_h)));

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

        ImGui::Begin("MainWindow", nullptr, windowFlags);

        ImGui::TextColored(ImVec4(0.38f, 0.72f, 0.98f, 1.0f), "DIGITAL PLAGIARISM CHECKER & ALGORITHM WORKBENCH");
        ImGui::SameLine();
        ImGui::TextDisabled("| High-Performance C++ String Engine");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTabBar("MainTabs")) {
            // TAB 1: Document Plagiarism Comparison
            if (ImGui::BeginTabItem("Document Plagiarism Comparison")) {
                ImGui::Spacing();
                ImGui::Text("Compare two documents to calculate lexical Jaccard similarity and N-gram syntactic overlap.");
                ImGui::Spacing();

                ImGui::Columns(2, "DocInputs", false);

                ImGui::Text("Document 1 Path:");
                ImGui::InputText("##Doc1Path", file1Path, sizeof(file1Path));
                ImGui::SameLine();
                if (ImGui::Button("Browse...##Doc1")) {
                    std::string picked = OpenFileDialog();
                    if (!picked.empty()) {
                        strncpy(file1Path, picked.c_str(), sizeof(file1Path) - 1);
                        file1Path[sizeof(file1Path) - 1] = '\0';
                        try {
                            TextDocument d(file1Path);
                            file1Content = d.content();
                            comparisonStatusMsg = "Document 1 loaded: " + picked;
                        } catch (const std::exception &e) {
                            comparisonStatusMsg = std::string("Error loading Doc 1: ") + e.what();
                        }
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Load Doc 1")) {
                    try {
                        TextDocument d(file1Path);
                        file1Content = d.content();
                        comparisonStatusMsg = "Document 1 loaded successfully.";
                    } catch (const std::exception &e) {
                        comparisonStatusMsg = std::string("Error loading Doc 1: ") + e.what();
                    }
                }

                ImGui::NextColumn();

                ImGui::Text("Document 2 Path:");
                ImGui::InputText("##Doc2Path", file2Path, sizeof(file2Path));
                ImGui::SameLine();
                if (ImGui::Button("Browse...##Doc2")) {
                    std::string picked = OpenFileDialog();
                    if (!picked.empty()) {
                        strncpy(file2Path, picked.c_str(), sizeof(file2Path) - 1);
                        file2Path[sizeof(file2Path) - 1] = '\0';
                        try {
                            TextDocument d(file2Path);
                            file2Content = d.content();
                            comparisonStatusMsg = "Document 2 loaded: " + picked;
                        } catch (const std::exception &e) {
                            comparisonStatusMsg = std::string("Error loading Doc 2: ") + e.what();
                        }
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Load Doc 2")) {
                    try {
                        TextDocument d(file2Path);
                        file2Content = d.content();
                        comparisonStatusMsg = "Document 2 loaded successfully.";
                    } catch (const std::exception &e) {
                        comparisonStatusMsg = std::string("Error loading Doc 2: ") + e.what();
                    }
                }

                ImGui::Columns(1);
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::SliderInt("N-Gram Size (phrase depth)", &ngramSize, 1, 10);
                ImGui::SameLine();
                if (ImGui::Button("Compare Documents", ImVec2(160, 32))) {
                    try {
                        TextDocument d1(file1Path);
                        TextDocument d2(file2Path);
                        file1Content = d1.content();
                        file2Content = d2.content();

                        auto start = std::chrono::high_resolution_clock::now();
                        auto res = DocumentComparer::compare(d1, d2, static_cast<size_t>(ngramSize));
                        auto end = std::chrono::high_resolution_clock::now();
                        double us = std::chrono::duration<double, std::micro>(end - start).count();

                        jaccardResult = res.jaccard;
                        ngramResult = res.ngram;
                        compared = true;

                        std::ostringstream ss;
                        ss << "Compared in " << std::fixed << std::setprecision(2) << us << " us. "
                           << "Doc1 words: " << d1.wordCount() << ", Doc2 words: " << d2.wordCount();
                        comparisonStatusMsg = ss.str();
                    } catch (const std::exception &e) {
                        comparisonStatusMsg = std::string("Comparison Error: ") + e.what();
                    }
                }

                ImGui::SameLine();
                ImGui::TextDisabled("%s", comparisonStatusMsg.c_str());

                ImGui::Spacing();

                if (compared) {
                    ImGui::BeginChild("MetricsFrame", ImVec2(0, 100), true);
                    ImGui::Columns(2, "MetricColumns", true);

                    ImVec4 jColor = (jaccardResult > 50.0) ? ImVec4(0.95f, 0.35f, 0.35f, 1.0f) :
                                    (jaccardResult > 20.0) ? ImVec4(0.95f, 0.75f, 0.25f, 1.0f) :
                                                             ImVec4(0.35f, 0.85f, 0.45f, 1.0f);

                    ImGui::Text("Jaccard Token Overlap:");
                    ImGui::TextColored(jColor, "%.2f %%", jaccardResult);
                    ImGui::ProgressBar(static_cast<float>(jaccardResult / 100.0), ImVec2(-1, 16));

                    ImGui::NextColumn();

                    ImVec4 nColor = (ngramResult > 50.0) ? ImVec4(0.95f, 0.35f, 0.35f, 1.0f) :
                                    (ngramResult > 20.0) ? ImVec4(0.95f, 0.75f, 0.25f, 1.0f) :
                                                           ImVec4(0.35f, 0.85f, 0.45f, 1.0f);

                    ImGui::Text("%d-Gram Phrase Overlap:", ngramSize);
                    ImGui::TextColored(nColor, "%.2f %%", ngramResult);
                    ImGui::ProgressBar(static_cast<float>(ngramResult / 100.0), ImVec2(-1, 16));

                    ImGui::Columns(1);
                    ImGui::EndChild();
                }

                ImGui::Spacing();
                ImGui::Text("Document Preview (Side by Side):");
                ImGui::Columns(2, "PreviewCols", true);

                ImGui::Text("Document 1: %s", file1Path);
                ImGui::BeginChild("Doc1View", ImVec2(0, 260), true);
                ImGui::TextWrapped("%s", file1Content.empty() ? "(Load document to view contents)" : file1Content.c_str());
                ImGui::EndChild();

                ImGui::NextColumn();

                ImGui::Text("Document 2: %s", file2Path);
                ImGui::BeginChild("Doc2View", ImVec2(0, 260), true);
                ImGui::TextWrapped("%s", file2Content.empty() ? "(Load document to view contents)" : file2Content.c_str());
                ImGui::EndChild();

                ImGui::Columns(1);
                ImGui::EndTabItem();
            }

            // TAB 2: Pattern Search & Algorithm Benchmarking
            if (ImGui::BeginTabItem("Pattern Search & Benchmarks")) {
                ImGui::Spacing();
                ImGui::Text("Search string occurrences inside a document using polymorphic search strategies.");
                ImGui::Spacing();

                ImGui::Text("Target Document Path:");
                ImGui::InputText("##SearchDocPath", searchFilePath, sizeof(searchFilePath));
                ImGui::SameLine();
                if (ImGui::Button("Browse...##SearchDoc")) {
                    std::string picked = OpenFileDialog();
                    if (!picked.empty()) {
                        strncpy(searchFilePath, picked.c_str(), sizeof(searchFilePath) - 1);
                        searchFilePath[sizeof(searchFilePath) - 1] = '\0';
                        searchStatusMsg = "Target document selected: " + picked;
                    }
                }

                ImGui::Spacing();
                ImGui::Text("Pattern Query:");
                ImGui::InputText("##PatternInput", patternBuf, sizeof(patternBuf));

                ImGui::Spacing();
                ImGui::Text("Algorithm Strategy:");
                ImGui::Combo("##AlgoSelector", &selectedAlgo, algoNames, IM_ARRAYSIZE(algoNames));

                ImGui::Checkbox("Ignore Case", &ignoreCase);
                ImGui::SameLine();
                ImGui::Checkbox("Whole Word Only", &wholeWord);
                ImGui::SameLine();
                ImGui::Checkbox("Verify Exact Matches", &verifyMatches);

                ImGui::Spacing();
                if (ImGui::Button("Execute Search & Benchmark", ImVec2(240, 32))) {
                    try {
                        TextDocument doc(searchFilePath);
                        loadedSearchDocContent = doc.content();

                        SearchSettings settings;
                        settings.ignoreCase = ignoreCase;
                        settings.wholeWord = wholeWord;
                        settings.verifyMatches = verifyMatches;
                        settings.ngramSize = static_cast<size_t>(ngramSize);

                        if (selectedAlgo == 0) settings.algorithm = "rabin";
                        else if (selectedAlgo == 1) settings.algorithm = "kmp";
                        else settings.algorithm = "naive";

                        auto strat = HashStrategyFactory::create(settings.algorithm);
                        SearchEngine engine(std::move(strat));

                        auto start = std::chrono::high_resolution_clock::now();
                        searchResult = engine.search(doc, patternBuf, settings);

                        if (settings.verifyMatches && !searchResult.positions.empty()) {
                            std::vector<size_t> verified;
                            std::string t = doc.content();
                            std::string p = patternBuf;
                            if (settings.ignoreCase) {
                                std::transform(t.begin(), t.end(), t.begin(), [](unsigned char c){ return static_cast<char>(tolower(c)); });
                                std::transform(p.begin(), p.end(), p.begin(), [](unsigned char c){ return static_cast<char>(tolower(c)); });
                            }
                            for (size_t pos : searchResult.positions) {
                                if (pos + p.size() <= t.size() && t.compare(pos, p.size(), p) == 0) {
                                    verified.push_back(pos);
                                }
                            }
                            searchResult.positions.swap(verified);
                        }

                        auto end = std::chrono::high_resolution_clock::now();
                        searchDurationUs = std::chrono::duration<double, std::micro>(end - start).count();

                        singleJaccard = PlagiarismChecker::jaccardPercentage(doc.content(), patternBuf, ignoreCase);
                        singleNgram = PlagiarismChecker::ngramPercentage(doc.content(), patternBuf, static_cast<size_t>(ngramSize), ignoreCase);

                        searchExecuted = true;
                        std::ostringstream ss;
                        ss << "Found " << searchResult.frequency() << " matches in "
                           << std::fixed << std::setprecision(2) << searchDurationUs << " us using "
                           << algoNames[selectedAlgo];
                        searchStatusMsg = ss.str();
                    } catch (const std::exception &e) {
                        searchStatusMsg = std::string("Search Failed: ") + e.what();
                    }
                }

                ImGui::SameLine();
                if (ImGui::Button("Export to search_output.txt", ImVec2(200, 32))) {
                    try {
                        TextDocument doc(searchFilePath);
                        SearchSettings s;
                        s.algorithm = (selectedAlgo == 0 ? "rabin" : (selectedAlgo == 1 ? "kmp" : "naive"));
                        s.ignoreCase = ignoreCase;
                        s.wholeWord = wholeWord;
                        s.ngramSize = static_cast<size_t>(ngramSize);
                        OutputWriter::writeSearchReport("search_output.txt", doc, patternBuf, s, searchResult, singleJaccard, singleNgram);
                        searchStatusMsg = "Report saved to search_output.txt";
                    } catch (const std::exception &e) {
                        searchStatusMsg = std::string("Export Error: ") + e.what();
                    }
                }

                ImGui::Spacing();
                ImGui::TextDisabled("%s", searchStatusMsg.c_str());
                ImGui::Spacing();

                if (searchExecuted) {
                    ImGui::BeginChild("SearchResultsFrame", ImVec2(0, 180), true);
                    ImGui::TextColored(ImVec4(0.38f, 0.72f, 0.98f, 1.0f), "EXECUTION BENCHMARK & METRICS:");
                    ImGui::BulletText("Algorithm: %s", algoNames[selectedAlgo]);
                    ImGui::BulletText("Occurrences Count: %zu", searchResult.frequency());
                    ImGui::BulletText("Search Execution Time: %.3f microseconds (%.4f ms)", searchDurationUs, searchDurationUs / 1000.0);
                    ImGui::BulletText("Pattern vs Document Jaccard Overlap: %.2f %%", singleJaccard);
                    ImGui::BulletText("Pattern vs Document %d-Gram Overlap: %.2f %%", ngramSize, singleNgram);

                    ImGui::Spacing();
                    ImGui::Text("Occurrence Character Offsets:");
                    if (searchResult.positions.empty()) {
                        ImGui::TextDisabled("No matches found.");
                    } else {
                        std::string posStr;
                        for (size_t p : searchResult.positions) {
                            posStr += std::to_string(p) + " ";
                        }
                        ImGui::TextWrapped("%s", posStr.c_str());
                    }
                    ImGui::EndChild();
                }

                ImGui::EndTabItem();
            }

            // TAB 3: Corpus Similarity Matrix
            if (ImGui::BeginTabItem("Corpus Matrix Comparison")) {
                ImGui::Spacing();
                ImGui::Text("Add multiple documents and compute an N x N pairwise similarity matrix.");
                ImGui::Spacing();

                // ── Add Document Controls ──
                if (ImGui::Button("Add Document (Browse)...", ImVec2(200, 28))) {
                    std::string picked = OpenFileDialog();
                    if (!picked.empty()) {
                        try {
                            TextDocument doc(picked);
                            corpusDocs.push_back(std::move(doc));
                            corpusPaths.push_back(picked);
                            matrixComputed = false;
                            corpusStatusMsg = "Added: " + pathBasename(picked) + "  (" + std::to_string(corpusDocs.size()) + " docs total)";
                        } catch (const std::exception &e) {
                            corpusStatusMsg = std::string("Error loading file: ") + e.what();
                        }
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Load Default Corpus", ImVec2(180, 28))) {
                    corpusDocs.clear();
                    corpusPaths.clear();
                    matrixComputed = false;
                    const char *defaults[] = { "file1.txt", "file2.txt" };
                    for (auto &f : defaults) {
                        try {
                            TextDocument doc(f);
                            corpusDocs.push_back(std::move(doc));
                            corpusPaths.push_back(f);
                        } catch (const std::exception &e) {
                            corpusStatusMsg = std::string("Error loading ") + f + ": " + e.what();
                        }
                    }
                    if (!corpusDocs.empty())
                        corpusStatusMsg = "Default corpus loaded (" + std::to_string(corpusDocs.size()) + " docs).";
                }
                ImGui::SameLine();
                if (ImGui::Button("Clear All", ImVec2(100, 28))) {
                    corpusDocs.clear();
                    corpusPaths.clear();
                    matrixComputed = false;
                    corpusStatusMsg = "Corpus cleared.";
                }

                ImGui::Spacing();

                // ── Document List ──
                if (!corpusPaths.empty()) {
                    ImGui::Text("Loaded Documents (%zu):", corpusPaths.size());
                    ImGui::BeginChild("CorpusDocList", ImVec2(0, std::min(120.0f, static_cast<float>(corpusPaths.size()) * 26.0f + 10.0f)), true);
                    int removeIdx = -1;
                    for (int i = 0; i < static_cast<int>(corpusPaths.size()); ++i) {
                        ImGui::PushID(i);
                        if (ImGui::SmallButton("X")) {
                            removeIdx = i;
                        }
                        ImGui::SameLine();
                        ImGui::BulletText("[%d] %s  (%zu words)", i + 1, pathBasename(corpusPaths[static_cast<size_t>(i)]).c_str(), corpusDocs[static_cast<size_t>(i)].wordCount());
                        ImGui::PopID();
                    }
                    ImGui::EndChild();
                    if (removeIdx >= 0) {
                        corpusDocs.erase(corpusDocs.begin() + removeIdx);
                        corpusPaths.erase(corpusPaths.begin() + removeIdx);
                        matrixComputed = false;
                        corpusStatusMsg = "Document removed. " + std::to_string(corpusDocs.size()) + " docs remaining.";
                    }
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ── Controls ──
                ImGui::SliderInt("N-Gram Size##Corpus", &corpusNgramSize, 1, 10);
                ImGui::SameLine();
                if (ImGui::Button("Calculate Matrix", ImVec2(160, 28))) {
                    if (corpusDocs.size() < 2) {
                        corpusStatusMsg = "Need at least 2 documents to compute a matrix.";
                    } else {
                        auto start = std::chrono::high_resolution_clock::now();
                        corpusMatrix = CorpusComparer::compareCorpus(corpusDocs, static_cast<size_t>(corpusNgramSize));
                        auto end = std::chrono::high_resolution_clock::now();
                        double ms = std::chrono::duration<double, std::milli>(end - start).count();
                        matrixComputed = true;

                        std::ostringstream ss;
                        ss << "Matrix computed for " << corpusDocs.size() << " documents in "
                           << std::fixed << std::setprecision(2) << ms << " ms.";
                        corpusStatusMsg = ss.str();
                    }
                }

                ImGui::Spacing();
                ImGui::TextDisabled("%s", corpusStatusMsg.c_str());
                ImGui::Spacing();

                // ── Matrix Display ──
                if (matrixComputed) {
                    ImGui::Checkbox("Show N-gram Matrix", &showNgramMatrix);
                    ImGui::SameLine();
                    ImGui::TextDisabled("(%s)", showNgramMatrix ? "N-gram sequence overlap" : "Jaccard token overlap");
                    ImGui::Spacing();

                    const auto &matrix = showNgramMatrix ? corpusMatrix.ngramMatrix : corpusMatrix.jaccardMatrix;
                    size_t n = corpusMatrix.documentNames.size();

                    // Heatmap color helper
                    auto cellColor = [](double val) -> ImVec4 {
                        if (val > 50.0) return ImVec4(0.95f, 0.30f, 0.30f, 1.0f);  // Red
                        if (val > 20.0) return ImVec4(0.95f, 0.75f, 0.25f, 1.0f);  // Yellow
                        return ImVec4(0.30f, 0.85f, 0.45f, 1.0f);                   // Green
                    };
                    auto cellBgColor = [](double val) -> ImU32 {
                        if (val > 50.0) return IM_COL32(180, 50, 50, 80);
                        if (val > 20.0) return IM_COL32(180, 140, 30, 60);
                        return IM_COL32(50, 160, 70, 50);
                    };

                    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                                 ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
                    float tableHeight = std::min(400.0f, static_cast<float>(n + 1) * 30.0f + 30.0f);

                    if (ImGui::BeginTable("SimilarityMatrix", static_cast<int>(n) + 1, tableFlags, ImVec2(0, tableHeight))) {
                        // Header row
                        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 120.0f);
                        for (size_t c = 0; c < n; ++c) {
                            ImGui::TableSetupColumn(corpusMatrix.documentNames[c].c_str(), ImGuiTableColumnFlags_WidthFixed, 100.0f);
                        }
                        ImGui::TableHeadersRow();

                        // Data rows
                        for (size_t r = 0; r < n; ++r) {
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::TextUnformatted(corpusMatrix.documentNames[r].c_str());

                            for (size_t c = 0; c < n; ++c) {
                                ImGui::TableSetColumnIndex(static_cast<int>(c) + 1);
                                double val = matrix[r][c];
                                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cellBgColor(val));
                                ImGui::TextColored(cellColor(val), "%.1f%%", val);

                                // Hover tooltip
                                if (ImGui::IsItemHovered()) {
                                    ImGui::BeginTooltip();
                                    ImGui::Text("%s vs %s", corpusMatrix.documentNames[r].c_str(), corpusMatrix.documentNames[c].c_str());
                                    ImGui::Separator();
                                    ImGui::Text("Jaccard: %.2f%%", corpusMatrix.jaccardMatrix[r][c]);
                                    ImGui::Text("%d-gram:  %.2f%%", corpusNgramSize, corpusMatrix.ngramMatrix[r][c]);
                                    if (val > 50.0) ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "HIGH SIMILARITY");
                                    else if (val > 20.0) ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), "MODERATE SIMILARITY");
                                    else ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.4f, 1), "LOW SIMILARITY");
                                    ImGui::EndTooltip();
                                }
                            }
                        }
                        ImGui::EndTable();
                    }

                    // Legend
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.30f, 0.85f, 0.45f, 1.0f), "GREEN"); ImGui::SameLine();
                    ImGui::TextDisabled("< 20%%"); ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.25f, 1.0f), "  YELLOW"); ImGui::SameLine();
                    ImGui::TextDisabled("20-50%%"); ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.95f, 0.30f, 0.30f, 1.0f), "  RED"); ImGui::SameLine();
                    ImGui::TextDisabled("> 50%%");
                }

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();

        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.11f, 0.12f, 0.15f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
