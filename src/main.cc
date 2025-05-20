#include "Analyzer.h"

#include <iostream>
#include <string>


int main(int argc, char* argv[]) {

  std::string sampleName = argv[1];
  std::string era = argv[2];
  int idx = std::stoi(argv[3]);
  Analyzer analyzer;

  analyzer.Init(sampleName, era, idx);
  analyzer.Run();
  analyzer.End();
  
  std::cout << "Analysis completed successfully" << std::endl;
  
  return 0;
} 