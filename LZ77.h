#ifndef LZ77_H_
#define LZ77_H_

#include<vector>
#include<string_view>
#include"IOStream.h"

namespace LZ_zip {
    class LZ77 {
        friend class Huffman;
        friend class LZL_zip;

        private:
            struct Token {
                int distance;
                int length;
                char literal;
            };

            std::vector<Token> tokens;
            std::string fileName;   
            InputStream fileStream;
            std::size_t fileSize;
            std::size_t maxWindow = 32767;

            void runLZ77();
            void generateTokens(const std::string_view& file);
        public:
            LZ77() = default;
            explicit LZ77(std::string name): 
                fileName(std::move(name)),
                fileStream(fileName) {}
            
            void encode();
            void display() const;

    };

    void LZ77::encode() { runLZ77(); }

    void LZ77::display() const {
        std::cout<<"file size : "<<fileSize<<std::endl;
        std::cout<<"tokens size : "<<tokens.size()<<std::endl;
        for(auto T : tokens) {
            std::cout << T.distance << " " <<T.length << " " << T.literal <<std::endl;
        }
    }

    void LZ77::runLZ77() {
        const std::string& fileContent = fileStream.readFile();
        fileSize = fileContent.size();
        std::cout<<"file content : "<<fileContent<<std::endl;
        generateTokens(fileContent);
    }

    void LZ77::generateTokens(const std::string_view& input) {
        size_t searchWindowSize = 0;
        size_t lookaheadWindowSize = 0;
        for(size_t i = 0; i < input.length(); i++) {
            searchWindowSize = (i + 1 <= maxWindow) ? i : maxWindow;
            lookaheadWindowSize = (i + searchWindowSize < input.length()) ? searchWindowSize : input.length() - i;

            std::string_view searchWindow = input.substr(i - searchWindowSize, searchWindowSize);
            std::string_view lookaheadWindow = input.substr(i, lookaheadWindowSize);

            int matchIndex = -1;
            while(lookaheadWindowSize > 0) {
                std::string_view candidate = lookaheadWindow.substr(0, lookaheadWindowSize);
                matchIndex = searchWindow.find(candidate);

                if(matchIndex != -1) 
                    break;
                lookaheadWindowSize--;
            }

            if(matchIndex != -1) {
                Token tmp;
                tmp.length = lookaheadWindowSize;
                tmp.distance = searchWindowSize - matchIndex;
                tmp.literal = input[i + lookaheadWindowSize];
                tokens.push_back(std::move(tmp));
                i += lookaheadWindowSize;
            } else {
                Token tmp;
                tmp.length = 0;
                tmp.distance = 0;
                tmp.literal = input[i + lookaheadWindowSize];
                tokens.push_back(std::move(tmp));
            }
        }
    }
}
#endif