/**
 * symmetrized_matrix_system_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/derived/symbol_table_map.h"
#include "scenarios/derived/lu_map_core_processor.h"

#include "scenarios/symmetrized/group.h"
#include "scenarios/symmetrized/representation.h"
#include "scenarios/symmetrized/symmetrized_matrix_system.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"
#include "scenarios/algebraic/name_table.h"

#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_matrix_system.h"

#include "symbolic/symbol_table.h"

#include "../sparse_utils.h"

#include <array>
#include <set>
#include <sstream>
#include <stdexcept>


namespace Moment::Tests {

    namespace {
        // Find CHSH symbols:
        symbol_name_t find_symbol(const Locality::LocalityContext& context, const SymbolTable& symbols,
                                    std::initializer_list<oper_name_t> ops) {
            OperatorSequence opSeq{ops, context};
            auto symPtr = symbols.where(opSeq);
            if (symPtr != nullptr) {
                return symPtr->Id();
            }

            std::stringstream errSS;
            errSS << "Could not find symbol for " << opSeq;
            throw std::runtime_error{errSS.str()};
        };


        std::array<symbol_name_t, 10> get_chsh_symbol_ids(const Locality::LocalityContext& context,
                                                         const SymbolTable& symbols) {
            if (context.Parties.size() != 2) {
                throw std::runtime_error{"Two parties expected."};
            }
            const auto& alice = context.Parties[0];
            const auto& bob = context.Parties[1];
            if (alice.size() != 2) {
                throw std::runtime_error{"Alice should have two operators."};
            }
            if (bob.size() != 2) {
                throw std::runtime_error{"Bob should have two operators."};
            }

            auto a0 = find_symbol(context, symbols, {alice[0]});
            auto a1 = find_symbol(context, symbols, {alice[1]});
            auto b0 = find_symbol(context, symbols, {bob[0]});
            auto b1 = find_symbol(context, symbols, {bob[1]});

            auto a0a1 = find_symbol(context, symbols, {alice[0], alice[1]});
            auto a0b0 = find_symbol(context, symbols, {alice[0], bob[0]});
            auto a0b1 = find_symbol(context, symbols, {alice[0], bob[1]});
            auto a1b0 = find_symbol(context, symbols, {alice[1], bob[0]});
            auto a1b1 = find_symbol(context, symbols, {alice[1], bob[1]});
            auto b0b1 = find_symbol(context, symbols, {bob[0], bob[1]});

            auto output = std::array<symbol_name_t, 10>{a0, a1, b0, b1, a0a1, a0b0, a0b1, a1b0, a1b1, b0b1};

            std::set check_unique(output.begin(), output.end());
            if (check_unique.size() != 10) {
                throw std::runtime_error{"All 10 symbols should be unique."};
            }

            return output;
        }


    }

    using namespace Moment::Symmetrized;

    TEST(Scenarios_Symmetry_MatrixSystem, Algebraic_Z2) {
        // Two variables, a & b
        auto amsPtr = std::make_shared<Algebraic::AlgebraicMatrixSystem>(
            std::make_unique<Algebraic::AlgebraicContext>(Algebraic::NameTable{"a", "b"})
        );
        auto& ams = *amsPtr;
        auto& context = ams.Context();
        ams.generate_dictionary(2);

        // Z2 symmetry; e.g. max "a + b" subject to "a + b < 10"
        std::vector<Eigen::SparseMatrix<double>> generators;
        generators.emplace_back(make_sparse(3, {1, 0, 0,
                                                0, 0, 1,
                                                0, 1, 0}));

        auto group_elems = Group::dimino_generation(generators);
        auto base_rep = std::make_unique<Representation>(1, std::move(group_elems));
        auto group = std::make_unique<Group>(context, std::move(base_rep));

        SymmetrizedMatrixSystem sms{amsPtr, std::move(group), 2, std::make_unique<Derived::LUMapCoreProcessor>()};

        ASSERT_EQ(&ams, &sms.base_system());
    }

    TEST(Scenarios_Symmetry_MatrixSystem, Locality_CHSH) {
        // Two variables, a & b
        auto lmsPtr = std::make_shared<Locality::LocalityMatrixSystem>(
            std::make_unique<Locality::LocalityContext>(Locality::Party::MakeList(2, 2, 2))
        );
        auto& lms = *lmsPtr;
        auto& locality_context = lms.localityContext;
        auto& locality_symbols = lms.Symbols();
        lms.generate_dictionary(2);

        // Get CHSH symbols
        auto [a0, a1, b0, b1, a0a1, a0b0, a0b1, a1b0, a1b1, b0b1] =
                get_chsh_symbol_ids(locality_context, locality_symbols);


        // Standard CHSH inequality symmetry
        std::vector<Eigen::SparseMatrix<double>> generators;
        generators.emplace_back(make_sparse(5, {1, 1, 0, 0, 0,
                                                0, 0, 0, 1, 0,
                                                0, 0, 0, 0, 1,
                                                0, 0, 1, 0, 0,
                                                0, -1,0, 0, 0}));
        generators.emplace_back(make_sparse(5, {1, 0, 0, 0, 0,
                                                0, 0, 0, 0, 1,
                                                0, 0, 0, 1, 0,
                                                0, 0, 1, 0, 0,
                                                0, 1, 0, 0, 0}));

        auto group_elems = Group::dimino_generation(generators);
        auto base_rep = std::make_unique<Representation>(1, std::move(group_elems));
        auto group = std::make_unique<Group>(locality_context, std::move(base_rep));

        ASSERT_EQ(group->size, 16);
        SymmetrizedMatrixSystem sms{lmsPtr, std::move(group), 2, std::make_unique<Derived::LUMapCoreProcessor>()};
        ASSERT_EQ(&lms, &sms.base_system());
        const auto& sym_symbols = sms.Symbols();

        const auto& map = sms.map();
        ASSERT_EQ(locality_symbols.size(), map.fwd_size()) << lms.Symbols(); // All symbols mapped
        EXPECT_EQ(map.inv_size(), 3); // 0, 1, 'y'
        ASSERT_EQ(sym_symbols.size(), 3) << sms.Symbols();

        // Check inverse map
        EXPECT_EQ(map.inverse(0), SymbolCombo::Zero());
        EXPECT_EQ(map.inverse(1), SymbolCombo::Scalar(1.0));
        SymbolCombo expected_new_symbol{SymbolExpression{a1, -0.25},
                                        SymbolExpression{b0, -0.25},
                                        SymbolExpression{a0b0, +0.25},
                                        SymbolExpression{a0b1, -0.25},
                                        SymbolExpression{a1b0, +0.25},
                                        SymbolExpression{a1b1, +0.25}};
        EXPECT_EQ(map.inverse(2), expected_new_symbol);

        // Check forward map
        ASSERT_EQ(map.fwd_size(), 12);
        EXPECT_EQ(map(0), SymbolCombo::Zero());
        EXPECT_EQ(map(1), SymbolCombo::Scalar(1.0));
        EXPECT_EQ(map(a0), SymbolCombo::Scalar(0.5));
        EXPECT_EQ(map(a1), SymbolCombo::Scalar(0.5));
        EXPECT_EQ(map(b0), SymbolCombo::Scalar(0.5));
        EXPECT_EQ(map(b1), SymbolCombo::Scalar(0.5));
        EXPECT_EQ(map(a0a1), SymbolCombo::Scalar(0.25));
        EXPECT_EQ(map(b0b1), SymbolCombo::Scalar(0.25));
        EXPECT_EQ(map(a0b0), SymbolCombo({SymbolExpression(1, 0.375), SymbolExpression(2, 1.0)}));
        EXPECT_EQ(map(a0b1), SymbolCombo({SymbolExpression(1, 0.125), SymbolExpression(2, -1.0)}));
        EXPECT_EQ(map(a1b0), SymbolCombo({SymbolExpression(1, 0.375), SymbolExpression(2, 1.0)}));
        EXPECT_EQ(map(a1b1), SymbolCombo({SymbolExpression(1, 0.375), SymbolExpression(2, 1.0)}));

    }
}