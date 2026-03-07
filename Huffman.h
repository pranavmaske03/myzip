#ifndef HUFFMAN_H_
#define HUFFMAN_H_

#include<iostream>
#include<memory>
#include<unordered_map>
#include<filesystem>    
#include<bitset>
#include<vector>
#include<queue>
#include<stdexcept>
#include"LZ77.h"

namespace LZ_zip {

    class LZ77;
    namespace fs = std::filesystem;

    /**
     * @ Huffman             : Encodes LZ77 tokens using Huffman coding and writes
     *                         the compressed bitstream to a .LZ-zip file.
     *                         Decodes the compressed file back into tokens.
     *
     * @ encodeTokens        : Full encode pipeline — process tokens, build trees,
     *                         generate codes, write bitstream to disk.
     *
     * @ decodeCompressedFile: Reads the .LZ-zip file and decodes it back into tokens.
     *
     * @ processTokens       : Extracts frequency tables and ordered sequences
     *                         from the LZ77 token list.
     *
     * @ buildHuffmanTree    : Builds a min-heap based Huffman tree from a
     *                         frequency table. Internal nodes stored in huffmanTree map.
     *
     * @ buildCodes          : DFS traversal of the Huffman tree to assign
     *                         binary codes to each symbol (0 = left, 1 = right).
     *
     * @ write               : Appends Huffman codes to a bit buffer and flushes
     *                         complete bytes to the output stream.
     *
     * @ decodeTokens        : Reads the bitstream and reconstructs the token list
     *                         by matching bit patterns against the Huffman code tables.
     *
     * @ zeroFill            : Pads the remaining bits in the buffer to a full byte
     *                         before the final flush.
     *
     * @ conStrChar          : Converts an 8-bit string of '0'/'1' characters to a char.
     * @ getBytes            : Converts a char to its 8-bit string representation.
     * @ init                : Clears the Huffman tree map between encode passes.
     * @ display             : Prints frequency tables to stdout for debugging.
     * @ returnDecodedtokens : Returns the reconstructed token list to LZL_zip.
     */
    class Huffman {
        friend class LZL_zip;
        
        private:
            int root = 0;
            size_t tokensSize = 0;
            std::string fileName;
            std::unordered_map<int, std::vector<int>> huffmanTree;

            std::vector<typename LZ77::Token> decodeResult;
            std::unordered_map<int, int> distance; 
            std::unordered_map<int, int> length;
            std::unordered_map<unsigned short, int> literal;

            std::vector<int> orderDistance;
            std::vector<int> orderLength;
            std::vector<unsigned short> orderLiteral;

            std::unordered_map<int, std::string> distanceCode;
            std::unordered_map<std::string, short> distanceNumber;
            std::unordered_map<int, std::string> lengthCode;
            std::unordered_map<std::string, short> lengthNumber;
            std::unordered_map<int, std::string> literalCode;
            std::unordered_map<std::string, unsigned short> literalNumber;

            template <typename Key, typename Value, typename Map>
            void buildHuffmanTree(Map mp, int index);

            template <typename OrderList, typename CodeMap>
            void write(std::string& buffer, OutputStream& os, const OrderList& keyValue, const CodeMap& code);

            template <typename CodeMap, typename NumberMap>
            void buildCodes(int node, std::string code, CodeMap& codeOut, NumberMap& numberOut);

            void processTokens(std::unique_ptr<LZ77>& lz_ptr);
            void decodeTokens(const std::string& fileContent);
            void writeEncodedData();
            bool zeroFill(std::string& str) const;
            char conStrChar(const std::string& str, size_t pos, size_t n);
            std::string getBytes(char ch);
            void init();
            void encodeTokens(std::unique_ptr<LZ77>& lz_ptr);
            void decodeCompressedFile();
            void display() const;

            const auto& returnDecodedtokens() const { return decodeResult; }

        public:
            Huffman() = default;
    };

    inline void Huffman::encodeTokens(std::unique_ptr<LZ77>& lz_ptr) {
        processTokens(lz_ptr);

        buildHuffmanTree<int, int, decltype(distance)>(distance, 32768);
        buildCodes(root, "", distanceCode, distanceNumber);
        init();

        buildHuffmanTree<int, int, decltype(length)>(length, 256);
        buildCodes(root, "", lengthCode, lengthNumber);
        init();

        buildHuffmanTree<int, unsigned short, decltype(literal)>(literal, 256);
        buildCodes(root, "", literalCode, literalNumber);
        init();

        writeEncodedData();
    }

