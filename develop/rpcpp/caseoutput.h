#pragma once

// --

struct CaseOutput {
  CaseOutput(const std::string& n) {
    std::cout << "--" << n << std::endl;
  }
  virtual ~CaseOutput() {
    std::cout << std::endl;
  }
};