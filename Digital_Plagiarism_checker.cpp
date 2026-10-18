#include <iostream>
#include <string>
#include "PlagiarismCore.hpp"

using namespace std;

int main() {
    try {
        cout << "=== Advanced Search & Plagiarism Tool ===\n";
        string filename;
        cout << "Enter path to text file: ";
        getline(cin, filename);
        if (filename.empty()) {
            cerr << "No filename provided.\n";
            return 1;
        }

        TextDocument doc;
        doc.loadFromFile(filename);
        cout << "Loaded '" << doc.filename() << "' (approx " << doc.wordCount() << " words).\n";

        string pattern;
        cout << "Enter pattern to search: ";
        getline(cin, pattern);
        if (pattern.empty()) {
            cerr << "Pattern cannot be empty.\n";
            return 1;
        }

        SearchSettings settings;
        cout << "Choose algorithm [rabin/kmp/naive] (default rabin): ";
        string alg;
        getline(cin, alg);
        if (!alg.empty()) settings.algorithm = alg;

        cout << "Ignore case? [y/n] (default y): ";
        string s;
        getline(cin, s);
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
            try {
                settings.ngramSize = max<size_t>(1, stoul(s));
            } catch (...) {
                settings.ngramSize = 3;
            }
        }

        unique_ptr<IHashStrategy> strat = HashStrategyFactory::create(settings.algorithm);
        SearchEngine engine(move(strat));
        SearchResult res = engine.search(doc, pattern, settings);

        if (settings.verifyMatches && !res.positions.empty()) {
            vector<size_t> verified;
            string text = doc.content();
            string pat = pattern;
            if (settings.ignoreCase) {
                string tc = text;
                transform(tc.begin(), tc.end(), tc.begin(), [](unsigned char c){ return static_cast<char>(tolower(c)); });
                string pc = pat;
                transform(pc.begin(), pc.end(), pc.begin(), [](unsigned char c){ return static_cast<char>(tolower(c)); });
                for (size_t pos : res.positions) {
                    if (pos + pc.size() <= tc.size() && tc.compare(pos, pc.size(), pc) == 0) verified.push_back(pos);
                }
            } else {
                for (size_t pos : res.positions) {
                    if (pos + pat.size() <= text.size() && text.compare(pos, pat.size(), pat) == 0) verified.push_back(pos);
                }
            }
            res.positions.swap(verified);
        }

        double jacc = PlagiarismChecker::jaccardPercentage(doc.content(), pattern, settings.ignoreCase);
        double ng = PlagiarismChecker::ngramPercentage(doc.content(), pattern, settings.ngramSize, settings.ignoreCase);

        string outFile = "search_output.txt";
        OutputWriter::writeSearchReport(outFile, doc, pattern, settings, res, jacc, ng);
        cout << "Search complete. Results saved to " << outFile << "\n";

        cout << "Would you like to compare this file with another document for similarity? [y/n]: ";
        getline(cin, s);
        if (s == "y" || s == "Y") {
            cout << "Enter path to second file: ";
            string file2;
            getline(cin, file2);
            try {
                TextDocument doc2(file2);
                auto comp = DocumentComparer::compare(doc, doc2, settings.ngramSize);
                cout << "Comparison results:\n";
                cout << " - Jaccard token overlap: " << comp.jaccard << "%\n";
                cout << " - " << settings.ngramSize << "-gram overlap: " << comp.ngram << "%\n";
            } catch (const exception &e) {
                cerr << "Failed to load second file: " << e.what() << "\n";
            }
        }
        return 0;
    } catch (const exception &ex) {
        cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}
