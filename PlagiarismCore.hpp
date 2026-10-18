#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct SearchSettings {
    bool ignoreCase = true;
    bool wholeWord = false;
    bool verifyMatches = true;
    size_t ngramSize = 3;
    std::string algorithm = "rabin";
};

class SearchResult {
public:
    std::vector<size_t> positions;
    size_t frequency() const { return positions.size(); }
    void add(size_t pos) { positions.push_back(pos); }
};

class Tokenizer {
public:
    static std::vector<std::string> tokenize(const std::string &text, bool lower = true) {
        std::vector<std::string> tokens;
        std::string cur;
        for (char ch : text) {
            if (isalnum(static_cast<unsigned char>(ch))) {
                cur.push_back(lower ? static_cast<char>(tolower(static_cast<unsigned char>(ch))) : ch);
            } else {
                if (!cur.empty()) {
                    tokens.push_back(cur);
                    cur.clear();
                }
            }
        }
        if (!cur.empty()) tokens.push_back(cur);
        return tokens;
    }
};

class NGramGenerator {
public:
    static std::unordered_set<std::string> genNGrams(const std::vector<std::string> &tokens, size_t n) {
        std::unordered_set<std::string> s;
        if (n == 0 || tokens.size() < n) return s;
        for (size_t i = 0; i + n <= tokens.size(); ++i) {
            std::string g = tokens[i];
            for (size_t j = 1; j < n; ++j) {
                g += '\x1F';
                g += tokens[i + j];
            }
            s.insert(std::move(g));
        }
        return s;
    }

    static std::unordered_set<std::string> genNGramsFromText(const std::string &text, size_t n, bool lower = true) {
        auto t = Tokenizer::tokenize(text, lower);
        return genNGrams(t, n);
    }
};

class TextDocument {
    std::string filename_, content_;
public:
    TextDocument() = default;
    explicit TextDocument(const std::string &filename) { loadFromFile(filename); }
    void loadFromFile(const std::string &filename) {
        std::ifstream in(filename, std::ios::in | std::ios::binary);
        if (!in) throw std::runtime_error("Failed to open file: " + filename);
        std::ostringstream ss;
        ss << in.rdbuf();
        content_ = ss.str();
        filename_ = filename;
    }
    void setContent(const std::string &name, const std::string &text) {
        filename_ = name;
        content_ = text;
    }
    const std::string &content() const noexcept { return content_; }
    const std::string &filename() const noexcept { return filename_; }
    size_t wordCount() const { return Tokenizer::tokenize(content_).size(); }
    std::vector<std::string> tokens(bool lower = true) const {
        return Tokenizer::tokenize(content_, lower);
    }
};

class TextIndexer {
    std::unordered_map<std::string, std::vector<size_t>> index_;
public:
    void build(const std::string &text, bool lower = true) {
        index_.clear();
        std::string cur;
        size_t tokenStart = std::string::npos;
        for (size_t i = 0; i < text.size(); ++i) {
            char ch = text[i];
            if (isalnum(static_cast<unsigned char>(ch))) {
                if (tokenStart == std::string::npos) tokenStart = i;
                cur.push_back(lower ? static_cast<char>(tolower(static_cast<unsigned char>(ch))) : ch);
            } else {
                if (!cur.empty()) {
                    index_[cur].push_back(tokenStart);
                    cur.clear();
                }
                tokenStart = std::string::npos;
            }
        }
        if (!cur.empty()) index_[cur].push_back(tokenStart);
    }
    const std::vector<size_t> &positionsFor(const std::string &word) const {
        static const std::vector<size_t> empty;
        auto it = index_.find(word);
        if (it == index_.end()) return empty;
        return it->second;
    }
};

class IHashStrategy {
public:
    virtual ~IHashStrategy() = default;
    virtual SearchResult search(const std::string &text, const std::string &pattern, const SearchSettings &settings) = 0;
};

class NaiveStrategy : public IHashStrategy {
public:
    SearchResult search(const std::string &text, const std::string &pattern, const SearchSettings &settings) override {
        if (pattern.empty()) throw std::invalid_argument("Pattern must not be empty.");
        std::string t = text;
        std::string p = pattern;
        if (settings.ignoreCase) {
            std::transform(t.begin(), t.end(), t.begin(), [](unsigned char c){ return static_cast<char>(tolower(c)); });
            std::transform(p.begin(), p.end(), p.begin(), [](unsigned char c){ return static_cast<char>(tolower(c)); });
        }
        SearchResult res;
        for (size_t i = 0; i + p.size() <= t.size(); ++i) {
            bool ok = true;
            for (size_t j = 0; j < p.size(); ++j) {
                if (t[i + j] != p[j]) { ok = false; break; }
            }
            if (ok) {
                if (settings.wholeWord) {
                    bool left_ok = (i == 0) || !isalnum(static_cast<unsigned char>(t[i-1]));
                    bool right_ok = (i + p.size() >= t.size()) || !isalnum(static_cast<unsigned char>(t[i + p.size()]));
                    if (!left_ok || !right_ok) continue;
                }
                res.add(i);
            }
        }
        return res;
    }
};

