#include <algorithm>
#include <cctype>
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

using namespace std;

struct SearchSettings {
    bool ignoreCase = true;
    bool wholeWord = false;          // pattern ko puray word ki tarhan treat kareyga
    bool verifyMatches = true;       // verify karega matches ko with direct comparison
    size_t ngramSize = 3;            // for n-gram plagiarism check
    string algorithm = "rabin";};      // default algorithm rabin-karp hai

class SearchResult {
public:
    vector<size_t> positions;
    size_t frequency() const { return positions.size(); }
    void add(size_t pos) { positions.push_back(pos); }};

/* ----------------------------
   Tokenizer & NGram generator
   ---------------------------- */

class Tokenizer {
public:
    // minimal normalization: lower-case (optionally) aur punctuation hata dega
    static vector<string> tokenize(const string &text, bool lower = true) {
        vector<string> tokens;
        string cur;
        for (char ch : text) {
            if (isalnum(static_cast<unsigned char>(ch))) {
                cur.push_back(lower ? (char)tolower(static_cast<unsigned char>(ch)) : ch);
            } else {
                if (!cur.empty()) {
                    tokens.push_back(cur);
                    cur.clear();}}}
        if (!cur.empty()) tokens.push_back(cur);
        return tokens;}};

class NGramGenerator {
public:
    static unordered_set<string> genNGrams(const vector<string> &tokens, size_t n) {
        unordered_set<string> s;
        if (n == 0) return s;
        if (tokens.size() < n) return s;
        for (size_t i = 0; i + n <= tokens.size(); ++i) {
            string g = tokens[i];
            for (size_t j = 1; j < n; ++j) {
                g += '\x1F'; // use a rarely-used separator char
                g += tokens[i + j];}
            s.insert(move(g));}
        return s;}
    static unordered_set<string> genNGramsFromText(const string &text, size_t n, bool lower = true) {
        auto t = Tokenizer::tokenize(text, lower);
        return genNGrams(t, n);}};

/* ----------------------------
   TextDocument: load + metadata
   ---------------------------- */

class TextDocument {
    string filename_;
    string content_;
public:
    TextDocument() = default;
    explicit TextDocument(const string &filename) { loadFromFile(filename); }
    void loadFromFile(const string &filename) {
        ifstream in(filename, ios::in | ios::binary);
        if (!in) throw runtime_error("Failed to open file: " + filename);
        ostringstream ss;
        ss << in.rdbuf();
        content_ = ss.str();
        filename_ = filename;}
    const string &content() const noexcept { return content_; }
    const string &filename() const noexcept { return filename_; }

    size_t wordCount() const {return Tokenizer::tokenize(content_).size();}

    // quick accessor for tokens
    vector<string> tokens(bool lower = true) const {
        return Tokenizer::tokenize(content_, lower);}};

/* ----------------------------
   Indexer (simple inverted index)
   ---------------------------- */

class TextIndexer {
    unordered_map<string, vector<size_t>> index_;
public:
    // build index of whole-word positions (token start indices in characters)
    void build(const string &text, bool lower = true) {
        index_.clear();
        string cur;
        size_t pos = 0;
        size_t tokenStart = string::npos;
        for (size_t i = 0; i < text.size(); ++i) {
            char ch = text[i];
            if (isalnum(static_cast<unsigned char>(ch))) {
                if (tokenStart == string::npos) tokenStart = i;
                cur.push_back(lower ? (char)tolower(static_cast<unsigned char>(ch)) : ch);
            } else {
                if (!cur.empty()) {
                    index_[cur].push_back(tokenStart);
                    cur.clear();}
                tokenStart = string::npos;}}
        if (!cur.empty()) index_[cur].push_back(tokenStart);}

    const vector<size_t> &positionsFor(const string &word) const {
        static const vector<size_t> empty;
        auto it = index_.find(word);
        if (it == index_.end()) return empty;
        return it->second;}};

/* ----------------------------
   Strategy interface & implementations
   ---------------------------- */

class IHashStrategy {
public:
    virtual ~IHashStrategy() = default;
    // search pattern inside text, respecting settings where possible
    virtual SearchResult search(const string &text, const string &pattern, const SearchSettings &settings) = 0;};

