// 240252_MAHATH.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cctype>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <cstring>

// Abstract base class for query operations
class QueryProcessor {
protected:
    std::string version;
    
public:
    QueryProcessor(const std::string& ver) : version(ver) {}
    virtual ~QueryProcessor() = default;
    
    virtual void execute() = 0;
    virtual void displayResult() const = 0;
    
    std::string getVersion() const { return version; }
};

// Template class for handling different data types in index
template<typename T>
class IndexEntry {
private:
    T value;
    size_t frequency;
    
public:
    IndexEntry(const T& val, size_t freq = 0) : value(val), frequency(freq) {}
    
    void increment() { frequency++; }
    size_t getFrequency() const { return frequency; }
    T getValue() const { return value; }
};

// Buffered file reader class
class BufferedFileReader {
private:
    std::ifstream file;
    char* buffer;
    size_t bufferSize;
    size_t bufferPosition;
    size_t bytesInBuffer;
    bool endOfFile;
    
public:
    BufferedFileReader(const std::string& filename, size_t size) 
        : bufferSize(size), bufferPosition(0), bytesInBuffer(0), endOfFile(false) {
        buffer = new char[bufferSize];
        file.open(filename, std::ios::binary);
        if (!file.is_open()) {
            delete[] buffer;
            throw std::runtime_error("Failed to open file: " + filename);
        }
        fillBuffer();
    }
    
    ~BufferedFileReader() {
        delete[] buffer;
        if (file.is_open()) {
            file.close();
        }
    }
    
    bool fillBuffer() {
        if (endOfFile) return false;
        
        file.read(buffer, bufferSize);
        bytesInBuffer = file.gcount();
        bufferPosition = 0;
        
        if (bytesInBuffer == 0) {
            endOfFile = true;
            return false;
        }
        return true;
    }
    
    char getNextChar() {
        if (bufferPosition >= bytesInBuffer) {
            if (!fillBuffer()) {
                return '\0';
            }
        }
        return buffer[bufferPosition++];
    }
    
    bool isEOF() const { return endOfFile && bufferPosition >= bytesInBuffer; }
    size_t getBufferSize() const { return bufferSize; }
};

// Tokenizer class
class Tokenizer {
private:
    std::string leftover;
    
    bool isAlphanumeric(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
    }
    
    char toLower(char c) {
        if (c >= 'A' && c <= 'Z')
            return c + ('a' - 'A');
        return c;
    }
    
public:
    std::vector<std::string> tokenize(BufferedFileReader& reader) {
        std::vector<std::string> tokens;
        std::string currentToken = leftover;
        leftover.clear();
        
        char c;
        bool inToken = !currentToken.empty();
        
        while (!reader.isEOF()) {
            c = reader.getNextChar();
            if (c == '\0') break;
            
            if (isAlphanumeric(c)) {
                currentToken += toLower(c);
                inToken = true;
            } else {
                if (inToken && !currentToken.empty()) {
                    tokens.push_back(currentToken);
                    currentToken.clear();
                    inToken = false;
                }
            }
        }
        
        // Handle remaining token
        if (!currentToken.empty()) {
            leftover = currentToken;
        }
        
        return tokens;
    }
    
    std::string getLeftover() const { return leftover; }
};

// Overloaded functions example
class StringUtils {
public:
    static std::string toLower(const std::string& str) {
        std::string result;
        for (char c : str) {
            if (c >= 'A' && c <= 'Z')
                result += c + ('a' - 'A');
            else
                result += c;
        }
        return result;
    }
    
    // Overloaded version for char arrays
    static std::string toLower(const char* str) {
        return toLower(std::string(str));
    }
};

// Versioned index class
class VersionedIndex {
private:
    std::string version;
    std::unordered_map<std::string, size_t> wordFrequency;
    
public:
    VersionedIndex(const std::string& ver) : version(ver) {}
    
    void addWord(const std::string& word) {
        wordFrequency[word]++;
    }
    
    void mergeTokens(const std::vector<std::string>& tokens) {
        for (size_t i = 0; i < tokens.size(); ++i) {
            addWord(tokens[i]);
        }
    }
    
    size_t getWordFrequency(const std::string& word) const {
        std::string lowerWord = StringUtils::toLower(word);
        auto it = wordFrequency.find(lowerWord);
        return (it != wordFrequency.end()) ? it->second : 0;
    }
    
    std::string getVersion() const { return version; }
    
