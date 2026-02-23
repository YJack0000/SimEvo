#include <core/Genes.hpp>
#include <cstring>
#include <functional>
#include <random>

Genes::Genes(const char *dnaStr) : Genes(dnaStr, nullptr) {}

Genes::Genes(const char *dnaStr, MutationFunction customMutationLogic = nullptr)
    : mutationLogic(customMutationLogic ? customMutationLogic : defaultMutationLogic) {
    std::memcpy(dna, dnaStr, 4);
}

/**
 * @brief Default mutation: randomly adjusts each gene by -3 to +3.
 * @param dna Array of 4 gene bytes (each 0-255) to mutate in place.
 *
 * Uses thread_local RNG so this is safe to call from multiple threads
 * during the parallel reaction phase.
 */
void Genes::defaultMutationLogic(char dna[4]) {
    // thread_local ensures each thread gets its own RNG instance for thread safety
    static thread_local std::mt19937 rng(std::random_device{}());
    // Mutation range: each gene shifts by -3 to +3 per generation
    std::uniform_int_distribution<int> distribution(-3, 3);

    for (int i = 0; i < 4; ++i) {
        dna[i] += distribution(rng);
    }
}

/** @brief Apply the mutation function to this organism's DNA. */
void Genes::mutate() { mutationLogic(dna); }

/**
 * @brief Access a specific gene value.
 * @param index Gene index (0=speed, 1=size, 2=awareness, 3=reserved).
 * @return Raw gene value as a char (0-255).
 */
char Genes::getDNA(int index) const { return dna[index]; }
