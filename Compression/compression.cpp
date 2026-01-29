#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <queue>

using namespace std;
namespace fs = filesystem;

class Compression
{
    public:
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

        unordered_map<char, int> map;
        unordered_map<char,string> codes;
        Node* root;
        long long fileSize = 0;
        int paddingSize = 0;
        const string filePath;
        const string fileName;

        // constructor
        Compression(const string& fp, const string& fn) : filePath(fp), fileName(fn) {}

        bool frequencyCount()
        {
            if(!fs::exists(filePath)) {
                throw runtime_error("The Input file does not exist.");
            }
        
            ifstream file(filePath);
            file.exceptions(ifstream::badbit);

            if(!file.is_open()) {
                throw runtime_error("Unable to read the file :: (permission denied or locked): " + filePath);
            }

            char buffer[1 << 20];  // read 1 MB of data at once
            while (file.read(buffer, sizeof(buffer)) || file.gcount()) {
                size_t n = file.gcount();
                for (size_t i = 0; i < n; ++i) {
                    map[(unsigned char)buffer[i]]++;
                }
            }

            if(!file.eof()) {
                throw runtime_error("Error occurred while reading file :: " + filePath);
            }
            if(map.empty()) {
                throw runtime_error("File is empty.");
            }
            for(const auto& [ch, count] : map) {
                cout << "Key: " << ch << "\tValue: " << count << '\n';
            }
            return true;
        } 
        
        bool buildHuffmanTree()
        {
            priority_queue<Node*,vector<Node*>,Compare> min_heap;

            for(const auto& [key, value]: map) {
                Node* new_node = new Node(key, value);
                min_heap.push(new_node);
            }

            while(min_heap.size() > 1) {
                Node* left = min_heap.top(); min_heap.pop();
                Node* right = min_heap.top(); min_heap.pop();

                Node* new_node = new Node(left->freq + right->freq, left, right);
                min_heap.push(new_node);
            }
            this->root = min_heap.top();

            return (this->root) ? true : false;
        }

        void buildCodes(Node* root, string path)
        {
            if(root == nullptr) return;
            if(!root->left && !root->right) {
                codes[root->ch] = path;
            }

            buildCodes(root->left, path + "0");
            buildCodes(root->right, path + "1");
        }

        bool generateCodes()
        {
            if(!root) {
                throw runtime_error("Null Huffman tree");
            }

            codes.clear(); 
            buildCodes(this->root, "");

            if(codes.empty()) {
                throw runtime_error("Huffman code generation failed");
            }
            if(codes.size() == 1) {
                codes.begin()->second = "0";
            }
            return true;
        }

        bool writeData(vector<uint8_t>& buffer)
        {
            string compressedFilePath = "../Storage/Compressed/" + this->fileName + ".bin";
            ofstream out(compressedFilePath, ios::binary);
            if(!out) {
                throw runtime_error("Failed to open output file.");
            }

            out.write(reinterpret_cast<char*>(buffer.data()), buffer.size());
            if(!out) {
                throw runtime_error("Write failed.");
            }

            out.close();   
            if(!out) {
                throw runtime_error("Error occurred while closing the file.");
            }

            return true;
        }

        uint8_t generateByte(string& code)
        {
            uint8_t byte = 0;
            for (int i = 0; i < 8 && i < code.length(); i++) {
                if (code[i] == '1') {
                    byte |= (1 << (7 - i));
                }
            }
            return byte;
        }

        bool encodeData()
        {
            if(!fs::exists(this->filePath)) {
                throw runtime_error("The Input file does not exist.");
            }
        
            ifstream file(filePath, ios::binary);
            file.exceptions(ifstream::badbit);

            if(!file.is_open()) {
                throw runtime_error("Unable to read the file :: (permission denied or locked): " + filePath);
            }

            string code;
            vector<uint8_t> buffer;
            char ch;
            char chunk[1 << 20]; // 1 MB

            while(file.read(chunk, sizeof(chunk)) || file.gcount()) {
                size_t n = file.gcount();
                for(size_t i = 0; i < n; ++i) {
                    code += codes[(unsigned char)chunk[i]];
                    while(code.size() >= 8) {
                        buffer.push_back(generateByte(code));
                        code.erase(0, 8);
                    }
                }
            }
            if(!code.empty()) {
                paddingSize = 8 - code.size();
                buffer.push_back(generateByte(code));
            }
            if(!file.eof()) {
                throw runtime_error("Error occurred while encoding the data :: " + filePath);
            }
            bool ret = writeData(buffer);
            return ret;
        }
};

int main(int argc, char* argv[])
{
    try {
        
        if(argc != 2) {
            cout<<"Error: Not enough arguments.\n";
            cerr << "Usage: program <file_path>\n";
            return EXIT_FAILURE;
        }

        fs::path inputPath(argv[1]);
        if(!fs::exists(inputPath)) {
            std::cerr << "Error: file does not exist\n";
            return EXIT_FAILURE;
        }
        const string filePath = inputPath.string();
        const string fileName = inputPath.stem().string();
        cout<<"File path (input) : "<<filePath<<'\n';
        cout<< "File name (compressed) : "<<fileName<< '\n';
        
        Compression obj(filePath, fileName);
        
        if(obj.frequencyCount()) cout<<"Successfully counted frequencies...\n";
        if(obj.buildHuffmanTree()) cout<<"Sucessfully build huffman tree..\n";
        if(obj.generateCodes()) cout<<"Code generation process successfull...\n";   
        if(obj.encodeData()) cout<<"Encoding sucessfull..\n";

    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

