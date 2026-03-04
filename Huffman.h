#ifndef HUFFMAN_H_
#define HUFFMAN_H_

#include<iostream>
#include<memory>
#include<unordered_map>
#include<unordered_set>     
#include <filesystem>    
#include<bitset>
#include<list>
#include<queue>
#include"LZ77.h"

namespace LZ_zip {

    class LZ77;
    namespace fs = std::filesystem;

    class Huffman {
        private:
            int root;
            size_t tokensSize;
            std::string fileName;
            std::vector<int> HuffmanTree[65537];

            std::vector<typename LZ77::Token> decodeResult;
            std::unordered_map<int, int> distance; 
            std::unordered_map<int, int> length;
            std::unordered_map<unsigned short, int> literal;

            std::list<int> orderDistance;
            std::list<int> orderLength;
            std::list<unsigned short> orderLiteral;

            std::unordered_map<int, std::string> distanceCode;
            std::unordered_map<std::string, short> distanceNumber;
            std::unordered_map<int, std::string> lengthCode;
            std::unordered_map<std::string, short> lengthNumber;
            std::unordered_map<int, std::string> literalCode;
            std::unordered_map<std::string, unsigned short> literalNumber;

            template <typename _key, typename _value, typename _target>
            void buildHuffmanTree(_target mp, int index);

            template <typename _mp, typename _code>
            void write(std::string& buffer, OutputStream& os, const _mp& keyValue, _code& code);

            void processTokens(std::unique_ptr<LZ77>& lz_ptr);
            void decodeTokens(const std::string& fileContent);
            void buildDistanceCodes(int root, std::string&& str);
            void buildLengthCodes(int root, std::string&& str);
            void buildLiteralCodes(int root, std::string&& str);
            void writeEncodedData();
            bool zeroFill(std::string& str) const ;
            char conStrChar(const std::string& str, size_t pos, size_t n);
            std::string getBytes(char ch);
            void init();


        public:
            Huffman() = default;
            void encodeTokens(std::unique_ptr<LZ77>& lz_ptr);
            void decodeCompressedFile();
            void display() const;

            const auto& returnDecodedtokens() const {
                return decodeResult;
            }
    };

    inline void Huffman::encodeTokens(std::unique_ptr<LZ77>& lz_ptr) {
        processTokens(lz_ptr);

        buildHuffmanTree<int, int, decltype(distance)> (distance, 32768);
        buildDistanceCodes(root, "");
        init();

        buildHuffmanTree<int, int, decltype(length)> (length, 256);
        buildLengthCodes(root, "");
        init();

        buildHuffmanTree<int, unsigned short, decltype(literal)> (literal, 256);
        buildLiteralCodes(root, "");
        init();

        writeEncodedData();
    }

    inline void Huffman::processTokens(std::unique_ptr<LZ77>& lz_ptr) {
        auto tokens = lz_ptr->getTokens();
        fileName = lz_ptr->getFileName();
        
        for(const auto& [_distance, _length, _literal] : tokens) {
            distance[_distance]++;
            orderDistance.push_back(_distance);
            length[_length]++;
            orderLength.push_back(_length);
            literal[_literal]++;
            orderLiteral.push_back(_literal);
        }
        tokensSize = tokens.size();
        decodeResult.resize(tokens.size());
    }

    template <typename _key, typename _value, typename _target>
    inline void Huffman::buildHuffmanTree(_target freqTable, int index) {
        using Node = std::pair<_key, _value>;
        std::priority_queue<Node,std::vector<Node>,std::greater<Node>> minHeap;

        for (const auto& [_symbol, _frequency] : freqTable) {
            minHeap.emplace(_frequency, _symbol);
        }

        while (minHeap.size() > 1) {
            Node leftNode  = minHeap.top(); minHeap.pop();
            Node rightNode = minHeap.top(); minHeap.pop();

            Node curr = std::make_pair(leftNode.first + rightNode.first, ++index);
            minHeap.emplace(std::move(curr));
            HuffmanTree[index].emplace_back(leftNode.second);
            HuffmanTree[index].emplace_back(rightNode.second);
        }
        root = minHeap.top().second;
    }

    inline void Huffman::buildDistanceCodes(int root, std::string&& str) {
        if(HuffmanTree[root].empty()) {
            // std::cout<<"Leaf found: Symbol = "<<root<<", Code = "<<str<<"\n";
            distanceNumber[str] = static_cast<short>(root);
            distanceCode[root] = std::move(str);
            return;
        }
        buildDistanceCodes(HuffmanTree[root][0], str + '0');
        buildDistanceCodes(HuffmanTree[root][1], str + '1');
    }

    inline void Huffman::buildLengthCodes(int root, std::string&& str) {
        if(HuffmanTree[root].empty()) {
            // std::cout<<"Leaf found: Symbol = "<<root<<", Code = "<<str<<"\n";
            lengthNumber[str] = static_cast<short>(root);
            lengthCode[root] = std::move(str);
            return;
        }
        buildLengthCodes(HuffmanTree[root][0], str + '0');
        buildLengthCodes(HuffmanTree[root][1], str + '1');
    }

