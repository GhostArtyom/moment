/**
 * monomial_matrix.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 *
 * @see monomial_matrix_basis.cpp for basis function definitions.
 */
#include "monomial_matrix.h"
#include "operator_matrix/operator_matrix.h"

#include "symbolic/symbol_table.h"
#include <stdexcept>

namespace Moment {

    namespace {
        /** Helper class, converts OSM -> Symbol matrix, registering new symbols */
        class OpSeqToSymbolConverter {
        private:
            const Context& context;
            SymbolTable& symbol_table;
            const OperatorMatrix::OpSeqMatrix& osm;
        public:
            const bool hermitian;
        public:
            OpSeqToSymbolConverter(const Context &context, SymbolTable &symbol_table,
                                   const OperatorMatrix::OpSeqMatrix &osm)
               : context{context}, symbol_table{symbol_table}, osm{osm}, hermitian{osm.is_hermitian()} { }


            std::unique_ptr<SquareMatrix<Monomial>> operator()() {
                auto unique_sequences = hermitian ? identify_unique_sequences_hermitian()
                                                  : identify_unique_sequences_generic();

                symbol_table.merge_in(std::move(unique_sequences));

                return hermitian ? build_symbol_matrix_hermitian()
                                 : build_symbol_matrix_generic();
            }

        private:

            [[nodiscard]] std::vector<Symbol> identify_unique_sequences_hermitian() const {
                std::vector<Symbol> build_unique;
                std::set<size_t> known_hashes;

                // First, always manually insert zero and one
                build_unique.emplace_back(Symbol::Zero(context));
                build_unique.emplace_back(Symbol::Identity(context));
                known_hashes.emplace(0);
                known_hashes.emplace(1);

                // Now, look at elements and see if they are unique or not
                for (size_t row = 0; row < osm.dimension; ++row) {
                    for (size_t col = row; col < osm.dimension; ++col) {
                        const auto& elem = osm[row][col];
                        const auto conj_elem = elem.conjugate();
                        int compare = OperatorSequence::compare_same_negation(elem, conj_elem);
                        const bool elem_hermitian = (compare == 1);

                        const size_t hash = elem.hash();
                        const size_t conj_hash = conj_elem.hash();

                        // Don't add what is already known
                        if (known_hashes.contains(hash) || (!elem_hermitian && known_hashes.contains(conj_hash))) {
                            continue;
                        }

                        if (elem_hermitian) {
                            build_unique.emplace_back(elem);
                            known_hashes.emplace(hash);
                        } else {
                            if (hash < conj_hash) {
                                build_unique.emplace_back(elem, conj_elem);
                            } else {
                                build_unique.emplace_back(conj_elem, elem);
                            }

                            known_hashes.emplace(hash);
                            known_hashes.emplace(conj_hash);
                        }
                    }
                }
                // NRVO?
                return build_unique;
            }

            [[nodiscard]] std::vector<Symbol> identify_unique_sequences_generic() const {
                std::vector<Symbol> build_unique;
                std::set<size_t> known_hashes;

                // First, always manually insert zero and one
                build_unique.emplace_back(Symbol::Zero(context));
                build_unique.emplace_back(Symbol::Identity(context));
                known_hashes.emplace(0);
                known_hashes.emplace(1);

                // Now, look at elements and see if they are unique or not
                for (size_t row = 0; row < osm.dimension; ++row) {
                    for (size_t col = 0; col < osm.dimension; ++col) {
                        const auto& elem = osm[row][col];
                        const auto conj_elem = elem.conjugate();
                        int compare = OperatorSequence::compare_same_negation(elem, conj_elem);
                        const bool elem_hermitian = (compare == 1);

                        const size_t hash = elem.hash();
                        const size_t conj_hash = conj_elem.hash();

                        // Don't add what is already known
                        if (known_hashes.contains(hash) || (!elem_hermitian && known_hashes.contains(conj_hash))) {
                            continue;
                        }

                        if (elem_hermitian) {
                            build_unique.emplace_back(elem);
                            known_hashes.emplace(hash);
                        } else {
                            if (hash < conj_hash) {
                                build_unique.emplace_back(elem, conj_elem);
                            } else {
                                build_unique.emplace_back(conj_elem, elem);
                            }

                            known_hashes.emplace(hash);
                            known_hashes.emplace(conj_hash);
                        }
                    }
                }

                // NRVO?
                return build_unique;
            }


            [[nodiscard]] std::unique_ptr<SquareMatrix<Monomial>> build_symbol_matrix_hermitian() const {
                std::vector<Monomial> symbolic_representation(osm.dimension * osm.dimension);

                for (size_t row = 0; row < osm.dimension; ++row) {
                    for (size_t col = row; col < osm.dimension; ++col) {
                        const size_t upper_index = (row * osm.dimension) + col;
                        const auto& elem = osm[row][col];
                        const size_t hash = elem.hash();
                        const bool negated = elem.negated();

                        auto [symbol_id, conjugated] = symbol_table.hash_to_index(hash);
                        if (symbol_id == std::numeric_limits<ptrdiff_t>::max()) {
                            std::stringstream ss;
                            ss << "Symbol \"" << osm[row][col] << "\" at index [" << row << "," << col << "]"
                               << " was not found in symbol table, while parsing Hermitian matrix.";
                            throw std::logic_error{ss.str()};
                        }
                        const auto& unique_elem = symbol_table[symbol_id];

                        symbolic_representation[upper_index] = Monomial{unique_elem.Id(), negated, conjugated};

                        // Make Hermitian, if off-diagonal
                        if (col > row) {
                            size_t lower_index = (col * osm.dimension) + row;
                            if (unique_elem.is_hermitian()) {
                                symbolic_representation[lower_index] = Monomial{unique_elem.Id(),
                                                                                negated, false};
                            } else {
                                symbolic_representation[lower_index] = Monomial{unique_elem.Id(),
                                                                                negated, !conjugated};
                            }
                        }
                    }
                }

                return std::make_unique<SquareMatrix<Monomial>>(osm.dimension, std::move(symbolic_representation));
            }

