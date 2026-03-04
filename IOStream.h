#ifndef IOSTREAM_H_
#define IOSTREAM_H_

#include<string>
#include<fstream>
#include<iostream>
#include<sstream>

namespace LZ_zip {
    class InputStream {
        public:
            explicit InputStream(std::string name)
            : fileStream(name, std::ios::binary | std::ios::in) {
                if (!fileStream.is_open()) {
                    std::cout << "Error: file not open\n";
                }
            }

            std::string readFile() {
                std::ostringstream buffer;
                buffer << fileStream.rdbuf();
                return buffer.str();
            }

            ~InputStream() { closeFile(); }
            void Close() { closeFile(); }

        private:
            std::ifstream fileStream;
            void closeFile() {
                if(fileStream.is_open())
                    fileStream.close();
            }
    };

    class OutputStream {
        public:
            explicit OutputStream(std::string name): fileName(name, std::ios::binary | std::ios::out) {
                if(!fileName.is_open()) {
                    std::cout << "Error: file not open\n";
                }
            }
            
            template <typename T>
            void writeFile(T&& data) {
                if(fileName.is_open()) {
                    fileName << data;
                }
            }

            ~OutputStream() { closeFile(); }
            void Close() { closeFile(); }

        private:
            std::ofstream fileName;
            void closeFile() {
                if(fileName.is_open()) {
                    fileName.close();
                }
            }
    };
}

#endif