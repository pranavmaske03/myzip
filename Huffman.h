#ifndef HUFFMAN_H_
#define HUFFMAN_H_

#include<iostream>
#include<memory>
#include<unordered_map>
#include<unordered_set>         
#include<bitset>
#include<list>
#include<queue>
#include"LZ77.h"

namespace LZ_zip {

    class LZ77;
    class Huffman {
        private:
            int root;
            size_t tokensSize;
            std::string fileName;
            std::vector<int> HuffmanTree[65537];

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

            template
            <typename _key, typename _value, typename _target>
            void encoding(_target mp, int index);
            void processTokens(std::unique_ptr<LZ77>& lz_ptr);

        public:
            Huffman() = default;
            void encodeTokens(std::unique_ptr<LZ77>& lz_ptr);
            void display() const;
    };

    void Huffman::encodeTokens(std::unique_ptr<LZ77>& lz_ptr) {
        processTokens(lz_ptr);
    }

    void Huffman::processTokens(std::unique_ptr<LZ77>& lz_ptr) {
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
    }

    void Huffman::display() const {
        std::cout << "\n===== HUFFMAN INITIAL DATA =====\n";
        std::cout << "\nTotal Tokens: " << tokensSize << "\n";
        // Distance frequency
        std::cout << "\nDistance Frequency:\n";
        for (const auto& [key, value] : distance) {
            std::cout << "Distance " << key << " -> " << value << "\n";
        }
        // Length frequency
        std::cout << "\nLength Frequency:\n";
        for (const auto& [key, value] : length) {
            std::cout << "Length " << key << " -> " << value << "\n";
        }
        // Literal frequency
        std::cout << "\nLiteral Frequency:\n";
        for (const auto& [key, value] : literal) {
            if (key == '\0')
                std::cout << "Literal \\0 -> " << value << "\n";
            else
                std::cout << "Literal '" << key << "' -> " << value << "\n";
        }
        std::cout << "\n===============================\n";
    }
}

#endif