class KMPStrategy : public IHashStrategy {
    static std::vector<size_t> buildPrefix(const std::string &p) {
        size_t n = p.size();
        std::vector<size_t> pi(n, 0);
        for (size_t i = 1; i < n; ++i) {
            size_t j = pi[i - 1];
            while (j > 0 && p[i] != p[j]) j = pi[j - 1];
            if (p[i] == p[j]) ++j;
            pi[i] = j;
        }
        return pi;
    }
public:
    SearchResult search(const std::string &text, const std::string &pattern, const SearchSettings &settings) override {
        if (pattern.empty()) throw std::invalid_argument("Pattern must not be empty.");
        std::string t = text;
        std::string p = pattern;
        if (settings.ignoreCase) {
            std::transform(t.begin(), t.end(), t.begin(), [](unsigned char c){ return static_cast<char>(tolower(c)); });
            std::transform(p.begin(), p.end(), p.begin(), [](unsigned char c){ return static_cast<char>(tolower(c)); });
        }
        SearchResult res;
        std::vector<size_t> pi = buildPrefix(p);
        size_t j = 0;
        for (size_t i = 0; i < t.size(); ++i) {
            while (j > 0 && t[i] != p[j]) j = pi[j - 1];
            if (t[i] == p[j]) ++j;
            if (j == p.size()) {
                size_t start = i - p.size() + 1;
                if (settings.wholeWord) {
                    bool left_ok = (start == 0) || !isalnum(static_cast<unsigned char>(t[start - 1]));
                    bool right_ok = (start + p.size() >= t.size()) || !isalnum(static_cast<unsigned char>(t[start + p.size()]));
                    if (!left_ok || !right_ok) {
                        j = pi[j - 1];
                        continue;
                    }
                }
                res.add(start);
                j = pi[j - 1];
            }
        }
        return res;
    }
};

class RabinKarpStrategy : public IHashStrategy {
public:
    SearchResult search(const std::string &text, const std::string &pattern, const SearchSettings &settings) override {
        if (pattern.empty()) throw std::invalid_argument("Pattern must not be empty.");
        std::string t = text;
        std::string p = pattern;
        if (settings.ignoreCase) {
            std::transform(t.begin(), t.end(), t.begin(), [](unsigned char c){ return static_cast<char>(tolower(c)); });
            std::transform(p.begin(), p.end(), p.begin(), [](unsigned char c){ return static_cast<char>(tolower(c)); });
        }
        size_t n = t.size(), m = p.size();
        SearchResult res;
        if (m > n) return res;

        const uint64_t B = 1315423911ULL;
        auto hashOf = [&](const std::string &s, size_t start, size_t len) {
            uint64_t h = 0;
            for (size_t i = 0; i < len; ++i) {
                h = h * B + static_cast<unsigned char>(s[start + i]) + 1;
            }
            return h;
        };

        uint64_t patHash = hashOf(p, 0, m);
        uint64_t curHash = hashOf(t, 0, m);
        if (curHash == patHash) {
            if (!settings.wholeWord || ((!isalnum(static_cast<unsigned char>(t[0])) || true) && (m >= n || !isalnum(static_cast<unsigned char>(t[m]))))) {
                res.add(0);
            }
        }

        for (size_t i = 1; i + m <= n; ++i) {
            curHash = hashOf(t, i, m);
            if (curHash == patHash) {
                if (settings.wholeWord) {
                    bool left_ok = (i == 0) || !isalnum(static_cast<unsigned char>(t[i-1]));
                    bool right_ok = (i + m >= n) || !isalnum(static_cast<unsigned char>(t[i + m]));
                    if (!left_ok || !right_ok) continue;
                }
                res.add(i);
            }
        }
        return res;
    }
};

class HashStrategyFactory {
public:
    static std::unique_ptr<IHashStrategy> create(const std::string &name) {
        std::string n = name;
        std::transform(n.begin(), n.end(), n.begin(), [](unsigned char c){ return static_cast<char>(tolower(c)); });
        if (n == "rabin" || n == "rabin-karp" || n == "rk") {
            return std::make_unique<RabinKarpStrategy>();
        } else if (n == "kmp") {
            return std::make_unique<KMPStrategy>();
        } else if (n == "naive" || n == "brute") {
            return std::make_unique<NaiveStrategy>();
        } else {
            throw std::invalid_argument("Unknown algorithm: " + name);
        }
    }
};

class SearchEngine {
    std::unique_ptr<IHashStrategy> strategy_;
public:
    explicit SearchEngine(std::unique_ptr<IHashStrategy> strat) : strategy_(std::move(strat)) {}
    SearchResult search(const TextDocument &doc, const std::string &pattern, const SearchSettings &settings) {
        if (!strategy_) throw std::runtime_error("Search strategy not set.");
        return strategy_->search(doc.content(), pattern, settings);
    }
};

