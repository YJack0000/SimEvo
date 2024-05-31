#ifndef GENES_HPP
#define GENES_HPP

#include <functional>

class Genes {
public:
    using MutationFunction = std::function<void(char[4])>;

    Genes(const char *dnaStr);
    Genes(const char *dnaStr, MutationFunction customMutationLogic);

    void mutate();
    char getDNA(int index) const;

private:
    char dna[4];
    MutationFunction mutationLogic;
    static void defaultMutationLogic(char dna[4]);
};

#endif
