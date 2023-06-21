/**
 * moment_rulebook_to_basis.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <Eigen/SparseCore>

namespace Moment {
    class SymbolTable;
    class MomentRulebook;
    class PolynomialFactory;

    /**
     * Convert a Polynomial into a vector of basis co-efficients.
     * Underlying MatrixSystem should be read-locked before invoking.
     */
    class MomentRulebookToBasis {
    public:
        using output_t = Eigen::SparseMatrix<double>;

        const SymbolTable& symbols;
        const double zero_tolerance;

    public:
        explicit MomentRulebookToBasis(const PolynomialFactory& factory);

        explicit MomentRulebookToBasis(const SymbolTable& symbols, double zero_tolerance);

        output_t operator()(const MomentRulebook& rulebook) const;
    };

}