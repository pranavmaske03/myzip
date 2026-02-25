#include"LZL-zip.h"

int main() {
    std::cout<<"Hello there..\n";
    LZ_zip::LZL_zip zip("test.txt");
    zip.compress();
}