#ifndef LZLZIP_H_
#define LZLZIP_H_

#include"LZ77.h"
#include"Huffman.h"

namespace LZ_zip {
    class LZL_zip {
        private:
            std::unique_ptr<LZ77> lz_ptr;
            std::unique_ptr<Huffman> huf_ptr;
        
        public:
            LZL_zip() = default;
            explicit LZL_zip(const std::string& str):
                lz_ptr(std::make_unique<LZ77>(str)),
                huf_ptr(std::make_unique<Huffman>()) {}
        
        void compress() {
            lz_ptr->encode();
            lz_ptr->display();
            huf_ptr->encodeTokens(lz_ptr);
            huf_ptr->display();
        }     

        void decompress() {
            huf_ptr->decodeCompressedFile();
            auto tokens = huf_ptr->returnDecodedtokens();
            lz_ptr->assignDecodedResult(tokens);
            lz_ptr->decode();
        }
    };
}

#endif