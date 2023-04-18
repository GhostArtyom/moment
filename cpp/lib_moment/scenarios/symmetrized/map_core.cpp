/**
 * map_core.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "map_core.h"
#include "symbolic/symbol_table.h"

#include <cmath>

#include <limits>

namespace Moment::Symmetrized {
    namespace {
        constexpr bool is_close(const double x, const double y, const double eps_mult = 1.0) {
            return std::abs(x - y)
                < eps_mult * std::numeric_limits<double>::epsilon() * std::max(std::abs(x), std::abs(y));
        }
    }


    MapCore::MapCore(const SymbolTable &origin_symbols, const Eigen::MatrixXd &raw_remap, const double zero_tolerance)
            : nontrivial_rows{static_cast<size_t>(raw_remap.rows()), false},
              nontrivial_cols{static_cast<size_t>(raw_remap.cols()), true} {

        this->nontrivial_cols[0] = false;
        this->nontrivial_rows[0] = true;


        // Check first column maps ID->ID
        if (!is_close(raw_remap.coeff(0,0), 1.0)) {
            throw std::range_error{"First column of transformation must map identity to the identity."};
        }
        auto col_has_any_non_constant = [&raw_remap,&zero_tolerance](Eigen::Index col) -> bool {
            auto row_iter = Eigen::MatrixXd::InnerIterator{raw_remap, col};
            ++row_iter;
            while (row_iter) {
                if (std::abs(row_iter.value()) > zero_tolerance) {
                    return true;
                }
                ++row_iter;
            }
            return false;
        };
        if (col_has_any_non_constant(0)) {
            throw std::range_error{"First column of transformation must map identity to the identity."};
        }

        for (int col_index = 1; col_index < raw_remap.cols(); ++col_index) {

            // Identify complex conjugate rows. TODO: remove SymbolTable dependency.
            auto [symbol_id, conjugated] = origin_symbols.OSGIndex(col_index);
            if (conjugated) {
                this->nontrivial_cols[col_index] = false;
                this->conjugates.emplace(col_index);
                continue;
            }

            // Identify rows with no values, or only a constant value:
            const bool hasConstant = std::abs(raw_remap(0, col_index)) > zero_tolerance;
            const bool hasAnythingElse = col_has_any_non_constant(col_index);
            if (!hasAnythingElse) {
                if (!hasConstant) {
                    this->constants.emplace(std::make_pair(col_index, 0));
                    this->nontrivial_cols[col_index] = false;
                    continue;
                } else {
                    const double offset_term = raw_remap.coeff(0, col_index);
                    this->constants.emplace(std::make_pair(col_index, offset_term));
                    this->nontrivial_cols[col_index] = false;
                    continue;
                }
            }

            // Otherwise, column is nontrivial - identify rows that are nontrivial
            for (auto row_iter = Eigen::MatrixXd::InnerIterator{raw_remap, col_index};
                 row_iter; ++row_iter) {
                if (std::abs(row_iter.value()) > zero_tolerance) {
                    this->nontrivial_rows[row_iter.row()] = true;
                }
            }
        }

        // Manually copy top row
        this->nontrivial_rows[0] = false;
        auto remapped_cols = static_cast<Eigen::Index>(this->nontrivial_cols.count());
        auto remapped_rows = static_cast<Eigen::Index>(this->nontrivial_rows.count());

        // Copy dense matrix (extracting constant row separately) and pruning close to zero with zero.
        this->core_offset.resize(remapped_cols);
        this->core.resize(remapped_rows, remapped_cols);
        auto *offset_write_iter = this->core_offset.data();
        auto *write_iter = this->core.data();
        for (const auto old_col_idx : this->nontrivial_cols) {
            double c_read_val = raw_remap.coeff(0, static_cast<Eigen::Index>(old_col_idx));
            *(offset_write_iter) = (abs(c_read_val) > zero_tolerance) ? c_read_val : 0.0;
            ++offset_write_iter;
            for (const auto old_row_idx : this->nontrivial_rows) {
                double read_val = raw_remap.coeff(static_cast<Eigen::Index>(old_row_idx),
                                                    static_cast<Eigen::Index>(old_col_idx));
                *(write_iter) = (abs(read_val) > zero_tolerance) ? read_val : 0.0;
                ++write_iter;
            }
        }
    }


    MapCore::MapCore(const SymbolTable& origin_symbols, const Eigen::SparseMatrix<double>& raw_remap)
            : nontrivial_rows{static_cast<size_t>(raw_remap.rows()), false},
              nontrivial_cols{static_cast<size_t>(raw_remap.cols()), true} {

        this->nontrivial_cols[0] = false;
        this->nontrivial_rows[0] = true;

        // Check first column maps ID->ID
        if ((raw_remap.col(0).nonZeros() != 1) || !is_close(raw_remap.coeff(0,0), 1.0)) {
            throw std::range_error{"First column of transformation must map identity to the identity."};
        }

        for (int col_index = 1; col_index < raw_remap.cols(); ++col_index) {

            // Identify complex conjugate rows. TODO: remove SymbolTable dependency.
            auto [symbol_id, conjugated] = origin_symbols.OSGIndex(col_index);
            if (conjugated) {
                this->nontrivial_cols[col_index] = false;
                this->conjugates.emplace(col_index);
                continue;
            }

            // Identify rows with no values, or only a constant value:
            Eigen::Index nnz = raw_remap.col(col_index).nonZeros();
            if (0 == nnz) {
                this->constants.emplace(std::make_pair(col_index, 0));
                this->nontrivial_cols[col_index] = false;
                continue;
            } else if (1 == nnz) {
                const double offset_term = raw_remap.coeff(0, col_index);
                if (offset_term > 0) {
                    this->constants.emplace(std::make_pair(col_index, offset_term));
                    this->nontrivial_cols[col_index] = false;
                    continue;
                }
            }

            // Otherwise, column is nontrivial - identify rows that are nontrivial
            for (auto row_iter = Eigen::SparseMatrix<double>::InnerIterator{raw_remap, col_index};
                    row_iter; ++row_iter) {
                this->nontrivial_rows[row_iter.row()] = true;
            }
        }

        // Manually copy top row
        this->nontrivial_rows[0] = false;
        auto remapped_cols = static_cast<Eigen::Index>(this->nontrivial_cols.count());
        auto remapped_rows = static_cast<Eigen::Index>(this->nontrivial_rows.count());

        // Copy dense matrix (extracting constant row separately)
        this->core_offset.resize(remapped_cols);
        this->core.resize(remapped_rows, remapped_cols);
        auto *offset_write_iter = this->core_offset.data();
        auto *write_iter = this->core.data();
        for (const auto old_col_idx : this->nontrivial_cols) {
            *(offset_write_iter) = raw_remap.coeff(0, static_cast<Eigen::Index>(old_col_idx));
            ++offset_write_iter;
            for (const auto old_row_idx : this->nontrivial_rows) {
                *(write_iter) = raw_remap.coeff(static_cast<Eigen::Index>(old_row_idx),
                                                static_cast<Eigen::Index>(old_col_idx));
                ++write_iter;
            }
        }
    }

}