/* Naive search (brute force) */
class NaiveStrategy : public IHashStrategy {
public:
    SearchResult search(const string &text, const string &pattern, const SearchSettings &settings) override {
        if (pattern.empty()) throw invalid_argument("Pattern must not be empty.");
        string t = text;
        string p = pattern;
        if (settings.ignoreCase) {
            transform(t.begin(), t.end(), t.begin(), [](unsigned char c){ return (char)tolower(c); });
            transform(p.begin(), p.end(), p.begin(), [](unsigned char c){ return (char)tolower(c); });}
        SearchResult res;
        for (size_t i = 0; i + p.size() <= t.size(); ++i) {
            bool ok = true;
            for (size_t j = 0; j < p.size(); ++j)
                if (t[i + j] != p[j]) { ok = false; break; }
            if (ok) {
                // For whole-word, check boundaries
                if (settings.wholeWord) {
                    bool left_ok = (i == 0) || !isalnum(static_cast<unsigned char>(t[i-1]));
                    bool right_ok = (i + p.size() >= t.size()) || !isalnum(static_cast<unsigned char>(t[i + p.size()]));
                    if (!left_ok || !right_ok) continue;}
                res.add(i);}}
        return res;}};

/* KMP search */
class KMPStrategy : public IHashStrategy {
    static vector<int> buildPrefix(const string &p) {
        int n = (int)p.size();
        vector<int> pi(n);
        pi[0] = 0;
        for (int i = 1; i < n; ++i) {
            int j = pi[i-1];
            while (j > 0 && p[i] != p[j]) j = pi[j-1];
            if (p[i] == p[j]) ++j;
            pi[i] = j;}
        return pi;}

public:
    SearchResult search(const string &text, const string &pattern, const SearchSettings &settings) override {
        if (pattern.empty()) throw invalid_argument("Pattern must not be empty.");
        string t = text;
        string p = pattern;
        if (settings.ignoreCase) {
            transform(t.begin(), t.end(), t.begin(), [](unsigned char c){ return (char)tolower(c); });
            transform(p.begin(), p.end(), p.begin(), [](unsigned char c){ return (char)tolower(c); });}
        SearchResult res;
        vector<int> pi = buildPrefix(p);
        int j = 0;
        for (int i = 0; i < (int)t.size(); ++i) {
            while (j > 0 && t[i] != p[j]) j = pi[j-1];
            if (t[i] == p[j]) ++j;
            if (j == (int)p.size()) {
                int start = i - (int)p.size() + 1;
                if (settings.wholeWord) {
                    bool left_ok = (start == 0) || !isalnum(static_cast<unsigned char>(t[start-1]));
                    bool right_ok = (start + p.size() >= t.size()) || !isalnum(static_cast<unsigned char>(t[start + p.size()]));
                    if (!left_ok || !right_ok) { j = pi[j-1]; continue; }}
                res.add(start);
                j = pi[j-1];}}
        return res;}};

/* Rabin-Karp strategy using 64-bit rolling hash (less mod hassle) */
class RabinKarpStrategy : public IHashStrategy {
    static constexpr uint64_t base = 1315423911ULL; // arbitrary large base (we'll use simpler approach)
public:
    SearchResult search(const string &text, const string &pattern, const SearchSettings &settings) override {
        if (pattern.empty()) throw invalid_argument("Pattern must not be empty.");
        string t = text;
        string p = pattern;
        if (settings.ignoreCase) {
            transform(t.begin(), t.end(), t.begin(), [](unsigned char c){ return (char)tolower(c); });
            transform(p.begin(), p.end(), p.begin(), [](unsigned char c){ return (char)tolower(c); });}
        size_t n = t.size(), m = p.size();
        SearchResult res;
        if (m > n) return res;

        // simple polynomial rolling hash with uint64_t (wrap-around acceptable)
        const uint64_t B = 1315423911ULL; // chosen base (we will reduce risk by using 64-bit)
        uint64_t power = 1;
        for (size_t i = 0; i < m - 1; ++i) power = power * B;

        auto hashOf = [&](const string &s, size_t start, size_t len) {
            uint64_t h = 0;
            for (size_t i = 0; i < len; ++i) h = h * B + static_cast<unsigned char>(s[start + i]) + 1;
            return h;};

        uint64_t patHash = hashOf(p, 0, m);
        uint64_t curHash = hashOf(t, 0, m);
        if (curHash == patHash) {
            if (!settings.wholeWord || ((0==0 || !isalnum(static_cast<unsigned char>(t[0-1]))) && ((m >= n) || !isalnum(static_cast<unsigned char>(t[m])))))
                res.add(0);}

        for (size_t i = 1; i + m <= n; ++i) {
            
            uint64_t left = (static_cast<uint64_t>(static_cast<unsigned char>(t[i-1])) + 1);
            curHash = hashOf(t, i, m);

            if (curHash == patHash) {
                if (settings.wholeWord) {
                    bool left_ok = (i == 0) || !isalnum(static_cast<unsigned char>(t[i-1]));
                    bool right_ok = (i + m >= n) || !isalnum(static_cast<unsigned char>(t[i + m]));
                    if (!left_ok || !right_ok) continue;}
                res.add(i);}}

        return res;}};

