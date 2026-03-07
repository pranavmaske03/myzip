#ifndef LZLZIP_H_
#define LZLZIP_H_

#include<stdexcept>
#include"LZ77.h"
#include"Huffman.h"

namespace LZ_zip {

    /**
     * @ LZL_zip    : Top-level interface for the compression library.
     *                Owns the LZ77 and Huffman instances and coordinates
     *                the full compress and decompress pipeline.
     *
     * @ compress   : Runs LZ77 token generation followed by Huffman encoding.
     *               Writes the compressed output to <filename>.LZ-zip.
     *
     * @ decompress : Reads the compressed file, decodes the Huffman bitstream
     *               back into tokens, then reconstructs the original file
     *               via LZ77 decoding. Writes output to decode-<filename>.
     */
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
                if(!lz_ptr || !huf_ptr) {
                    throw std::runtime_error("compress: LZL_zip was not initialized with a filename");
                }
                lz_ptr->encode();
                huf_ptr->encodeTokens(lz_ptr);
            }     

            void decompress() {
                if(!lz_ptr || !huf_ptr) {
                    throw std::runtime_error("decompress: LZL_zip was not initialized with a filename");
                }
                huf_ptr->decodeCompressedFile();
                auto tokens = huf_ptr->returnDecodedtokens();
                lz_ptr->assignDecodedResult(tokens);
                lz_ptr->decode();
            }

            void display() const {
                if(lz_ptr)  lz_ptr->display();
                if(huf_ptr) huf_ptr->display();
            }
    };
}

#endif