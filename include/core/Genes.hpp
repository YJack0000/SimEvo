#ifndef GENES_HPP
#define GENES_HPP

#include <functional>

/**
 * @brief Encodes the genetic traits of an organism as a 4-byte DNA sequence.
 *
 * Each byte in the DNA array maps to a specific organism trait:
 *   - dna[0]: speed
 *   - dna[1]: size
 *   - dna[2]: awareness
 *   - dna[3]: reserved
 *
 * Mutation logic can be customized by providing a MutationFunction. If none
 * is provided, the default adds a small random offset ([-3, +3]) to each byte.
 */
class Genes {
public:
    /// @brief Function type for custom mutation logic operating on the 4-byte DNA.
    using MutationFunction = std::function<void(char[4])>;

    /**
     * @brief Construct genes from a 4-byte DNA string using default mutation logic.
     * @param dnaStr Pointer to a 4-byte character array representing the DNA.
     */
    Genes(const char *dnaStr);

    /**
     * @brief Construct genes with optional custom mutation logic.
     * @param dnaStr Pointer to a 4-byte character array representing the DNA.
     * @param customMutationLogic Custom mutation function; if null, uses default.
     */
    Genes(const char *dnaStr, MutationFunction customMutationLogic);

    /// @brief Apply the mutation function to this gene's DNA in-place.
    void mutate();

    /**
     * @brief Access a specific DNA byte by index.
     * @param index The DNA index (0-3).
     * @return The raw byte value at the given index.
     */
    char getDNA(int index) const;

private:
    char dna[4];                    ///< The 4-byte DNA sequence
    MutationFunction mutationLogic; ///< The mutation strategy applied during reproduction

    /// @brief Default mutation: adds uniform random offset in [-3, +3] to each byte.
    static void defaultMutationLogic(char dna[4]);
};

#endif
