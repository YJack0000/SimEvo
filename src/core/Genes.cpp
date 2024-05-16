#include <core/Genes.hpp>
#include <cstring>
#include <ctime>
#include <cstdlib>

Genes::Genes(const char *dnaStr, MutationFunction customMutationLogic = nullptr)
    : mutationLogic(customMutationLogic ? customMutationLogic
                                        : defaultMutationLogic) {
    std::memcpy(dna, dnaStr, 4);
}

void Genes::defaultMutationLogic(char dna[4]) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    for (int i = 0; i < 4; ++i) {
        int mutation = (std::rand() % 3) - 1;
        int mutatedChar = static_cast<int>(dna[i]) + mutation;

        dna[i] = static_cast<char>(mutatedChar);
    }
}
