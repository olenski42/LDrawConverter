#pragma once
#include <iostream>

#define LogI(x) {std::cout << "[INFO]: " << x << "  - (" << __FILE__ << ":" << __LINE__ << ")" << std::endl;}
#define LogW(x) {std::cout << "[WARNING]: " << x << "  - (" << __FILE__ << ":" << __LINE__ << ")" << std::endl;}
#define LogE(x) {std::cout << "[ERROR]: " << x << "  - (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; exit(-1);}