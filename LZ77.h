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
    }

    void LZ77::runLZ77() {
        const std::string& fileContent = fileStream.readFile();
        fileSize = fileContent.size();
        std::cout<<"file content : "<<fileContent<<std::endl;
        generateTokens(fileContent);
    }

    void LZ77::generateTokens(const std::string_view& fileContent) {

    }
}
#endif