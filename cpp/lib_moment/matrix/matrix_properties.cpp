/**
 * symbol_matrix_properties.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "matrix_properties.h"

#include "symbolic_matrix.h"
#include "symbolic/symbol_table.h"
#include "symbolic/symbol_set.h"

#include <iostream>


namespace Moment {
    MatrixProperties::MatrixProperties(const SymbolicMatrix& matrix, const SymbolTable& table,
                                       std::set<symbol_name_t>&& included)
            : dimension{matrix.Dimension()}, included_symbols{std::move(included)}, mat_is_herm{matrix.IsHermitian()} {

        this->rebuild_keys(table);
    }

    void MatrixProperties::rebuild_keys(const SymbolTable& table) {
        this->real_entries.clear();
        this->imaginary_entries.clear();
        this->elem_keys.clear();

        for (const auto& id : this->included_symbols) {
            const auto& unique_symbol = table[id];
            assert(id == unique_symbol.Id());

            if (!unique_symbol.is_antihermitian()) {
                this->real_entries.insert(id);
            }
            if (!unique_symbol.is_hermitian()) {
                this->imaginary_entries.insert(id);
            }

            this->elem_keys.insert(this->elem_keys.end(), std::make_pair(id, unique_symbol.basis_key()));
        }

        // Matrix type depends on whether there are imaginary symbols or not
        const bool has_imaginary = !this->imaginary_entries.empty();
        if (has_imaginary) {
            this->basis_type = this->mat_is_herm ? MatrixType::Hermitian : MatrixType::Complex;
        } else {
            this->basis_type = this->mat_is_herm ? MatrixType::Symmetric : MatrixType::Real;
        }
    }

    MatrixProperties::MatrixProperties(size_t dim, MatrixType type, const SymbolSet &entries)
        :  dimension{dim}, basis_type{type} {

        ptrdiff_t real_count = 0;
        ptrdiff_t im_count = 0;

        // Go through symbol table... [should be sorted...]
        for (const auto& [id, symbol] : entries) {
            // Skip 0 symbol.
            if (id == 0) {
                continue;
            }

            ptrdiff_t re_index = -1;
            ptrdiff_t im_index = -1;

            if (!symbol.real_is_zero) {
                re_index = real_count++;
                this->real_entries.insert(real_entries.end(), symbol.id);
            }

            if (!symbol.im_is_zero) {
                im_index = im_count++;
                this->imaginary_entries.insert(imaginary_entries.end(), symbol.id);
            }

            // Inferred basis maps, ignoring 0 element.
            this->elem_keys.insert(this->elem_keys.end(),
                                   std::make_pair(symbol.id, std::make_pair(re_index, im_index)));
        }

        // Unknown matrix type depends on whether there are imaginary symbols
        if (this->basis_type == MatrixType::Unknown) {
            this->basis_type = this->imaginary_entries.empty() ? MatrixType::Symmetric : MatrixType::Hermitian;
        }
    }


    std::ostream& operator<<(std::ostream& os, const MatrixProperties& mp) {
        os << mp.dimension << "x" << mp.dimension << " ";
        if (mp.is_complex()) {
            if (mp.is_hermitian()) {
                os << "Hermitian matrix";
            } else {
                os << "Complex matrix";
            }
        } else {
            if (mp.is_hermitian()) {
                os << "Symmetric matrix";
            } else {
                os << "Real matrix";
            }
        }
        const auto num_us = mp.included_symbols.size();
        os << " with "
           << num_us << " unique " << (num_us != 1 ? "symbols" : "symbol");
        const auto num_re = mp.real_entries.size();
        if (num_re > 0) {
            os << ", " << num_re << " real";
        }
        const auto num_im = mp.imaginary_entries.size();
        if (num_im > 0) {
            os << ", " << num_im << " imaginary";
        }

        os << ".";

        return os;
    }
}