    std::vector<std::pair<std::string, size_t> > getTopK(size_t k) const {
        std::vector<std::pair<std::string, size_t> > words;
        for (const auto& entry : wordFrequency) {
            words.push_back(entry);
        }
        
        // Custom sort using function object (C++11 compatible)
        struct {
            bool operator()(const std::pair<std::string, size_t>& a, 
                           const std::pair<std::string, size_t>& b) const {
                if (a.second != b.second) {
                    return a.second > b.second;
                }
                return a.first < b.first;
            }
        } customComparator;
        
        std::sort(words.begin(), words.end(), customComparator);
        
        if (words.size() > k) {
            words.resize(k);
        }
        return words;
    }
    
    // C++11 compatible version - explicitly specifying return type
    const std::unordered_map<std::string, size_t>& getAllFrequencies() const { 
        return wordFrequency; 
    }
};

// Derived query classes
class WordCountQuery : public QueryProcessor {
private:
    VersionedIndex& index;
    std::string word;
    size_t result;
    
public:
    WordCountQuery(const std::string& ver, VersionedIndex& idx, const std::string& w)
        : QueryProcessor(ver), index(idx), word(w), result(0) {}
    
    void execute() override {
        result = index.getWordFrequency(word);
    }
    
    void displayResult() const override {
        std::cout << "Version: " << version << std::endl;
        std::cout << "Word: '" << word << "' appears " << result << " times" << std::endl;
    }
};

class DiffQuery : public QueryProcessor {
private:
    VersionedIndex& index1;
    VersionedIndex& index2;
    std::string word;
    size_t result1;
    size_t result2;
    std::string version2;
    
public:
    DiffQuery(const std::string& ver, VersionedIndex& idx1, 
              const std::string& ver2, VersionedIndex& idx2, const std::string& w)
        : QueryProcessor(ver), index1(idx1), index2(idx2), word(w), 
          result1(0), result2(0), version2(ver2) {}
    
    void execute() override {
        result1 = index1.getWordFrequency(word);
        result2 = index2.getWordFrequency(word);
    }
    
    void displayResult() const override {
        std::cout << "Version " << version << ": " << result1 << " times" << std::endl;
        std::cout << "Version " << version2 << ": " << result2 << " times" << std::endl;
        long diff = 0;
        if (result1 > result2) {
            diff = result1 - result2;
        } else {
            diff = result2 - result1;
        }
        std::cout << "Difference: " << diff << std::endl;
    }
};

class TopKQuery : public QueryProcessor {
private:
    VersionedIndex& index;
    size_t k;
    std::vector<std::pair<std::string, size_t> > results;
    
public:
    TopKQuery(const std::string& ver, VersionedIndex& idx, size_t topK)
        : QueryProcessor(ver), index(idx), k(topK) {}
    
    void execute() override {
        results = index.getTopK(k);
    }
    
    void displayResult() const override {
        std::cout << "Version: " << version << std::endl;
        std::cout << "Top " << k << " words:" << std::endl;
        for (size_t i = 0; i < results.size(); ++i) {
            std::cout << i + 1 << ". " << results[i].first 
                      << " (" << results[i].second << ")" << std::endl;
        }
    }
};

// Main indexer class
class FileIndexer {
private:
    std::unordered_map<std::string, VersionedIndex*> indices;
    size_t bufferSizeKB;
    
public:
    FileIndexer(size_t bufferKB) : bufferSizeKB(bufferKB) {
        if (bufferKB < 256 || bufferKB > 1024) {
            throw std::out_of_range("Buffer size must be between 256 KB and 1024 KB");
        }
    }
    
    ~FileIndexer() {
        // Clean up dynamically allocated indices
        for (auto& pair : indices) {
            delete pair.second;
        }
    }
    
    VersionedIndex& indexFile(const std::string& filename, const std::string& version) {
        auto startTime = std::chrono::high_resolution_clock::now();
        
        try {
            VersionedIndex* index = new VersionedIndex(version);
            BufferedFileReader reader(filename, bufferSizeKB * 1024);
            Tokenizer tokenizer;
            
            while (!reader.isEOF()) {
                std::vector<std::string> tokens = tokenizer.tokenize(reader);
                index->mergeTokens(tokens);
            }
            
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
                           (endTime - startTime);
            
            std::cout << "Indexed file: " << filename << std::endl;
            std::cout << "Buffer size: " << bufferSizeKB << " KB" << std::endl;
            std::cout << "Indexing time: " << duration.count() / 1000.0 << " seconds" << std::endl;
            
            indices[version] = index;
            return *indices[version];
            
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to index file: " + std::string(e.what()));
        }
    }
    
