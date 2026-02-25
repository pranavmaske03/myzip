#ifndef IOSTREAM_H_
#define IOSTREAM_H_

#include<string>
#include<fstream>
#include<iostream>
#include<sstream>

namespace LZ_zip {
    class InputStream {
        public:
            explicit InputStream(std::string fileName)
            : encodeFile(std::move(fileName)) {
                if (!encodeFile.is_open()) {
                    std::cout << "Error: file not open\n";
                }
            }

            std::string readFile() {
                std::ostringstream buffer;
                buffer << encodeFile.rdbuf();
                return buffer.str();
            }

            ~InputStream() { closeFile(); }
            void Close() { closeFile(); }

        private:
            std::fstream encodeFile;
            void closeFile() {
                if(encodeFile.is_open()) 
                    encodeFile.close();
            }
    };

    class OutputStream {

    };
}

#endif