/* Factory for creating strategies by name */
class HashStrategyFactory {
public:
    static unique_ptr<IHashStrategy> create(const string &name) {
        string n = name;
        transform(n.begin(), n.end(), n.begin(), [](unsigned char c){ return (char)tolower(c); });
        if (n == "rabin" || n == "rabin-karp" || n == "rk") {
            return make_unique<RabinKarpStrategy>();
        } else if (n == "kmp") {
            return make_unique<KMPStrategy>();
        } else if (n == "naive" || n == "brute") {
            return make_unique<NaiveStrategy>();
        } else {
            throw invalid_argument("Unknown algorithm: " + name);}}};

/* ----------------------------
   SearchEngine
   ---------------------------- */

class SearchEngine {
    unique_ptr<IHashStrategy> strategy_;
public:
    explicit SearchEngine(unique_ptr<IHashStrategy> strat) : strategy_(move(strat)) {}
    SearchResult search(const TextDocument &doc, const string &pattern, const SearchSettings &settings) {
        if (!strategy_) throw runtime_error("Search strategy not set.");
        // If wholeWord requested, we can use index for speed in future; for now call strategy
        return strategy_->search(doc.content(), pattern, settings);}};

/* ----------------------------
   PlagiarismChecker
   ---------------------------- */

class PlagiarismChecker {
public:
    // Jaccard on tokens
    static double jaccardPercentage(const string &text, const string &pattern, bool lower = true) {
        auto ta = Tokenizer::tokenize(text, lower);
        auto pa = Tokenizer::tokenize(pattern, lower);
        unordered_set<string> ts(ta.begin(), ta.end()), ps(pa.begin(), pa.end());
        if (ps.empty()) return 0.0;
        size_t inter = 0;
        for (const auto &w : ps) if (ts.count(w)) ++inter;
        size_t uni = ts.size() + ps.size() - inter;
        if (uni == 0) return 0.0;
        return (double(inter) / double(uni)) * 100.0;}

    // n-gram overlap (set-based)
    static double ngramPercentage(const string &text, const string &pattern, size_t n = 3, bool lower = true) {
        if (n == 0) return 0.0;
        auto tg = NGramGenerator::genNGramsFromText(text, n, lower);
        auto pg = NGramGenerator::genNGramsFromText(pattern, n, lower);
        if (pg.empty()) return 0.0;
        size_t inter = 0;
        for (const auto &g : pg) if (tg.count(g)) ++inter;
        size_t uni = tg.size() + pg.size() - inter;
        if (uni == 0) return 0.0;
        return (double(inter) / double(uni)) * 100.0;}};

/* ----------------------------
   DocumentComparer: compare two docs
   ---------------------------- */

class DocumentComparer {
public:
    struct Result {
        double jaccard;
        double ngram;};

    static Result compare(const TextDocument &a, const TextDocument &b, size_t ngramSize = 3) {
        double j = PlagiarismChecker::jaccardPercentage(a.content(), b.content());
        double ng = PlagiarismChecker::ngramPercentage(a.content(), b.content(), ngramSize);
        return { j, ng };}};

/* ----------------------------
   OutputWriter
   ---------------------------- */

class OutputWriter {
public:
    static void writeSearchReport(const string &outFile,
                                  const TextDocument &doc,
                                  const string &pattern,
                                  const SearchSettings &settings,
                                  const SearchResult &result,
                                  double jaccardPercent,
                                  double ngramPercent) {
        ofstream out(outFile);
        if (!out) throw runtime_error("Failed to open output file: " + outFile);
        out << "File: " << doc.filename() << "\n";
        out << "Pattern: " << pattern << "\n";
        out << "Algorithm: " << settings.algorithm << "\n";
        out << "Ignore case: " << (settings.ignoreCase ? "yes" : "no") << "\n";
        out << "Whole word: " << (settings.wholeWord ? "yes" : "no") << "\n\n";

        out << "Occurrences (count = " << result.frequency() << "): ";
        if (result.positions.empty()) out << "none\n";
        else {
            for (size_t p : result.positions) out << p << " ";
            out << "\n";}

        out << "\nPlagiarism (similarity) metrics:\n";
        out << " - Jaccard token overlap: " << jaccardPercent << "%\n";
        out << " - " << settings.ngramSize << "-gram overlap: " << ngramPercent << "%\n";
        out.close();}};

