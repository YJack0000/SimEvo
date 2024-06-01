#include <core/Genes.hpp>
#include <cstring>
#include <functional>
#include <random>

Genes::Genes(const char *dnaStr) : Genes(dnaStr, nullptr) {
    std::memcpy(dna, dnaStr, 4);
    // printf("Genes %s is being created with %u %u %u %u\n", dnaStr, dna[0], dna[1], dna[2], dna[3]);
}

Genes::Genes(const char *dnaStr, MutationFunction customMutationLogic = nullptr)
    : mutationLogic(customMutationLogic ? customMutationLogic : defaultMutationLogic) {
    std::memcpy(dna, dnaStr, 4);
    // printf("Genes %s is being created with %u %u %u %u\n", dnaStr, dna[0], dna[1], dna[2], dna[3]);
}

void Genes::defaultMutationLogic(char dna[4]) {
    static std::mt19937 rng(std::random_device{}());         // Seed with random_device
    std::uniform_int_distribution<int> distribution(-3, 3);  // Range from -10 to 10

    for (int i = 0; i < 4; ++i) {
        int mutation = distribution(rng);  // Generate a random mutation from -1 to 1
        dna[i] += mutation;
    }
}

void Genes::mutate() { mutationLogic(dna); }

char Genes::getDNA(int index) const { return dna[index]; }
