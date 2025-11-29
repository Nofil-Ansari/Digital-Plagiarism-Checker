/* Naive search (brute force) */
/* KMP search */

Class: Tokenizer

Purpose:
Breaks text into words for Jaccard and n-gram processing.

Key behavior:

Removes punctuation

Converts to lowercase (optional)

Stores only alphanumeric sequences

Used for:

Word count

Plagiarism checks

N-gram generation


🔷 Class: TextDocument

Purpose:
Represents a loaded text file + metadata.

Members:

filename_

content_

Key methods:

loadFromFile() — loads entire file into memory

content() — returns full string

tokens() — returns vector of words

wordCount() — number of words

This is a clean abstraction: future features (e.g., metadata, authorship) can plug in easily.

Class: TextIndexer

Purpose:
Builds a simple inverted index:
word → positions in text

This helps whole-word searching.

Maintains:

unordered_map<string, vector<size_t>> index_

Index is built by scanning character-by-character.

Although not used heavily in this version, the class sets foundation for:

fast whole-word search

autocomplete

keyword indexing

Interface: IHashStrategy

Purpose:
Defines the polymorphic search interface for Strategy design pattern.

Method (pure virtual):

virtual SearchResult search(...) = 0;


Every algorithm (Naive, KMP, Rabin-Karp) inherits from this.

This allows runtime selection of search algorithms.

Class: NaiveStrategy (implements IHashStrategy)

Purpose:
Brute-force string matching.

Checks character-by-character for each possible start point.

Features included:

case-insensitive mode

whole-word mode

returns all match positions

Most basic but easiest to understand.



Class: KMPStrategy (implements IHashStrategy)

Purpose:
Efficient string matching using prefix table (O(n + m)).

Key components:

buildPrefix() — builds failure function

search loop that uses prefix table to avoid redundant comparisons

Supports:

ignoreCase

wholeWord



Class: RabinKarpStrategy (implements IHashStrategy)

Purpose:
Uses a rolling hash to detect matches faster.

Key idea:

Compute hash of pattern

Compute hash of each substring of same length

Compare hashes → if equal, consider match

Supports:

ignoreCase

whole-word checking

Note: Uses 64-bit overflow hashing — simple and fast.


Class: HashStrategyFactory

Purpose:
Creates the appropriate search strategy object based on user input.

Uses:

Factory Method pattern

returns unique_ptr<IHashStrategy>

Example:

create("kmp") → returns KMPStrategy


🔷 Class: SearchEngine

Purpose:
Executes a search using a chosen strategy.

Members:

unique_ptr<IHashStrategy> strategy_ — polymorphic behavior

Method:

search(doc, pattern, settings) → delegates to strategy


🔷 Class: PlagiarismChecker

Purpose:
Performs similarity checks between two texts.

Supports:

Jaccard similarity of tokens

N-gram overlap similarity

Both return percentages.

🔷 Class: DocumentComparer

Purpose:
Compares two documents and produces:

Jaccard similarity %

N-gram similarity %

Utility for document-to-document plagiarism.

🔷 Class: OutputWriter

Purpose:
Writes search & similarity results into a .txt report file.

Includes:

file name

pattern

algorithm

settings used

occurrences

plagiarism metrics (Jaccard + N-gram)

Encapsulates file output cleanly.

