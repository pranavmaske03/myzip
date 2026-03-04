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
            std::vector<Token> decodedResult;
            std::string fileName;   
            std::string decodeFileContent;
            InputStream fileStream;
            std::size_t fileSize;
            std::size_t maxWindow = 32767;

            void runLZ77();
            void generateTokens(const std::string_view& file);
            const std::vector<Token>& getTokens() const;
            const std::string& getFileName() const;
            void assignDecodedResult(const decltype(decodedResult)& result);
            void decodeData();

        public:
            LZ77() = default;
            explicit LZ77(std::string name): 
                fileName(std::move(name)),
                fileStream(fileName) {}
            
            void encode();
            void decode();
            void display() const;

    };

    inline void LZ77::encode() { runLZ77(); }
    inline void LZ77::decode() { decodeData(); }

    inline const std::vector<LZ77::Token>& LZ77::getTokens() const {
        return tokens;
    }

    inline const std::string& LZ77::getFileName() const {
        return fileName;
    }

    inline void LZ77::assignDecodedResult(const decltype(decodedResult)& result) {
        decodedResult.assign(result.begin(), result.end());
    }

    inline void LZ77::display() const {
        std::cout<<"file size : "<<fileSize<<std::endl;
        std::cout<<"tokens size : "<<tokens.size()<<std::endl;
        for(auto T : tokens) {
            std::cout << T.distance << " " <<T.length << " " << T.literal <<std::endl;
        }
    }

    inline void LZ77::decodeData() {
        for(int i = 0; i < decodedResult.size(); i++) {
            if(decodedResult[i].length == 0) {
                decodeFileContent += decodedResult[i].literal;
            } else {
                int length = decodeFileContent.length();
                length -= decodedResult[i].distance;
                std::string tmp = decodeFileContent.substr(length, decodedResult[i].length);
                decodeFileContent += tmp;
                if(decodedResult[i].literal != '\0') {
                    decodeFileContent += decodedResult[i].literal;
                }
            }
        }

        // std::cout<<"Decode file content : \n";
        // std::cout<<decodeFileContent<<std::endl;
        OutputStream os("decode-"+fileName);
        os.writeFile(decodeFileContent);
    }

    inline void LZ77::runLZ77() {
        const std::string& fileContent = fileStream.readFile();
        fileSize = fileContent.size();
        // std::cout<<"file content : "<<fileContent<<std::endl;
        decodeFileContent.reserve(fileContent.size());
        generateTokens(fileContent);
    }

    inline void LZ77::generateTokens(const std::string_view& input) {
        size_t searchWindowSize = 0;
        size_t lookaheadWindowSize = 0;
        constexpr size_t maxLookahead = 256;
        
        for(size_t i = 0; i < input.length(); i++) {
            searchWindowSize = (i + 1 <= maxWindow) ? i : maxWindow;
            lookaheadWindowSize = std::min( input.length() - i, maxLookahead );
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
                tmp.literal = (i + lookaheadWindowSize < input.length()) ? input[i + lookaheadWindowSize] : '\0';
                tokens.push_back(std::move(tmp));
                i += lookaheadWindowSize;
            } else {
                Token tmp;
                tmp.length = 0;
                tmp.distance = 0;
                tmp.literal = (i + lookaheadWindowSize < input.length()) ? input[i + lookaheadWindowSize] : '\0';
                tokens.push_back(std::move(tmp));
            }
        }
    }
}
#endif