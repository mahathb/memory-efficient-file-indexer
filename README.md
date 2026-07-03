# 240252_MAHATH.md

## Memory-Efficient Versioned File Indexer

### Project Overview
This program implements a memory-efficient versioned file indexer that processes large text files using a fixed-size buffer (256KB - 1024KB). It builds word-level indexes and supports three types of queries: word count, difference between versions, and top-K frequent words.

### Design Architecture

#### Class Structure (6 User-Defined Classes)
1. **QueryProcessor** (Abstract Base Class)
   - Pure virtual functions: execute(), displayResult()
   - Derived classes: WordCountQuery, DiffQuery, TopKQuery

2. **BufferedFileReader**
   - Manages file reading with fixed-size buffer
   - Handles buffer refilling automatically

3. **Tokenizer**
   - Extracts words from buffer content
   - Handles split tokens across buffer boundaries
   - Case-insensitive word processing

4. **VersionedIndex**
   - Stores word frequencies for a specific version
   - Uses unordered_map for O(1) lookups

5. **FileIndexer**
   - Coordinates indexing process
   - Manages multiple versions

6. **ArgumentParser**
   - Parses command-line arguments
   - Validates input parameters

#### Helper Classes
- **IndexEntry<T>** (Template class)
- **StringUtils** (Demonstrates function overloading)

### Key Features Implemented

#### Memory Efficiency
- Fixed-size buffer (256KB - 1024KB)
- Streams file content without loading entire file
- Memory usage grows only with unique words count

#### Version Management
- Supports multiple versions in same execution
- Separate index per version
- Version identification via user-provided names

#### Query Support
1. **Word Count Query**: Frequency of specific word
2. **Difference Query**: Compare word frequencies across versions
3. **Top-K Query**: Most frequent words in descending order

### C++ Requirements Satisfied

1. **Object-Oriented Design**: 6 user-defined classes with clear responsibilities
2. **Inheritance**: QueryProcessor as abstract base class with 3 derived classes
3. **Runtime Polymorphism**: Virtual functions in QueryProcessor hierarchy
4. **Function Overloading**: StringUtils::toLower() overloaded for string and char*
5. **Exception Handling**: Try-catch blocks throughout for error management
6. **Templates**: IndexEntry<T> class template

### Compilation Instructions
```bash
g++ -std=c++11 -o analyzer 240252_MAHATH.cpp