    VersionedIndex* getIndex(const std::string& version) {
        auto it = indices.find(version);
        if (it != indices.end()) {
            return it->second;
        }
        return NULL;
    }
    
    size_t getBufferSize() const { return bufferSizeKB; }
};

// Command line argument parser
class ArgumentParser {
private:
    std::unordered_map<std::string, std::string> args;
    
public:
    ArgumentParser(int argc, char* argv[]) {
        for (int i = 1; i < argc; i += 2) {
            if (i + 1 < argc) {
                std::string key = argv[i];
                std::string value = argv[i + 1];
                args[key] = value;
            }
        }
    }
    
    std::string getString(const std::string& key, const std::string& defaultValue = "") {
        auto it = args.find(key);
        return (it != args.end()) ? it->second : defaultValue;
    }
    
    int getInt(const std::string& key, int defaultValue = 0) {
        auto it = args.find(key);
        if (it != args.end()) {
            try {
                return std::stoi(it->second);
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }
    
    bool hasKey(const std::string& key) {
        return args.find(key) != args.end();
    }
};

// Main function
int main(int argc, char* argv[]) {
    auto programStart = std::chrono::high_resolution_clock::now();
    
    try {
        if (argc < 2) {
            std::cout << "Usage examples:" << std::endl;
            std::cout << "  Word query: ./analyzer --file test.txt --version v1 --buffer 512 --query word --word error" << std::endl;
            std::cout << "  Top-K query: ./analyzer --file test.txt --version v1 --buffer 512 --query top --top 10" << std::endl;
            std::cout << "  Diff query: ./analyzer --file1 v1.txt --version1 v1 --file2 v2.txt --version2 v2 --buffer 512 --query diff --word error" << std::endl;
            return 1;
        }
        
        ArgumentParser parser(argc, argv);
        
        // Parse buffer size
        int bufferKB = parser.getInt("--buffer", 512);
        
        // Create indexer
        FileIndexer indexer(bufferKB);
        
        // Parse query type
        std::string queryType = parser.getString("--query");
        QueryProcessor* query = NULL;
        
        if (queryType == "word") {
            std::string file = parser.getString("--file");
            std::string version = parser.getString("--version");
            std::string word = parser.getString("--word");
            
            if (file.empty() || version.empty() || word.empty()) {
                throw std::runtime_error("Missing required arguments for word query");
            }
            
            VersionedIndex& idx = indexer.indexFile(file, version);
            query = new WordCountQuery(version, idx, word);
            
        } else if (queryType == "diff") {
            std::string file1 = parser.getString("--file1");
            std::string file2 = parser.getString("--file2");
            std::string version1 = parser.getString("--version1");
            std::string version2 = parser.getString("--version2");
            std::string word = parser.getString("--word");
            
            if (file1.empty() || file2.empty() || version1.empty() || 
                version2.empty() || word.empty()) {
                throw std::runtime_error("Missing required arguments for diff query");
            }
            
            VersionedIndex& idx1 = indexer.indexFile(file1, version1);
            VersionedIndex& idx2 = indexer.indexFile(file2, version2);
            query = new DiffQuery(version1, idx1, version2, idx2, word);
            
        } else if (queryType == "top") {
            std::string file = parser.getString("--file");
            std::string version = parser.getString("--version");
            int k = parser.getInt("--top", 10);
            
            if (file.empty() || version.empty() || k <= 0) {
                throw std::runtime_error("Missing required arguments for top query");
            }
            
            VersionedIndex& idx = indexer.indexFile(file, version);
            query = new TopKQuery(version, idx, k);
            
        } else {
            throw std::runtime_error("Invalid query type. Must be 'word', 'diff', or 'top'");
        }
        
        // Execute query
        std::cout << "\n=== Query Results ===" << std::endl;
        query->execute();
        query->displayResult();
        
        // Clean up query
        delete query;
        
        // Display execution time
        auto programEnd = std::chrono::high_resolution_clock::now();
        auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>
                            (programEnd - programStart);
        
        std::cout << "\nAllocated buffer size: " << bufferKB << " KB" << std::endl;
        std::cout << "Total execution time: " << totalDuration.count() / 1000.0 
                  << " seconds" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}