    inline void Huffman::buildLiteralCodes(int root, std::string&& str) {
        if(HuffmanTree[root].empty()) {
            // std::cout<<"Leaf found: Symbol = "<<root<<", Code = "<<str<<"\n";
            literalNumber[str] = static_cast<short>(root);
            literalCode[root] = std::move(str);
            return;
        }
        buildLiteralCodes(HuffmanTree[root][0], str + '0');
        buildLiteralCodes(HuffmanTree[root][1], str + '1');
    }

    inline void Huffman::writeEncodedData() {
        fs::path inputFile(fileName);
        const std::string _filename = inputFile.stem().string();
        // std::cout<<"file name without extension: "<<_filename<<std::endl;
        OutputStream os(_filename + ".LZ-zip");
        std::string buffer;

        write(buffer, os, orderDistance, distanceCode);
        write(buffer, os, orderLength, lengthCode);
        write(buffer, os, orderLiteral, literalCode);
        if(zeroFill(buffer)) {
            char byte = conStrChar(buffer, 0, 8);
            os.writeFile(byte);
        }
    }

    template <typename _mp, typename _code>
    inline void Huffman::write(std::string& buffer, OutputStream& os, const _mp& keyValue, _code& code)
    {
        for(const auto& _number : keyValue) {
            std::string bits = code[_number];
            buffer.append(bits);

            while(buffer.size() >= 8) {
                char byte = conStrChar(buffer, 0, 8);
                os.writeFile(byte);
                buffer.erase(0, 8);
            }
        }
    }

    inline bool Huffman::zeroFill(std::string& str) const {
        size_t len = 8 - str.size();
        if(len) {
            while(len--) str.push_back('0');        
            return true;
        }
        else return false;
    }

    inline void Huffman::init() {
        for(int i = 0 ;i < 65537; ++i) HuffmanTree[i].clear();
    }

    inline char Huffman::conStrChar(const std::string& str, size_t pos, size_t n) {
        std::bitset<8> number(str, pos, n);
        return static_cast<char> (number.to_ulong());
    }

    inline void Huffman::display() const {
        std::cout << "\n===== HUFFMAN INITIAL DATA =====\n";
        std::cout << "\nTotal Tokens: " << tokensSize << "\n";
        std::cout << "\nDistance Frequency:\n";
        for (const auto& [key, value] : distance) {
            std::cout << "Distance " << key << " -> " << value << "\n";
        }
        std::cout << "\nLength Frequency:\n";
        for (const auto& [key, value] : length) {
            std::cout << "Length " << key << " -> " << value << "\n";
        }
        std::cout << "\nLiteral Frequency:\n";
        for (const auto& [key, value] : literal) {
            if (key == '\0')
                std::cout << "Literal \\0 -> " << value << "\n";
            else
                std::cout << "Literal '" << key << "' -> " << value << "\n";
        }
        std::cout << "\n===============================\n";
    }

    // Decoding methods
    inline void Huffman::decodeCompressedFile() {
        fs::path inputFile(fileName);
        const std::string _filename = inputFile.stem().string();
        InputStream in(_filename + ".LZ-zip");
        std::string data = in.readFile();
        for(int i = 0; i < 20 && i < data.size(); i++) {
            unsigned char byte = static_cast<unsigned char> (data[i]);
        }
        std::string fileContent;
        for(int i = 0; i < data.size(); i++) {
            fileContent.append(std::move(getBytes(data[i])));
        }
        // std::cout<<"filecontent : "<<fileContent<<std::endl;
        decodeTokens(fileContent);
    }

    inline void Huffman::decodeTokens(const std::string& fileContent) {
        size_t index = 0;
        std::string str;

        size_t entry = 0;
        while(entry < tokensSize) {
            str.push_back(fileContent[index++]);
            if(distanceNumber.find(str) != distanceNumber.end()) {
                decodeResult[entry++].distance = distanceNumber[str];
                str.clear();
            }
        }

        entry = 0;
        while(entry < tokensSize) {
            str.push_back(fileContent[index++]);
            if(lengthNumber.find(str) != lengthNumber.end()) {
                decodeResult[entry++].length = lengthNumber[str];
                str.clear();
            }
        }

        entry = 0;
        while(entry < tokensSize) {
            str.push_back(fileContent[index++]);
            if(literalNumber.find(str) != literalNumber.end()) {
                decodeResult[entry++].literal = literalNumber[str];
                str.clear();
            }
        }

        // std::cout<<"Size of the decode result is :"<<decodeResult.size()<<std::endl;
        // std::cout<<"Decode result tocken: \n";
        // for(auto T : decodeResult){
        //     std::cout << T.distance << " " <<T.length << " " << T.literal << std::endl;
        // }
    }

    inline std::string Huffman::getBytes(char ch) {
        static unsigned char bit[8] = {128, 64, 32, 16, 8, 4, 2, 1};
        std::string str;
        for(int i = 0;i < 8; ++i) {
            ch & bit[i] ? str.push_back('1') : str.push_back('0');
        }
        return str;
    }
}

#endif