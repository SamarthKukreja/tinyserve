#include "runtime/generate.hpp"

#include <iostream>

#ifndef TINYSERVE_VERSION
#define TINYSERVE_VERSION "unknown"
#endif

int main(int argc, char** argv) {
  if (argc == 1) {
    std::cout << "TinyServe " << TINYSERVE_VERSION
              << " - educational inference runtime (fixture-validated only)\n";
  }
  return tinyserve::run_cli(argc, argv);
}
