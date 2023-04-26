/**
 * export_sequence_matrix.cpp
 *
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "export_sequence_matrix.h"

#include "matrix_system.h"
#include "matrix/operator_matrix.h"
#include "matrix/polynomial_matrix.h"

#include "symbolic/symbol_table.h"
#include "scenarios/context.h"

#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_matrix_system.h"
#include "scenarios/locality/locality_operator_formatter.h"

#include "scenarios/inflation/factor_table.h"
#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"

#include "utilities/reporting.h"

#include "error_codes.h"

#include "mex.hpp"

namespace Moment::mex {

    namespace {
        class DirectFormatView {
        public:
            using raw_const_iterator = SquareMatrix<OperatorSequence>::ColumnMajorView::TransposeIterator;

            class const_iterator {
            public:
                using iterator_category = std::input_iterator_tag;
                using difference_type = ptrdiff_t;
                using value_type = matlab::data::MATLABString;

            private:
                const Context* context = nullptr;
                DirectFormatView::raw_const_iterator raw_iter;

            public:
                constexpr const_iterator(const Context& context, raw_const_iterator rci)
                        : context{&context}, raw_iter{rci} { }


                constexpr bool operator==(const const_iterator& rhs) const noexcept { return this->raw_iter == rhs.raw_iter; }
                constexpr bool operator!=(const const_iterator& rhs)  const noexcept { return this->raw_iter != rhs.raw_iter; }

                constexpr const_iterator& operator++() {
                    ++(this->raw_iter);
                    return *this;
                }

                constexpr const_iterator operator++(int) & {
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                value_type operator*() const {
                    assert(context != nullptr);
                    return {matlab::engine::convertUTF8StringToUTF16String(context->format_sequence(*raw_iter))};
                }
            };

            static_assert(std::input_iterator<DirectFormatView::const_iterator>);

        private:
            const const_iterator iter_begin;
            const const_iterator iter_end;

        public:
            DirectFormatView(const Context &context, const SquareMatrix<OperatorSequence>& inputMatrix)
                : iter_begin{context, inputMatrix.ColumnMajor.begin()},
                  iter_end{context, inputMatrix.ColumnMajor.end()} {
            }

            [[nodiscard]] auto begin() const { return iter_begin; }
            [[nodiscard]] auto end() const { return iter_end; }

        };

        class LocalityFormatView {
        public:
            using raw_const_iterator = SquareMatrix<OperatorSequence>::ColumnMajorView::TransposeIterator;

            class const_iterator {
            public:
                using iterator_category = std::input_iterator_tag;
                using difference_type = ptrdiff_t;
                using value_type = matlab::data::MATLABString;

            private:
                const Locality::LocalityContext* context = nullptr;
                const Locality::LocalityOperatorFormatter* formatter = nullptr;
                DirectFormatView::raw_const_iterator raw_iter;

            public:
                constexpr const_iterator(const Locality::LocalityContext& context,
                                         const Locality::LocalityOperatorFormatter& formatter,
                                         raw_const_iterator rci)
                        : context{&context}, formatter{&formatter}, raw_iter{rci} { }


                constexpr bool operator==(const const_iterator& rhs) const noexcept { return this->raw_iter == rhs.raw_iter; }
                constexpr bool operator!=(const const_iterator& rhs)  const noexcept { return this->raw_iter != rhs.raw_iter; }

                constexpr const_iterator& operator++() {
                    ++(this->raw_iter);
                    return *this;
                }

                constexpr const_iterator operator++(int) & {
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                value_type operator*() const {
                    assert(context != nullptr);
                    assert(formatter != nullptr);
                    return {matlab::engine::convertUTF8StringToUTF16String(
                            context->format_sequence(*formatter, *raw_iter)
                            )};
                }
            };

            static_assert(std::input_iterator<DirectFormatView::const_iterator>);

        private:
            const const_iterator iter_begin;
            const const_iterator iter_end;
            const Locality::LocalityOperatorFormatter& formatter;

        public:
            LocalityFormatView(const Locality::LocalityContext &context,
                               const Locality::LocalityOperatorFormatter& formatter,
                               const SquareMatrix<OperatorSequence>& inputMatrix)
                : iter_begin{context, formatter, inputMatrix.ColumnMajor.begin()},
                  iter_end{context, formatter, inputMatrix.ColumnMajor.end()}, formatter{formatter} {
            }

            [[nodiscard]] auto begin() const { return iter_begin; }
            [[nodiscard]] auto end() const { return iter_end; }

        };

        class InferredFormatView {
        public:
            using raw_const_iterator = SquareMatrix<SymbolExpression>::ColumnMajorView::TransposeIterator;

            class const_iterator {
            public:
                using iterator_category = std::input_iterator_tag;
                using difference_type = ptrdiff_t;
                using value_type = matlab::data::MATLABString;

            private:
                const Context * context = nullptr;
                const SymbolTable * symbols = nullptr;
                InferredFormatView::raw_const_iterator raw_iter;

            public:
                constexpr const_iterator(const Context& context,
                                         const SymbolTable& symbols,
                                         raw_const_iterator rci)
                        : context{&context}, symbols{&symbols}, raw_iter{rci} { }

                constexpr bool operator==(const const_iterator& rhs) const noexcept { return this->raw_iter == rhs.raw_iter; }
                constexpr bool operator!=(const const_iterator& rhs)  const noexcept { return this->raw_iter != rhs.raw_iter; }

                constexpr const_iterator& operator++() {
                    ++(this->raw_iter);
                    return *this;
                }

                constexpr const_iterator operator++(int) & {
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                value_type operator*() const {
                    return {matlab::engine::convertUTF8StringToUTF16String(infer_one_symbol(*symbols, *raw_iter))};
                }

                [[nodiscard]] static std::string infer_one_symbol(const SymbolTable& symbols,
                                                                  const SymbolExpression& expr,
                                                                  bool with_prefix = false) {

                    std::stringstream ss;
                    if ((expr.id < 0) || (expr.id >= symbols.size())) {
                        if (with_prefix) {
                            ss << " + ";
                        }
                        ss << "[MISSING:" << expr.id << "]";
                        return ss.str();
                    }

                    const auto& symEntry = symbols[expr.id];

                    std::string symbol_str = expr.conjugated ? symEntry.formatted_sequence_conj()
                                                                   : symEntry.formatted_sequence();

                    if (1.0 == expr.factor) {
                        if (with_prefix) {
                            ss << " + ";
                        }
                        ss << symbol_str;
                        return ss.str();
                    } else if (0.0 == expr.factor) {
                        if (with_prefix) {
                            return "";
                        }
                        return "0";
                    }

                    if (-1.0 == expr.factor) {
                        if (with_prefix) {
                            ss << " - ";
                        } else {
                            ss << "-";
                        }
                        ss << symbol_str;
                    } else {
                        if (with_prefix) {
                            ss << " - " << (-expr.factor);
                        } else {
                            ss << expr.factor;
                        }
                        if (symEntry.Id() != 1) {
                            ss << symbol_str;
                        }
                    }
                    return ss.str();
                }
            };

            static_assert(std::input_iterator<DirectFormatView::const_iterator>);

        private:
            const const_iterator iter_begin;
            const const_iterator iter_end;

        public:
            InferredFormatView(const Context &context, const SymbolTable& symbols,
                             const SquareMatrix<SymbolExpression>& inputMatrix)
                : iter_begin{context, symbols, inputMatrix.ColumnMajor.begin()},
                  iter_end{context, symbols, inputMatrix.ColumnMajor.end()} {
            }

            [[nodiscard]] auto begin() const { return iter_begin; }
            [[nodiscard]] auto end() const { return iter_end; }

        };

        class InferredPolynomialFormatView {
        public:
            using raw_const_iterator = SquareMatrix<SymbolCombo>::ColumnMajorView::TransposeIterator;

            class const_iterator {
            public:
                using iterator_category = std::input_iterator_tag;
                using difference_type = ptrdiff_t;
                using value_type = matlab::data::MATLABString;

            private:
                const Context * context = nullptr;
                const SymbolTable * symbols = nullptr;
                InferredPolynomialFormatView::raw_const_iterator raw_iter;

            public:
                constexpr const_iterator(const Context& context,
                                         const SymbolTable& symbols,
                                         raw_const_iterator rci)
                        : context{&context}, symbols{&symbols}, raw_iter{rci} { }

                constexpr bool operator==(const const_iterator& rhs) const noexcept { return this->raw_iter == rhs.raw_iter; }
                constexpr bool operator!=(const const_iterator& rhs)  const noexcept { return this->raw_iter != rhs.raw_iter; }

                constexpr const_iterator& operator++() {
                    ++(this->raw_iter);
                    return *this;
                }

                constexpr const_iterator operator++(int) & {
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                value_type operator*() const {
                    bool done_once = false;
                    std::stringstream output;
                    for (const auto& expr : *raw_iter) {
                        output << InferredFormatView::const_iterator::infer_one_symbol(*symbols, expr, done_once);
                        done_once = true;
                    }
                    return {matlab::engine::convertUTF8StringToUTF16String(output.str())};
                }
            };

            static_assert(std::input_iterator<InferredPolynomialFormatView::const_iterator>);

        private:
            const const_iterator iter_begin;
            const const_iterator iter_end;

        public:
            InferredPolynomialFormatView(const Context &context, const SymbolTable& symbols,
                             const SquareMatrix<SymbolCombo>& inputMatrix)
                : iter_begin{context, symbols, inputMatrix.ColumnMajor.begin()},
                  iter_end{context, symbols, inputMatrix.ColumnMajor.end()} {
            }

            [[nodiscard]] auto begin() const { return iter_begin; }
            [[nodiscard]] auto end() const { return iter_end; }

        };


        class FactorFormatView {
        public:
            using raw_const_iterator = SquareMatrix<SymbolExpression>::ColumnMajorView::TransposeIterator;

            class const_iterator {
            public:
                using iterator_category = std::input_iterator_tag;
                using difference_type = ptrdiff_t;
                using value_type = matlab::data::MATLABString;

            private:
                const Inflation::InflationContext * context = nullptr;
                const Inflation::FactorTable * factors = nullptr;
                FactorFormatView::raw_const_iterator raw_iter;

            public:
                constexpr const_iterator(const Inflation::InflationContext& context,
                                         const Inflation::FactorTable& factors,
                                         raw_const_iterator rci)
                        : context{&context}, factors{&factors}, raw_iter{rci} { }

                constexpr bool operator==(const const_iterator& rhs) const noexcept { return this->raw_iter == rhs.raw_iter; }
                constexpr bool operator!=(const const_iterator& rhs)  const noexcept { return this->raw_iter != rhs.raw_iter; }

                constexpr const_iterator& operator++() {
                    ++(this->raw_iter);
                    return *this;
                }

                constexpr const_iterator operator++(int) & {
                    auto copy = *this;
                    ++(*this);
                    return copy;
                }

                value_type operator*() const {
                    assert(context != nullptr);

                    const auto id = raw_iter->id;
                    if ((raw_iter->id < 0) || (raw_iter->id >= factors->size())) {
                        std::stringstream ssErr;
                        ssErr << "[MISSING:" << id << "]";
                        return {matlab::engine::convertUTF8StringToUTF16String(ssErr.str())};
                    }
                    if (raw_iter->id == 0) {
                        return {u"0"};
                    }

                    const auto& facEntry = (*factors)[raw_iter->id];
                    if (1.0 == raw_iter->factor) {
                        return {matlab::engine::convertUTF8StringToUTF16String(facEntry.sequence_string())};
                    }

                    std::stringstream ss;
                    if (-1.0 == raw_iter->factor) {
                        ss << "-" << facEntry.sequence_string();
                    } else {
                        ss << raw_iter->factor;
                        if (facEntry.id != 1) {
                            ss << facEntry.sequence_string();
                        }
                    }

                    return {matlab::engine::convertUTF8StringToUTF16String(ss.str())};

                }
            };

            static_assert(std::input_iterator<DirectFormatView::const_iterator>);

        private:
            const const_iterator iter_begin;
            const const_iterator iter_end;

        public:
            FactorFormatView(const Inflation::InflationContext &context, const Inflation::FactorTable& factors,
                             const SquareMatrix<SymbolExpression>& inputMatrix)
                : iter_begin{context, factors, inputMatrix.ColumnMajor.begin()},
                  iter_end{context, factors, inputMatrix.ColumnMajor.end()} {
            }

            [[nodiscard]] auto begin() const { return iter_begin; }
            [[nodiscard]] auto end() const { return iter_end; }

        };


        template<class format_view_t, class matrix_data_t, typename... Args>
        matlab::data::Array do_export(matlab::engine::MATLABEngine& engine,
                                      const SquareMatrix<matrix_data_t>& inputMatrix,
                                      Args&... fv_extra_args) {

            matlab::data::ArrayFactory factory;
            matlab::data::ArrayDimensions array_dims{inputMatrix.dimension, inputMatrix.dimension};

            format_view_t formatView{std::forward<Args&>(fv_extra_args)..., inputMatrix};

            auto outputArray = factory.createArray<matlab::data::MATLABString>(std::move(array_dims));
            auto writeIter = outputArray.begin();
            auto readIter = formatView.begin();

            while ((writeIter != outputArray.end()) && (readIter != formatView.end())) {
                *writeIter = *readIter;
                ++writeIter;
                ++readIter;
            }
            if (writeIter != outputArray.end()) {
                throw_error(engine, errors::internal_error,
                            "export_symbol_matrix index count mismatch: too few input elements." );
            }
            if (readIter != formatView.end()) {
                throw_error(engine, errors::internal_error,
                            "export_symbol_matrix index count mismatch: too many input elements.");
            }

            return outputArray;
        }
    }


    matlab::data::Array SequenceMatrixExporter::operator()(const OperatorMatrix &op_matrix) const  {
        return this->export_direct(op_matrix);
    }

    matlab::data::Array SequenceMatrixExporter::operator()(const MonomialMatrix &inputMatrix,
                                                           const Locality::LocalityOperatorFormatter &formatter) const {

        // Get locality context, or throw
        const auto& localityContext = [&]() -> const Locality::LocalityContext& {
            try {
                return dynamic_cast<const Locality::LocalityContext&>(inputMatrix.context);
            } catch (const std::bad_cast& bce) {
                throw_error(this->engine, errors::internal_error,
                            "Supplied matrix was not part of a locality matrix system.");
            }
        }();

        // If no operator matrix, infer one:
        if (!inputMatrix.has_operator_matrix()) {
            return this->export_inferred(inputMatrix);
            //throw_error(this->engine, errors::internal_error,
            //            "Supplied matrix is not associated with an operator matrix.");
        }

        matlab::data::ArrayFactory factory;
        const size_t dimension = inputMatrix.Dimension();
        matlab::data::ArrayDimensions array_dims{dimension, dimension};

        LocalityFormatView formatView{localityContext, formatter, inputMatrix.operator_matrix()()};

        auto outputArray = factory.createArray<matlab::data::MATLABString>(std::move(array_dims));
        auto writeIter = outputArray.begin();
        auto readIter = formatView.begin();

        while ((writeIter != outputArray.end()) && (readIter != formatView.end())) {
            *writeIter = *readIter;
            ++writeIter;
            ++readIter;
        }
        if (writeIter != outputArray.end()) {
            throw_error(engine, errors::internal_error,
                        "export_symbol_matrix index count mismatch: too few input elements." );
        }
        if (readIter != formatView.end()) {
            throw_error(engine, errors::internal_error,
                        "export_symbol_matrix index count mismatch: too many input elements.");
        }

        return outputArray;
    }

    matlab::data::Array SequenceMatrixExporter::operator()(const MonomialMatrix &matrix,
                                                           const MatrixSystem& system) const {
        // Is this an inflation matrix? If so, display factorized format:
        const auto* inflSystem = dynamic_cast<const Inflation::InflationMatrixSystem*>(&system);
        if (nullptr != inflSystem) {
            return this->export_factored(inflSystem->InflationContext(), inflSystem->Factors(), matrix);
        }

        // Do we have direct sequences? If so, export direct (neutral) view.
        if (matrix.has_operator_matrix()) {
            const auto& op_mat = matrix.operator_matrix();

            return this->export_direct(op_mat);
        }

        // If all else fails, use inferred string formatting
        return this->export_inferred( matrix);
    }

    matlab::data::Array
    SequenceMatrixExporter::operator()(const PolynomialMatrix &matrix, const MatrixSystem &system) const {

        // Do we have direct sequences? If so, export direct (neutral) view.
        if (matrix.has_operator_matrix()) [[unlikely]] {
             // Unlikely: Most polynomial matrices are not created from categorizing symbols in an operator matrix.
            return this->export_direct(matrix.operator_matrix());
        }

        // Use inferred string formatting
        return this->export_inferred(matrix);
    }


    matlab::data::Array SequenceMatrixExporter::export_direct(const OperatorMatrix& opMatrix) const {
        return do_export<DirectFormatView>(this->engine, opMatrix(), opMatrix.context);
    }

    matlab::data::Array SequenceMatrixExporter::export_inferred(const MonomialMatrix& inputMatrix) const {
        return do_export<InferredFormatView>(this->engine, inputMatrix.SymbolMatrix(),
                                             inputMatrix.context, inputMatrix.Symbols);
    }

    matlab::data::Array SequenceMatrixExporter::export_inferred(const PolynomialMatrix& inputMatrix) const {
        return do_export<InferredPolynomialFormatView>(this->engine, inputMatrix.SymbolMatrix(),
                                                       inputMatrix.context, inputMatrix.Symbols);
    }

    matlab::data::Array
    SequenceMatrixExporter::export_factored(const Inflation::InflationContext& context,
                                            const Inflation::FactorTable& factors,
                                            const MonomialMatrix& inputMatrix) const {
        assert(&inputMatrix.context == &context);
         return do_export<FactorFormatView>(this->engine, inputMatrix.SymbolMatrix(),
                                           context, factors);
    }
}