            [[nodiscard]] std::unique_ptr<SquareMatrix<Monomial>> build_symbol_matrix_generic() const {
                std::vector<Monomial> symbolic_representation(osm.dimension * osm.dimension);
                for (size_t row = 0; row < osm.dimension; ++row) {
                    for (size_t col = 0; col < osm.dimension; ++col) {
                        const size_t index = (row * osm.dimension) + col;
                        const auto& elem = osm[row][col];
                        const bool negated = elem.negated();
                        const size_t hash = elem.hash();

                        auto [symbol_id, conjugated] = symbol_table.hash_to_index(hash);
                        if (symbol_id == std::numeric_limits<ptrdiff_t>::max()) {
                            std::stringstream ss;
                            ss << "Symbol \"" << osm[row][col] << "\" at index [" << row << "," << col << "]"
                               << " was not found in symbol table.";
                            throw std::logic_error{ss.str()};
                        }
                        const auto& unique_elem = symbol_table[symbol_id];

                        symbolic_representation[index] = Monomial{unique_elem.Id(), negated, conjugated};
                    }
                }

                return std::make_unique<SquareMatrix<Monomial>>(osm.dimension,
                                                                std::move(symbolic_representation));
            }
        };
    }

    MonomialMatrix::MonomialMatrix(const Context& context, SymbolTable& symbols, const double zero_tolerance,
                                   std::unique_ptr<SquareMatrix<Monomial>> symbolMatrix,
                                   const bool constructed_as_hermitian)
        : Matrix{context, symbols, symbolMatrix ? symbolMatrix->dimension : 0},
            SymbolMatrix{*this}, sym_exp_matrix{std::move(symbolMatrix)}
        {
            if (!sym_exp_matrix) {
                throw std::runtime_error{"Symbol pointer passed to MonomialMatrix constructor was nullptr."};
            }

            // Count symbols
            this->MonomialMatrix::renumerate_bases(symbols, zero_tolerance);

            // Set  matrix properties
            this->description = "Monomial Symbolic Matrix";
            this->hermitian = constructed_as_hermitian;
    }

    MonomialMatrix::MonomialMatrix(SymbolTable &symbols, std::unique_ptr<OperatorMatrix> op_mat_ptr)
        : MonomialMatrix{op_mat_ptr->context, symbols, 1.0,
                         OpSeqToSymbolConverter{op_mat_ptr->context, symbols, (*op_mat_ptr)()}(),
                         op_mat_ptr->is_hermitian()} {
        assert(op_mat_ptr);
        this->op_mat = std::move(op_mat_ptr);

        // Count symbols
        this->MonomialMatrix::renumerate_bases(symbols, 1.0);

        // Set matrix properties
        this->op_mat->set_properties(*this);

    }

    MonomialMatrix::~MonomialMatrix() noexcept = default;

    void MonomialMatrix::renumerate_bases(const SymbolTable &symbols, double zero_tolerance) {
        for (auto& symbol : *this->sym_exp_matrix) {
            // Make conjugation status canonical:~
            if (symbol.conjugated) {
                const auto& ref_symbol = symbols[symbol.id];
                if (ref_symbol.is_hermitian()) {
                    symbol.conjugated = false;
                } else if (ref_symbol.is_antihermitian()) {
                    symbol.conjugated = false;
                    symbol.factor *= -1.0;
                }
            }
            // If zero, replace with canonical zero.
            if (approximately_zero(symbol.factor, zero_tolerance)) {
                symbol.id = 0;
                symbol.conjugated = false;
                symbol.factor = 0;
            }
        }

        this->identify_symbols_and_basis_indices();
    }

    void MonomialMatrix::identify_symbols_and_basis_indices() {
        // Find and canonicalize included symbols
        const size_t max_symbol_id = symbols.size();
        this->complex_coefficients = false;
        this->included_symbols.clear();
        for (auto& x : *sym_exp_matrix) {
            assert(x.id < max_symbol_id);
            this->included_symbols.emplace(x.id);
            if (!this->complex_coefficients && x.complex_factor()) { // <- first clause, avoid unnecessary tests
                this->complex_coefficients = true;
            }
        }

        // All included symbols:~
        this->real_basis_elements.clear();
        this->imaginary_basis_elements.clear();
        this->basis_key.clear();
        for (const auto symbol_id : this->included_symbols) {
            auto &symbol_info = this->symbols[symbol_id];
            auto [re_key, im_key] = symbol_info.basis_key();
            if (re_key >= 0) {
                this->real_basis_elements.emplace(re_key);
            }
            if (im_key >= 0) {
                this->imaginary_basis_elements.emplace(im_key);
            }
            this->basis_key.emplace_hint(this->basis_key.end(),
                                         std::make_pair(symbol_id, std::make_pair(re_key, im_key)));
        }

        this->complex_basis = !this->imaginary_basis_elements.empty();
    };

}