    inline void Huffman::processTokens(std::unique_ptr<LZ77>& lz_ptr) {
        auto tokens = lz_ptr->getTokens();
        fileName = lz_ptr->getFileName();

        if(tokens.empty()) {
            throw std::runtime_error("processTokens: token list is empty — was encode() called?");
        }
        
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

    template <typename Key, typename Value, typename Map>
    inline void Huffman::buildHuffmanTree(Map freqTable, int index) {
        if(freqTable.empty()) {
            throw std::runtime_error("buildHuffmanTree: frequency table is empty");
        }

        using Node = std::pair<Key, Value>;
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> minHeap;

        for (const auto& [_symbol, _frequency] : freqTable) {
            minHeap.emplace(_frequency, _symbol);
        }

        if(minHeap.size() == 1) {
            root = minHeap.top().second;
            return;
        }

        while (minHeap.size() > 1) {
            Node leftNode  = minHeap.top(); minHeap.pop();
            Node rightNode = minHeap.top(); minHeap.pop();

            Node curr = std::make_pair(leftNode.first + rightNode.first, ++index);
            minHeap.emplace(std::move(curr));
            huffmanTree[index].emplace_back(leftNode.second);
            huffmanTree[index].emplace_back(rightNode.second);
        }
        root = minHeap.top().second;
    }

    template <typename CodeMap, typename NumberMap>
    inline void Huffman::buildCodes(int node, std::string code, CodeMap& codeOut, NumberMap& numberOut) {
        if (huffmanTree[node].empty()) {
            if(code.empty()) code = "0";
            codeOut[node]   = code;
            numberOut[code] = node;
            return;
        }
        buildCodes(huffmanTree[node][0], code + '0', codeOut, numberOut);
        buildCodes(huffmanTree[node][1], code + '1', codeOut, numberOut);
    }

    inline void Huffman::writeEncodedData() {
        fs::path inputFile(fileName);
        const std::string _filename = inputFile.stem().string();
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

    template <typename OrderList, typename CodeMap>
    inline void Huffman::write(std::string& buffer, OutputStream& os, const OrderList& keyValue, const CodeMap& code) {
        for(const auto& _number : keyValue) {
            auto it = code.find(_number);
            if (it != code.end()) {
                buffer.append(it->second);
            }
            while(buffer.size() >= 8) {
                char byte = conStrChar(buffer, 0, 8);
                os.writeFile(byte);
                buffer.erase(0, 8);
            }
        }
    }

    inline bool Huffman::zeroFill(std::string& str) const {
        if(str.empty()) return false;
        size_t len = 8 - str.size();
        while(len--) str.push_back('0');
        return true;
    }

    inline void Huffman::init() {
        huffmanTree.clear();
    }

    inline char Huffman::conStrChar(const std::string& str, size_t pos, size_t n) {
        std::bitset<8> number(str, pos, n);
        return static_cast<char>(number.to_ulong());
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
                std::cout << "Literal '" << (char)key << "' -> " << value << "\n";
        }
        std::cout << "\n===============================\n";
    }

    inline void Huffman::decodeCompressedFile() {
        fs::path inputFile(fileName);
        const std::string _filename = inputFile.stem().string();
        InputStream in(_filename + ".LZ-zip");
        std::string data = in.readFile();

        if(data.empty()) {
            throw std::runtime_error("decodeCompressedFile: compressed file is empty or missing");
        }

        std::string fileContent;
        fileContent.reserve(data.size() * 8);
        for(size_t i = 0; i < data.size(); i++) {
            fileContent.append(getBytes(data[i]));
        }
        decodeTokens(fileContent);
    }

    inline void Huffman::decodeTokens(const std::string& fileContent) {
        size_t index = 0;
        std::string str;
        const size_t totalBits = fileContent.size();

        auto safePeek = [&]() -> bool {
            return index < totalBits;
        };

        size_t entry = 0;
        while(entry < tokensSize) {
            if(!safePeek()) throw std::runtime_error("decodeTokens: bitstream ended while reading distance tokens");
            str.push_back(fileContent[index++]);
            if(distanceNumber.find(str) != distanceNumber.end()) {
                decodeResult[entry++].distance = distanceNumber[str];
                str.clear();
            }
        }

        entry = 0;
        while(entry < tokensSize) {
            if(!safePeek()) throw std::runtime_error("decodeTokens: bitstream ended while reading length tokens");
            str.push_back(fileContent[index++]);
            if(lengthNumber.find(str) != lengthNumber.end()) {
                decodeResult[entry++].length = lengthNumber[str];
                str.clear();
            }
        }

        entry = 0;
        while(entry < tokensSize) {
            if(!safePeek()) throw std::runtime_error("decodeTokens: bitstream ended while reading literal tokens");
            str.push_back(fileContent[index++]);
            if(literalNumber.find(str) != literalNumber.end()) {
                decodeResult[entry++].literal = literalNumber[str];
                str.clear();
            }
        }
    }

    inline std::string Huffman::getBytes(char ch) {
        static const unsigned char bit[8] = {128, 64, 32, 16, 8, 4, 2, 1};
        std::string str;
        str.reserve(8);
        for(int i = 0; i < 8; ++i) {
            str.push_back((ch & bit[i]) ? '1' : '0');
        }
        return str;
    }
}

#endif