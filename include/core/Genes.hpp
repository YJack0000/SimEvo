#ifndef GENES_HPP
#define GENES_HPP

#include <functional>

class Genes {
public:
    using MutationFunction = std::function<void(char[4])>;

    Genes(const char *dnaStr, MutationFunction customMutationLogic);

    void mutate() { mutationLogic(dna); }
    char getGene(int index) const { return dna[index]; }
private:
    char dna[4];
    MutationFunction mutationLogic;
    static void defaultMutationLogic(char dna[4]);
};

#endif
