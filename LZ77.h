#ifndef LZ77_H_
#define LZ77_H_

#include<vector>
#include<string_view>
#include<algorithm>
#include<stdexcept>
#include"IOStream.h"

namespace LZ_zip {

    /**
     * @ LZ77          : Core compression class. Encodes a file into LZ77 tokens
     *                   and decodes tokens back into the original file content.
     *
     * @ encode        : Entry point for compression. Reads the file and runs LZ77.
     * @ decode        : Entry point for decompression. Reconstructs file from tokens.
     * @ display       : Prints token list and file size to stdout for debugging.
     *
     * @ runLZ77       : Reads file content and delegates to generateTokens.
     * @ generateTokens: Scans input with a sliding window and lookahead buffer.
     *                   Emits a (distance, length, literal) token for each position.
     *
     * @ decodeData    : Reconstructs original content from decoded token list
     *                   and writes it to disk as decode-<filename>.
     *
     * @ getTokens     : Returns the generated token list (used by Huffman).
     * @ getFileName   : Returns the source file name (used by Huffman).
     * @ assignDecodedResult : Receives the decoded token list from Huffman.
     */
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
            void assignDecodedResult(const decltype(decodedResult) result);
            void decodeData();
            void encode();
            void decode();
            void display() const;

        public:
            LZ77() = default;
            explicit LZ77(std::string name): 
                fileName(std::move(name)),
                fileStream(fileName) {}
    };

    inline void LZ77::encode() { runLZ77(); }
    inline void LZ77::decode() { decodeData(); }

    inline const std::vector<LZ77::Token>& LZ77::getTokens() const { return tokens; }

    inline const std::string& LZ77::getFileName() const { return fileName; }

    inline void LZ77::assignDecodedResult(const decltype(decodedResult) result) {
        decodedResult = std::move(result);
    }

    inline void LZ77::display() const {
        std::cout<<"file size : "<<fileSize<<std::endl;
        std::cout<<"tokens size : "<<tokens.size()<<std::endl;
        for(const auto& T : tokens) {
            std::cout << T.distance << " " << T.length << " " << T.literal << std::endl;
        }
    }

    inline void LZ77::decodeData() {
        for(size_t i = 0; i < decodedResult.size(); i++) {
            if(decodedResult[i].length == 0) {
                decodeFileContent += decodedResult[i].literal;
            } else {
                int start = static_cast<int>(decodeFileContent.length()) - decodedResult[i].distance;
                if(start < 0) {
                    throw std::runtime_error("decodeData: invalid distance — points before start of decoded content");
                }
                std::string tmp = decodeFileContent.substr(start, decodedResult[i].length);
                decodeFileContent += tmp;
                if(decodedResult[i].literal != '\0') {
                    decodeFileContent += decodedResult[i].literal;
                }
            }
        }

        OutputStream os("decode-" + fileName);
        os.writeFile(decodeFileContent);
    }

    inline void LZ77::runLZ77() {
        const std::string& fileContent = fileStream.readFile();
        fileSize = fileContent.size();

        if(fileSize == 0) {
            throw std::runtime_error("runLZ77: input file is empty — cannot generate tokens");
        }
        
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