/* ----------------------------
   Example CLI main
   ---------------------------- */

int main() {
    try {
        cout << "=== Advanced Search & Plagiarism Tool ===\n";

        string filename;
        cout << "Enter path to text file: ";
        getline(cin, filename);
        if (filename.empty()) { cerr << "No filename provided.\n"; return 1; }

        TextDocument doc;
        doc.loadFromFile(filename);
        cout << "Loaded '" << doc.filename() << "' (approx " << doc.wordCount() << " words).\n";
        string pattern;
        cout << "Enter pattern to search: ";
        getline(cin, pattern);
        if (pattern.empty()) { cerr << "Pattern cannot be empty.\n"; return 1; }

        SearchSettings settings;
        cout << "Choose algorithm [rabin/kmp/naive] (default rabin): ";
        string alg; getline(cin, alg);
        if (!alg.empty()) settings.algorithm = alg;
        cout << "Ignore case? [y/n] (default y): ";
        string s; getline(cin, s);
        if (!s.empty()) settings.ignoreCase = (s == "y" || s == "Y");
        cout << "Whole-word search? [y/n] (default n): ";
        getline(cin, s);
        if (!s.empty()) settings.wholeWord = (s == "y" || s == "Y");
        cout << "Verify matches with exact compare? [y/n] (default y): ";
        getline(cin, s);
        if (!s.empty()) settings.verifyMatches = (s == "y" || s == "Y");
        cout << "n-gram size for similarity (default 3): ";
        getline(cin, s);
        if (!s.empty()) {
            try { settings.ngramSize = max<size_t>(1, stoul(s)); } catch(...) { settings.ngramSize = 3; }}

        // Create strategy
        unique_ptr<IHashStrategy> strat = HashStrategyFactory::create(settings.algorithm);
        SearchEngine engine(move(strat));

        // Perform search
        SearchResult res = engine.search(doc, pattern, settings);

        // Optionally verify matches (if verifyMatches enabled and algorithm didn't verify)
        if (settings.verifyMatches && !res.positions.empty()) {
            vector<size_t> verified;
            string text = doc.content();
            string pat = pattern;
            if (settings.ignoreCase) {
                string tc = text; transform(tc.begin(), tc.end(), tc.begin(), [](unsigned char c){ return (char)tolower(c); });
                string pc = pat; transform(pc.begin(), pc.end(), pc.begin(), [](unsigned char c){ return (char)tolower(c); });
                for (size_t pos : res.positions) {
                    if (pos + pc.size() <= tc.size() && tc.compare(pos, pc.size(), pc) == 0) verified.push_back(pos);}
            } else {
                for (size_t pos : res.positions) {
                    if (pos + pat.size() <= text.size() && text.compare(pos, pat.size(), pat) == 0) verified.push_back(pos);}}
            res.positions.swap(verified);}

        // Compute plagiarism/similarity metrics between whole file and pattern
        double jacc = PlagiarismChecker::jaccardPercentage(doc.content(), pattern, settings.ignoreCase);
        double ng = PlagiarismChecker::ngramPercentage(doc.content(), pattern, settings.ngramSize, settings.ignoreCase);

        // Output
        string outFile = "search_output.txt";
        OutputWriter::writeSearchReport(outFile, doc, pattern, settings, res, jacc, ng);
        cout << "Search complete. Results saved to " << outFile << "\n";

        // Extra: ask if user wants to compare with another file
        cout << "Would you like to compare this file with another document for similarity? [y/n]: ";
        getline(cin, s);
        if (s == "y" || s == "Y") {
            cout << "Enter path to second file: ";
            string file2; getline(cin, file2);
            try {
                TextDocument doc2(file2);
                auto comp = DocumentComparer::compare(doc, doc2, settings.ngramSize);
                cout << "Comparison results:\n";
                cout << " - Jaccard token overlap: " << comp.jaccard << "%\n";
                cout << " - " << settings.ngramSize << "-gram overlap: " << comp.ngram << "%\n";
            } catch (const exception &e) {
                cerr << "Failed to load second file: " << e.what() << "\n";}}
        return 0;
    } catch (const exception &ex) {
        cerr << "Error: " << ex.what() << "\n";
        return 1;}}