class PlagiarismChecker {
public:
    static double jaccardPercentage(const std::string &text, const std::string &pattern, bool lower = true) {
        auto ta = Tokenizer::tokenize(text, lower);
        auto pa = Tokenizer::tokenize(pattern, lower);
        std::unordered_set<std::string> ts(ta.begin(), ta.end()), ps(pa.begin(), pa.end());
        if (ps.empty() || ts.empty()) return 0.0;
        size_t inter = 0;
        for (const auto &w : ps) {
            if (ts.count(w)) ++inter;
        }
        size_t uni = ts.size() + ps.size() - inter;
        if (uni == 0) return 0.0;
        return (static_cast<double>(inter) / static_cast<double>(uni)) * 100.0;
    }

    static double ngramPercentage(const std::string &text, const std::string &pattern, size_t n = 3, bool lower = true) {
        if (n == 0) return 0.0;
        auto tg = NGramGenerator::genNGramsFromText(text, n, lower);
        auto pg = NGramGenerator::genNGramsFromText(pattern, n, lower);
        if (pg.empty() || tg.empty()) return 0.0;
        size_t inter = 0;
        for (const auto &g : pg) {
            if (tg.count(g)) ++inter;
        }
        size_t uni = tg.size() + pg.size() - inter;
        if (uni == 0) return 0.0;
        return (static_cast<double>(inter) / static_cast<double>(uni)) * 100.0;
    }
};

class DocumentComparer {
public:
    struct Result { double jaccard = 0.0; double ngram = 0.0; };
    static Result compare(const TextDocument &a, const TextDocument &b, size_t ngramSize = 3) {
        double j = PlagiarismChecker::jaccardPercentage(a.content(), b.content());
        double ng = PlagiarismChecker::ngramPercentage(a.content(), b.content(), ngramSize);
        return { j, ng };
    }
};

class OutputWriter {
public:
    static void writeSearchReport(const std::string &outFile, const TextDocument &doc, const std::string &pattern,
                                  const SearchSettings &settings, const SearchResult &result,
                                  double jaccardPercent, double ngramPercent) {
        std::ofstream out(outFile);
        if (!out) throw std::runtime_error("Failed to open output file: " + outFile);
        out << "File: " << doc.filename() << "\n";
        out << "Pattern: " << pattern << "\n";
        out << "Algorithm: " << settings.algorithm << "\n";
        out << "Ignore case: " << (settings.ignoreCase ? "yes" : "no") << "\n";
        out << "Whole word: " << (settings.wholeWord ? "yes" : "no") << "\n\n";
        out << "Occurrences (count = " << result.frequency() << "): ";
        if (result.positions.empty()) {
            out << "none\n";
        } else {
            for (size_t p : result.positions) out << p << " ";
            out << "\n";
        }
        out << "\nPlagiarism (similarity) metrics:\n";
        out << " - Jaccard token overlap: " << jaccardPercent << "%\n";
        out << " - " << settings.ngramSize << "-gram overlap: " << ngramPercent << "%\n";
        out.close();
    }
};

// ──────────────────────────────────────────────────────────────────
// Multi-Document Corpus Comparison (Phase 2)
// ──────────────────────────────────────────────────────────────────

/// Holds pairwise N×N similarity matrices for a corpus of documents.
struct SimilarityMatrix {
    std::vector<std::string> documentNames;              ///< Display names (basenames)
    std::vector<std::vector<double>> jaccardMatrix;      ///< N×N Jaccard token overlap %
    std::vector<std::vector<double>> ngramMatrix;        ///< N×N N-gram sequence overlap %
};

/// Utility: extract the filename basename from a full path.
inline std::string pathBasename(const std::string &path) {
    auto pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
}

/// Computes pairwise similarity matrices across a corpus of documents.
/// Uses symmetric optimization: only computes (i,j) once, then sets (j,i) = same value.
/// Diagonal is always 100.0%.
class CorpusComparer {
public:
    static SimilarityMatrix compareCorpus(const std::vector<TextDocument> &docs, size_t ngramSize = 3) {
        size_t n = docs.size();
        SimilarityMatrix mat;
        mat.documentNames.reserve(n);
        mat.jaccardMatrix.assign(n, std::vector<double>(n, 0.0));
        mat.ngramMatrix.assign(n, std::vector<double>(n, 0.0));

        for (size_t i = 0; i < n; ++i) {
            mat.documentNames.push_back(pathBasename(docs[i].filename()));
            mat.jaccardMatrix[i][i] = 100.0;
            mat.ngramMatrix[i][i] = 100.0;
        }

        // Symmetric pairwise comparison — O(N*(N-1)/2) pairs
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = i + 1; j < n; ++j) {
                double jac = PlagiarismChecker::jaccardPercentage(docs[i].content(), docs[j].content());
                double ngr = PlagiarismChecker::ngramPercentage(docs[i].content(), docs[j].content(), ngramSize);
                mat.jaccardMatrix[i][j] = jac;
                mat.jaccardMatrix[j][i] = jac;
                mat.ngramMatrix[i][j] = ngr;
                mat.ngramMatrix[j][i] = ngr;
            }
        }

        return mat;
    }
};
