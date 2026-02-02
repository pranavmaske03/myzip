#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <queue>
#include <array>

namespace fs = std::filesystem;

// Error handling
struct CompressionError : std::runtime_error {
    using std::runtime_error::runtime_error;
};
struct FileError : CompressionError {
    using CompressionError::CompressionError;
};
struct DataError : CompressionError {
    using CompressionError::CompressionError;
};

class Compression 
{
    private:
        struct Node {
            char ch;
            int freq;
            Node* left;
            Node* right;

            Node(char c, int f): ch(c), freq(f), left(nullptr), right(nullptr) {}
            Node(int f, Node* l, Node* r): ch('\0'), freq(f), left(l), right(r) {}
        };
        struct Compare {
            bool operator() (Node* a, Node* b) {
                return a->freq > b->freq;
            }
        };

        const std::string filePath;
        const std::string fileName;
        std::unordered_map<char, int> freq;
        std::unordered_map<char, std::string> codes;
        int paddingSize;
        Node* root;

        void buildCodes(Node* node, const std::string& path);
        uint8_t makeByte(const std::string& bits);
        void writeOutput(const std::vector<uint8_t>& buffer);
        void deleteTree(Node* node);

    public:
        Compression(const std::string& fp, const std::string& fn);
        ~Compression();

        void frequencyCount();
        void buildHuffmanTree();
        void generateCodes();
        void encodeData();
};

Compression::Compression(const std::string& fp, const std::string& fn): filePath(fp), fileName(fn) 
{
    root = nullptr;
    paddingSize = 0;
}

Compression::~Compression() 
{
    deleteTree(root);
}

void Compression::buildCodes(Node* node, const std::string& path) 
{
    if(node == nullptr) return;

    if(!node->left && !node->right) {
        codes[node->ch] = path;
    }

    buildCodes(node->left, path + "0");
    buildCodes(node->right, path + "1");
}

uint8_t Compression::makeByte(const std::string& bits) 
{ 
    uint8_t byte = 0;
    for(size_t i = 0; i < 8 && i < bits.size(); i++) {
        if(bits[i] == '1') {
            byte |= (1 << (7 - i));
        }
    }
    return byte;
}

void Compression::writeOutput(const std::vector<uint8_t>& buffer) 
{
   fs::create_directories("../Storage/Compressed");
    std::string outputPath = "../Storage/Compressed/" + fileName + ".bin";

    std::ofstream out(outputPath, std::ios::binary);
    if(!out) {
        throw FileError("Cannot open output file: " + outputPath);
    }

    out.write(
        reinterpret_cast<const char*>(buffer.data()),
        static_cast<std::streamsize>(buffer.size())
    );

    if (!out) {
        throw FileError("Failed to write output file: " + outputPath);
    } 
}

void Compression::deleteTree(Node* node) 
{
    if(node == nullptr) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

void Compression::frequencyCount() 
{
    std::ifstream file(filePath, std::ios::binary);
    if(!file) {
        throw FileError("Cannot open the input file. "+filePath);
    }
    char buffer[1 << 20]; // 1 MB size buffer

    while(file.read(buffer, sizeof(buffer)) || file.gcount()) {
        for(size_t i = 0; i < static_cast<size_t>(file.gcount()); ++i) {
            freq[(unsigned char)buffer[i]]++;
        }
    }
     for(const auto& [ch, count] : freq) {
                std::cout << "Key: " << ch << "\tValue: " << count << '\n';
            }
    if(freq.empty()) {
        throw DataError("Input file is empty.");
    }
}

void Compression::buildHuffmanTree() 
{
    std::priority_queue<Node*, std::vector<Node*>, Compare> pq;

    for(int i = 0; i < 256; ++i) {
        if(freq[i] > 0) {
            pq.push(new Node((char)i, freq[i]));
        }
    }
    if(pq.empty()) {
        throw DataError("No symbols to build Huffman tree");    
    }

    while(pq.size() > 1) {
        Node* left = pq.top(); pq.pop();
        Node* right = pq.top(); pq.pop();

        Node* parent = new Node(left->freq + right->freq, left, right);
        pq.push(parent);
    }
    root = pq.top();
}

void Compression::generateCodes() 
{
    if(!root) {
        throw DataError("Null Huffman tree.\nCodes cannot be generated.");
    }

    codes.clear(); 
    buildCodes(root, "");

    if(codes.size() == 1) {
        codes.begin()->second = "0";
    }
}

void Compression::encodeData() 
{
    std::ifstream file(filePath, std::ios::binary);
    if(!file) {
        throw FileError("Cannot open input file for encoding.\n");
    }          
    std::vector<uint8_t> output;
    std::string bitBuffer;
    char chunk[1 << 20]; // read 1MB data per read call;

    while(file.read(chunk, sizeof(chunk)) || file.gcount()) {
        for(size_t i = 0; i < static_cast<size_t>(file.gcount()); i++) {
            auto it = codes.find(chunk[i]);
            if(it == codes.end()) {
                throw DataError("Missing Huffman Code.\n");
            }

            bitBuffer += it->second;

            while(bitBuffer.size() >= 8) {
                output.push_back(makeByte(bitBuffer.substr(0, 8)));
                bitBuffer.erase(0, 8);
            }
        }
    }
    if(file.bad()) {
        throw FileError("I/O error while reading input file.\n");
    }   
    if(!bitBuffer.empty()) {
        paddingSize = 8 - bitBuffer.size();
        output.push_back(makeByte(bitBuffer));
    }

    writeOutput(output);
}

int main(int argc, char* argv[])
{
    try {
        if(argc != 2) {
            std::cerr<<"Error: Wrong command line arguments.\n";
            std::cerr<<"Usage: compressor <input_file>\n";
            return EXIT_FAILURE;
        }

        fs::path defaultDir = "../Storage/Input/";
        fs::path inputPath(argv[1]);

        if(inputPath.has_filename() && !inputPath.has_parent_path()) {
            inputPath = defaultDir / inputPath;
        }

        if(!fs::exists(inputPath)) {
            throw FileError("Input file does not exist: " + inputPath.string());
        }

        Compression compressor(inputPath.string(), inputPath.stem().string());

        compressor.frequencyCount();
        compressor.buildHuffmanTree();
        compressor.generateCodes();
        compressor.encodeData();

        std::cout<<"Compression completed successfully.\n";
        return EXIT_SUCCESS;
    }
    catch(const FileError& e) {
        std::cerr<<"File error: "<< e.what()<<'\n';
    }
    catch(const DataError& e) {
        std::cerr<<"Data error: "<< e.what()<<'\n';
    }
    catch(const std::exception& e) {
        std::cerr<<"Unexpected error: "<< e.what()<<'\n';
    }
    return EXIT_FAILURE;
}