/**
 * composite_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "monomial_matrix.h"
#include "polynomial_matrix.h"

#include "dictionary/raw_polynomial.h"

#include "matrix_system/matrix_indices.h"

#include "multithreading/maintains_mutex.h"
#include "multithreading/multithreading.h"

#include <cassert>

#include <complex>
#include <concepts>
#include <memory>
#include <vector>

namespace Moment {
    class MatrixSystem;

    /**
     * A polynomial matrix formed by summing together a collection of other matrices.
     */
    class CompositeMatrix : public PolynomialMatrix {
    public:
        struct ConstituentInfo {
        public:
            /** The size of the matrix */
            size_t matrix_dimension;

            /** Pointers to the elements of the matrix */
            std::vector<std::pair<const SymbolicMatrix*, std::complex<double>>> elements;

            /** Delete copy constructor. */
            ConstituentInfo(const ConstituentInfo& rhs) = delete;

            /** Default move constructor. */
            ConstituentInfo(ConstituentInfo&& rhs) = default;

            /** Construct empty collection. */
            explicit ConstituentInfo(size_t dim = 0) : matrix_dimension{dim} { }

            /** Construct 'collection' of one single matrix. */
            explicit ConstituentInfo(const SymbolicMatrix& input, std::complex<double> scale = {1.0, 0.0})
                : matrix_dimension{input.Dimension()} {
                this->elements.emplace_back(&input, scale);
            }

            /** Do not construct collection including pointer to transient r-value. */
            explicit ConstituentInfo(SymbolicMatrix&& input) = delete;

            /** Size of constituents is number of elements. */
            [[nodiscard]] inline size_t size() const noexcept {
                return this->elements.size();
            }

            /** Constituents are an empty collection if it has no elements. */
            [[nodiscard]] inline bool empty() const noexcept {
                return this->elements.empty();
            }

            /** Attempt to set dimension automatically; returns false if could not */
            bool auto_set_dimension() noexcept {
                // Cannot infer dimension if no matrices added
                if (this->elements.empty()) {
                    return false;
                }

                // Otherwise, first element sets dimension
                assert(this->elements.front().first);
                this->matrix_dimension = this->elements.front().first->Dimension();
                return true;
            }

        };

    protected:
        ConstituentInfo constituents;

    public:
        /** Constructor for non-empty polynomial localizing matrix. */
        CompositeMatrix(const Context& context, SymbolTable& symbols,
                        const PolynomialFactory& factory, ConstituentInfo&& constituents);

    protected:
        /**
         * Synthesize constituent data into a single polynomial matrix data object.
         */
        [[nodiscard]] static std::unique_ptr<PolynomialMatrix::MatrixData>
        compile_to_polynomial_matrix_data(const PolynomialFactory& factory,
                                          const ConstituentInfo& constituents);
    };

    /**
     * Generic implementation of composition of monomial matrix of a particular type into its polynomial equivalent.
     *
     * @tparam matrix_system_t The matrix system type
     * @tparam polynomial_index_t The polynomial index type.
     * @tparam monomial_indices_t The index storage bank type, that produces the constituent monomial matrices.
     * @tparam realized_type
     */
    template<typename matrix_system_t,
             typename polynomial_index_t,
             typename monomial_indices_t>
    class CompositeMatrixImpl : public CompositeMatrix {
    public:
        using ImplType = CompositeMatrixImpl<matrix_system_t, polynomial_index_t, monomial_indices_t>;
        using PolynomialIndex = polynomial_index_t;
        using MonomialIndex = typename polynomial_index_t::ComponentIndex;
        using OSGIndex = typename polynomial_index_t::OSGIndex;


        /** Full index that defines this polynomial matrix */
        const PolynomialIndex index;

    public:
        CompositeMatrixImpl(const Context& context, SymbolTable& symbols, const PolynomialFactory& factory,
                            PolynomialIndex index_in, CompositeMatrix::ConstituentInfo&& constituents_in)
                : CompositeMatrix{context, symbols, factory, std::move(constituents_in)}, index{std::move(index_in)} {
            this->description = index.to_string(context, symbols);
        }

        /**
        * Constructs a polynomial matrix from a Polynomial, invoking the construction of any necessary components.
        * @param write_lock A locked write lock for the system.
        * @param system The matrix system.
        * @param monomial_matrices The indexing object for the monomial matrices that will be composited.
        * @param polynomial_index The index of the polynomial matrix to construct.
        * @param mt_policy The multi-threading policy to use (potentially relevant in the generation of components).
        * @return A newly created polynomial matrix.
        */
        static std::unique_ptr<ImplType>
        create(const MaintainsMutex::WriteLock& write_lock,
               matrix_system_t& system, monomial_indices_t& monomial_matrices,
               const PolynomialIndex& polynomial_index,
               Multithreading::MultiThreadPolicy mt_policy) {
            assert(system.is_locked_write_lock(write_lock));

            // System parts:
            const auto& context = system.Context();
            auto& symbols = system.Symbols();
            const auto& poly_factory = system.polynomial_factory();

            // First ensure constituent parts exist
            CompositeMatrix::ConstituentInfo constituents;
            constituents.elements.reserve(polynomial_index.Polynomial.size());
            for (auto [mono_index, factor] : polynomial_index.MonomialIndices(symbols)) {
                auto [mono_offset, mono_matrix] = monomial_matrices.create(write_lock, mono_index, mt_policy);
                constituents.elements.emplace_back(&mono_matrix, factor);
            }

            // If no constituents, we have to query for size in another way:
            if (!constituents.auto_set_dimension()) {
                constituents.matrix_dimension = system.osg_size(polynomial_index.Level);
            }

            return std::make_unique<ImplType>(context, symbols, system.polynomial_factory(),
                                              polynomial_index,
                                              std::move(constituents));
        }

        /**
         * Constructs a polynomial matrix from a RawPolynomial, invoking the construction of any necessary components.
         * @param write_lock A locked write lock for the system.
         * @param system The matrix system.
         * @param monomial_matrices The indexing object for the monomial matrices that will be composited.
         * @param osg_index The OSG index for the matrices.
         * @param raw_polynomials The raw polynomial to construct.
         * @param mt_policy The multi-threading policy to use (potentially relevant in the generation of components).
         * @return A newly created polynomial matrix.
         */
        static std::unique_ptr<ImplType>
        create_from_raw(const MaintainsMutex::WriteLock& write_lock,
                        matrix_system_t& system,
                        monomial_indices_t& monomial_matrices,
                        OSGIndex osg_index, const RawPolynomial& raw_polynomial,
                        Multithreading::MultiThreadPolicy mt_policy) {
            assert(system.is_locked_write_lock(write_lock));

            // First invoke factory to ensure constituent parts exist
            CompositeMatrix::ConstituentInfo constituents;
            constituents.elements.reserve(raw_polynomial.size());
            for (auto& [op_seq, factor] : raw_polynomial) {
                auto [mono_offset, mono_matrix] = monomial_matrices.create(write_lock,
                                                                           MonomialIndex{osg_index, op_seq},
                                                                           mt_policy);
                constituents.elements.emplace_back(&mono_matrix, factor);
            }

            if (!constituents.auto_set_dimension()) {
                constituents.matrix_dimension = system.osg_size(osg_index);
            }

            // Now, make raw matrix from this
            const auto& context = system.Context();
            auto& symbols = system.Symbols();
            const auto& poly_factory = system.polynomial_factory();

            return std::make_unique<ImplType>(context, symbols, system.polynomial_factory(),
                                              PolynomialIndex{osg_index,
                                                              raw_polynomial.to_polynomial(poly_factory, symbols)},
                                              std::move(constituents));
        }
    };



}