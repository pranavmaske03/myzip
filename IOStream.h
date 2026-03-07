#ifndef IOSTREAM_H_
#define IOSTREAM_H_

#include<string>
#include<fstream>
#include<iostream>
#include<sstream>
#include<stdexcept>

namespace LZ_zip {

    /**
     * @ InputStream  : Opens a file in binary read mode and loads its content.
     * @ readFile     : Reads the entire file into a string and returns it.
     * @ closeFile    : Closes the file stream if it is still open.
     * @ Close        : Public wrapper to manually close the stream early.
     */
    class InputStream {
        public:
            explicit InputStream(std::string name)
            : fileStream(name, std::ios::binary | std::ios::in) {
                if (!fileStream.is_open()) {
                    throw std::runtime_error("InputStream: cannot open file -> " + name);
                }
            }

            std::string readFile() {
                std::ostringstream buffer;
                buffer << fileStream.rdbuf();
                if (fileStream.fail() && !fileStream.eof()) {
                    throw std::runtime_error("InputStream: failed to read file");
                }
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

    /**
     * @ OutputStream : Opens a file in binary write mode.
     * @ writeFile    : Writes any data type to the file stream.
     * @ closeFile    : Closes the file stream if it is still open.
     * @ Close        : Public wrapper to manually close the stream early.
     */
    class OutputStream {
        public:
            explicit OutputStream(std::string name): fileStream(name, std::ios::binary | std::ios::out) {
                if(!fileStream.is_open()) {
                    throw std::runtime_error("OutputStream: cannot open file -> " + name);
                }
            }
            
            template <typename T>
            void writeFile(T&& data) {
                if(fileStream.is_open()) {
                    fileStream << data;
                    if(fileStream.fail()) {
                        throw std::runtime_error("OutputStream: failed to write data");
                    }
                }
            }

            ~OutputStream() { closeFile(); }
            void Close() { closeFile(); }

        private:
            std::ofstream fileStream;
            void closeFile() {
                if(fileStream.is_open()) {
                    fileStream.close();
                }
            }
    };
}

#endif