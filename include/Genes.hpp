#ifndef _GENES_HPP
#define _GENES_HPP

#include <functional>

class Genes {
public:
  using MutationFunction = std::function<void(char[4])>;

  Genes(const char *dnaStr, MutationFunction customMutationLogic);

private:
  char dna[4];
  MutationFunction mutationLogic;
  static void defaultMutationLogic(char dna[4]);
};

#endif
