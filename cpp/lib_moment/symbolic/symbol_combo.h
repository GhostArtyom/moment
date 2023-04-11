/**
 * symbol_combo.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"

#include "symbol_expression.h"
#include "utilities/small_vector.h"

#include <algorithm>
#include <map>
#include <iosfwd>
#include <utility>
#include <vector>

namespace Moment {
    class SymbolTable;

    class SymbolCombo {
    public:
        /**
         * Storage for linear  combination of symbolic expressions.
         * Monomial on stack, polynomial on heap.
         * */
        using storage_t = SmallVector<SymbolExpression, 1>;

    private:
        storage_t data;

    public:

        SymbolCombo() = default;

        SymbolCombo(const SymbolCombo& rhs) = default;

        SymbolCombo(SymbolCombo&& rhs) = default;

        inline SymbolCombo& operator=(const SymbolCombo& rhs) = default;

        inline SymbolCombo& operator=(SymbolCombo&& rhs) noexcept = default;

        explicit SymbolCombo(storage_t input);

        explicit SymbolCombo(const std::map<symbol_name_t, double>& input);

        SymbolCombo(std::initializer_list<SymbolExpression> input)
            : SymbolCombo{storage_t{input}} { }

        [[nodiscard]] size_t size() const noexcept { return this->data.size(); }
        [[nodiscard]] bool empty() const noexcept { return this->data.empty(); }
        [[nodiscard]] auto begin() const noexcept { return this->data.cbegin(); }
        [[nodiscard]] auto end() const noexcept { return this->data.cend(); }
        [[nodiscard]] const SymbolExpression& operator[](size_t index) const noexcept { return this->data[index]; }



        SymbolCombo& operator+=(const SymbolCombo& rhs);

        [[nodiscard]] friend SymbolCombo operator+(const SymbolCombo& lhs, const SymbolCombo& rhs) {
            SymbolCombo output{lhs};
            output += rhs;
            return output;
        }

        SymbolCombo& operator*=(double factor) noexcept;

        [[nodiscard]] friend SymbolCombo operator*(SymbolCombo lhs, const double factor) noexcept {
            lhs *= factor;
            return lhs;
        }

        bool operator==(const SymbolCombo& rhs) const noexcept;

        inline bool operator!=(const SymbolCombo& rhs) const noexcept {
            return !(this->operator==(rhs));
        }

        /**
         * Transform this combo into its complex conjugate.
         */
        SymbolCombo& conjugate_in_place(const SymbolTable& symbols) noexcept;

        /**
         * Return a new SymbolCombo equal to the complex conjugate of this one.
         */
        [[nodiscard]] SymbolCombo conjugate(const SymbolTable& symbols) const {
            SymbolCombo output{*this};
            output.conjugate_in_place(symbols);
            return output;
        }

        /**
         * True if sum of symbols is Hermitian.
         * @param symbols Symbol table (needed to know which symbols are purely real/imaginary etc.).
         */
        [[nodiscard]] bool is_hermitian(const SymbolTable& symbols) const noexcept;

        /**
         * True if other is conjugate of this symbol combo.
         * @param symbols Symbol table (needed to know which symbols are purely real).
         * @param other SymbolCombo to compare against.
         * @return True if this and other are Hermitian conjugates of each other.
         */
        [[nodiscard]] bool is_conjugate(const SymbolTable& symbols, const SymbolCombo& other) const noexcept;

        /**
         * Construct an empty combination.
         */
        inline static SymbolCombo Zero() {
            return SymbolCombo{};
        }

        /**
         * Construct a combination representing a scalar.
         * @param the_factor The scalar value (default: 1.0).
         */
        inline static SymbolCombo Scalar(const double the_factor = 1.0) {
            return SymbolCombo(storage_t{SymbolExpression{1, the_factor , false}});
        }


        friend std::ostream& operator<<(std::ostream& os, const SymbolCombo& combo);

    };

}