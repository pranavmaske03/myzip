#include"LZL-zip.h"

int main() {
    LZ_zip::LZL_zip zip("test.txt");
    zip.compress();
    zip.decompress();
}