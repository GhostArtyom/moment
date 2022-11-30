/**
 * explicit_symbols_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "operators/matrix/moment_matrix.h"
#include "operators/locality/locality_context.h"
#include "operators/locality/locality_matrix_system.h"
#include "operators/locality/locality_explicit_symbols.h"

#include "operators/inflation/inflation_context.h"
#include "operators/inflation/inflation_matrix_system.h"

#include <memory>

namespace NPATK::Tests {

    TEST(ExplicitSymbols, OnePartyOneMeasurementThreeOutcomes) {

        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(1, 1, 3))};
        const auto& context = system.localityContext;
        auto [id, momentMatrix] = system.create_moment_matrix(1);

        const auto& alice = context.Parties[0];
        auto a0_loc = momentMatrix.Symbols.where(
                        OperatorSequence({alice.measurement_outcome(0, 0)}, context));
        ASSERT_NE(a0_loc, nullptr);
        auto a1_loc = momentMatrix.Symbols.where(
                        OperatorSequence({alice.measurement_outcome(0, 1)}, context));
        ASSERT_NE(a1_loc, nullptr);
        ASSERT_NE(a0_loc->Id(), a1_loc->Id());

        const ExplicitSymbolIndex& cgForm = system.ExplicitSymbolTable();
        const ExplicitSymbolIndex& cgForm2 = system.ExplicitSymbolTable();
        EXPECT_EQ(&cgForm, &cgForm2);
        ASSERT_EQ(cgForm.Level, 1);

        std::vector<size_t> empty{};
        auto idSpan = cgForm.get(empty);
        ASSERT_EQ(idSpan.size(), 1);
        EXPECT_EQ(idSpan[0].symbol_id, 1);

        std::vector<size_t> a_index{0};
        auto aSpan = cgForm.get(a_index);
        ASSERT_EQ(aSpan.size(), 2);
        EXPECT_EQ(aSpan[0].symbol_id, a0_loc->Id());
        EXPECT_EQ(aSpan[1].symbol_id, a1_loc->Id());
    }

    TEST(ExplicitSymbols, TwoPartyTwoMeasurementTwoOutcomes) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 2, 2))};
        const auto& context = system.localityContext;
        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];


        auto [id, momentMatrix] = system.create_moment_matrix(1);

        auto alice_a0 = momentMatrix.Symbols.where(
                        OperatorSequence({alice.measurement_outcome(0, 0)}, context));
        ASSERT_NE(alice_a0, nullptr);
        auto alice_b0 = momentMatrix.Symbols.where(
                        OperatorSequence({alice.measurement_outcome(1, 0)}, context));
        ASSERT_NE(alice_b0, nullptr);
        auto bob_a0 = momentMatrix.Symbols.where(
                        OperatorSequence({bob.measurement_outcome(0, 0)}, context));
        ASSERT_NE(bob_a0, nullptr);
        auto bob_b0 = momentMatrix.Symbols.where(
                        OperatorSequence({bob.measurement_outcome(1, 0)}, context));
        ASSERT_NE(bob_b0, nullptr);

        auto alice_a0_bob_a0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 0), bob.measurement_outcome(0, 0)}, context));
        ASSERT_NE(alice_a0_bob_a0, nullptr);
        auto alice_a0_bob_b0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 0), bob.measurement_outcome(1, 0)}, context));
        ASSERT_NE(alice_a0_bob_b0, nullptr);
        auto alice_b0_bob_a0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 0), bob.measurement_outcome(0, 0)}, context));
        ASSERT_NE(alice_b0_bob_a0, nullptr);
        auto alice_b0_bob_b0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 0), bob.measurement_outcome(1, 0)}, context));
        ASSERT_NE(alice_b0_bob_b0, nullptr);

        const ExplicitSymbolIndex& cgForm = system.ExplicitSymbolTable();
        ASSERT_EQ(cgForm.Level, 2);

        auto id_span = cgForm.get({});
        ASSERT_EQ(id_span.size(), 1);
        EXPECT_EQ(id_span[0].symbol_id, 1);

        auto alice_a_span = cgForm.get({0});
        ASSERT_EQ(alice_a_span.size(), 1);
        EXPECT_EQ(alice_a_span[0].symbol_id, alice_a0->Id());

        auto alice_b_span = cgForm.get({1});
        ASSERT_EQ(alice_b_span.size(), 1);
        EXPECT_EQ(alice_b_span[0].symbol_id, alice_b0->Id());

        auto bob_a_span = cgForm.get({2});
        ASSERT_EQ(bob_a_span.size(), 1);
        EXPECT_EQ(bob_a_span[0].symbol_id, bob_a0->Id());

        auto bob_b_span = cgForm.get({3});
        ASSERT_EQ(bob_b_span.size(), 1);
        EXPECT_EQ(bob_b_span[0].symbol_id, bob_b0->Id());

        auto aa_span = cgForm.get({0, 2});
        ASSERT_EQ(aa_span.size(), 1);
        EXPECT_EQ(aa_span[0].symbol_id, alice_a0_bob_a0->Id());

        auto ab_span = cgForm.get({0, 3});
        ASSERT_EQ(ab_span.size(), 1);
        EXPECT_EQ(ab_span[0].symbol_id, alice_a0_bob_b0->Id());

        auto ba_span = cgForm.get({1, 2});
        ASSERT_EQ(ba_span.size(), 1);
        EXPECT_EQ(ba_span[0].symbol_id, alice_b0_bob_a0->Id());

        auto bb_span = cgForm.get({1, 3});
        ASSERT_EQ(bb_span.size(), 1);
        EXPECT_EQ(bb_span[0].symbol_id, alice_b0_bob_b0->Id());
    }

    TEST(ExplicitSymbols, GetWithFixed222) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 2, 2))};
        const auto& context = system.localityContext;

        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];

        auto [id, momentMatrix] = system.create_moment_matrix(1);

        auto alice_a0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 0)}, context));
        ASSERT_NE(alice_a0, nullptr);
        auto alice_b0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 0)}, context));
        ASSERT_NE(alice_b0, nullptr);
        auto bob_a0 = momentMatrix.Symbols.where(
                OperatorSequence({bob.measurement_outcome(0, 0)}, context));
        ASSERT_NE(bob_a0, nullptr);
        auto bob_b0 = momentMatrix.Symbols.where(
                OperatorSequence({bob.measurement_outcome(1, 0)}, context));
        ASSERT_NE(bob_b0, nullptr);

        auto alice_a0_bob_a0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 0), bob.measurement_outcome(0, 0)}, context));
        ASSERT_NE(alice_a0_bob_a0, nullptr);
        auto alice_a0_bob_b0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 0), bob.measurement_outcome(1, 0)}, context));
        ASSERT_NE(alice_a0_bob_b0, nullptr);
        auto alice_b0_bob_a0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 0), bob.measurement_outcome(0, 0)}, context));
        ASSERT_NE(alice_b0_bob_a0, nullptr);
        auto alice_b0_bob_b0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 0), bob.measurement_outcome(1, 0)}, context));
        ASSERT_NE(alice_b0_bob_b0, nullptr);

        const ExplicitSymbolIndex& cgForm = system.ExplicitSymbolTable();
        ASSERT_EQ(cgForm.Level, 2);

        auto fixA0freeB0 = cgForm.get({0, 2}, {0, -1});
        ASSERT_EQ(fixA0freeB0.size(), 1);
        EXPECT_EQ(fixA0freeB0[0].symbol_id, alice_a0_bob_a0->Id());

        auto fixA0freeB1 = cgForm.get({0, 3}, {0, -1});
        ASSERT_EQ(fixA0freeB1.size(), 1);
        EXPECT_EQ(fixA0freeB1[0].symbol_id, alice_a0_bob_b0->Id());

        auto fixB0freeB0 = cgForm.get({1, 2}, {0, -1});
        ASSERT_EQ(fixB0freeB0.size(), 1);
        EXPECT_EQ(fixB0freeB0[0].symbol_id, alice_b0_bob_a0->Id());

        auto fixB0freeB1 = cgForm.get({1, 3}, {0, -1});
        ASSERT_EQ(fixB0freeB1.size(), 1);
        EXPECT_EQ(fixB0freeB1[0].symbol_id, alice_b0_bob_b0->Id());

        auto freeA0fixB0 = cgForm.get({0, 2}, {-1, 0});
        ASSERT_EQ(freeA0fixB0.size(), 1);
        EXPECT_EQ(freeA0fixB0[0].symbol_id, alice_a0_bob_a0->Id());

        auto freeA0fixB1 = cgForm.get({0, 3}, {-1, 0});
        ASSERT_EQ(freeA0fixB1.size(), 1);
        EXPECT_EQ(freeA0fixB1[0].symbol_id, alice_a0_bob_b0->Id());

        auto freeB0fixB0 = cgForm.get({1, 2}, {-1, 0});
        ASSERT_EQ(freeB0fixB0.size(), 1);
        EXPECT_EQ(freeB0fixB0[0].symbol_id, alice_b0_bob_a0->Id());

        auto freeB0fixB1 = cgForm.get({1, 3}, {-1, 0});
        ASSERT_EQ(freeB0fixB1.size(), 1);
        EXPECT_EQ(freeB0fixB1[0].symbol_id, alice_b0_bob_b0->Id());

        auto fixA0fixB0 = cgForm.get({0, 2}, {0, 0});
        ASSERT_EQ(fixA0fixB0.size(), 1);
        EXPECT_EQ(fixA0fixB0[0].symbol_id, alice_a0_bob_a0->Id());

        auto fixA0fixB1 = cgForm.get({0, 3}, {0, 0});
        ASSERT_EQ(fixA0fixB1.size(), 1);
        EXPECT_EQ(fixA0fixB1[0].symbol_id, alice_a0_bob_b0->Id());

        auto fixA1fixB0 = cgForm.get({1, 2}, {0, 0});
        ASSERT_EQ(fixA1fixB0.size(), 1);
        EXPECT_EQ(fixA1fixB0[0].symbol_id, alice_b0_bob_a0->Id());

        auto fixA1fixB1 = cgForm.get({1, 3}, {0, 0});
        ASSERT_EQ(fixA1fixB1.size(), 1);
        EXPECT_EQ(fixA1fixB1[0].symbol_id, alice_b0_bob_b0->Id());

        auto freeA0freeB0 = cgForm.get({0, 2}, {-1, -1});
        ASSERT_EQ(freeA0freeB0.size(), 1);
        EXPECT_EQ(freeA0freeB0[0].symbol_id, alice_a0_bob_a0->Id());

        auto freeA0freeB1 = cgForm.get({0, 3}, {-1, -1});
        ASSERT_EQ(freeA0freeB1.size(), 1);
        EXPECT_EQ(freeA0freeB1[0].symbol_id, alice_a0_bob_b0->Id());

        auto freeA1freeB0 = cgForm.get({1, 2}, {-1, -1});
        ASSERT_EQ(freeA1freeB0.size(), 1);
        EXPECT_EQ(freeA1freeB0[0].symbol_id, alice_b0_bob_a0->Id());

        auto freeA1freeB1 = cgForm.get({1, 3}, {-1, -1});
        ASSERT_EQ(freeA1freeB1.size(), 1);
        EXPECT_EQ(freeA1freeB1[0].symbol_id, alice_b0_bob_b0->Id());
    }

    TEST(ExplicitSymbols, GetWithFixed223) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 2, 3))};
        const auto& context = system.localityContext;
        ASSERT_EQ(context.Parties.size(), 2);
        const auto &alice = context.Parties[0];
        const auto &bob = context.Parties[1];

        auto [id, momentMatrix] = system.create_moment_matrix(1);

        auto alice_a0_bob_a0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 0), bob.measurement_outcome(0, 0)}, context));
        auto alice_a0_bob_a1 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 0), bob.measurement_outcome(0, 1)}, context));
        auto alice_a0_bob_b0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 0), bob.measurement_outcome(1, 0)}, context));
        auto alice_a0_bob_b1 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 0), bob.measurement_outcome(1, 1)}, context));
        ASSERT_NE(alice_a0_bob_a0, nullptr);
        ASSERT_NE(alice_a0_bob_a1, nullptr);
        ASSERT_NE(alice_a0_bob_b0, nullptr);
        ASSERT_NE(alice_a0_bob_b1, nullptr);

        auto alice_a1_bob_a0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 1), bob.measurement_outcome(0, 0)}, context));
        auto alice_a1_bob_a1 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 1), bob.measurement_outcome(0, 1)}, context));
        auto alice_a1_bob_b0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 1), bob.measurement_outcome(1, 0)}, context));
        auto alice_a1_bob_b1 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 1), bob.measurement_outcome(1, 1)}, context));
        ASSERT_NE(alice_a1_bob_a0, nullptr);
        ASSERT_NE(alice_a1_bob_a1, nullptr);
        ASSERT_NE(alice_a1_bob_b0, nullptr);
        ASSERT_NE(alice_a1_bob_b1, nullptr);

        auto alice_b0_bob_a0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 0), bob.measurement_outcome(0, 0)}, context));
        auto alice_b0_bob_a1 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 0), bob.measurement_outcome(0, 1)}, context));
        auto alice_b0_bob_b0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 0), bob.measurement_outcome(1, 0)}, context));
        auto alice_b0_bob_b1 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 0), bob.measurement_outcome(1, 1)}, context));
        ASSERT_NE(alice_b0_bob_a0, nullptr);
        ASSERT_NE(alice_b0_bob_a1, nullptr);
        ASSERT_NE(alice_b0_bob_b0, nullptr);
        ASSERT_NE(alice_b0_bob_b1, nullptr);

        auto alice_b1_bob_a0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 1), bob.measurement_outcome(0, 0)}, context));
        auto alice_b1_bob_a1 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 1), bob.measurement_outcome(0, 1)}, context));
        auto alice_b1_bob_b0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 1), bob.measurement_outcome(1, 0)}, context));
        auto alice_b1_bob_b1 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 1), bob.measurement_outcome(1, 1)}, context));
        ASSERT_NE(alice_b1_bob_a0, nullptr);
        ASSERT_NE(alice_b1_bob_a1, nullptr);
        ASSERT_NE(alice_b1_bob_b0, nullptr);
        ASSERT_NE(alice_b1_bob_b1, nullptr);

        const ExplicitSymbolIndex& cgForm = system.ExplicitSymbolTable();
        ASSERT_EQ(cgForm.Level, 2);

        auto fixA00freeB0 = cgForm.get({0, 2}, {0, -1});
        ASSERT_EQ(fixA00freeB0.size(), 2);
        EXPECT_EQ(fixA00freeB0[0].symbol_id, alice_a0_bob_a0->Id());
        EXPECT_EQ(fixA00freeB0[1].symbol_id, alice_a0_bob_a1->Id());

        auto fixA00freeB1 = cgForm.get({0, 3}, {0, -1});
        ASSERT_EQ(fixA00freeB1.size(), 2);
        EXPECT_EQ(fixA00freeB1[0].symbol_id, alice_a0_bob_b0->Id());
        EXPECT_EQ(fixA00freeB1[1].symbol_id, alice_a0_bob_b1->Id());

        auto fixA01freeB0 = cgForm.get({0, 2}, {1, -1});
        ASSERT_EQ(fixA01freeB0.size(), 2);
        EXPECT_EQ(fixA01freeB0[0].symbol_id, alice_a1_bob_a0->Id());
        EXPECT_EQ(fixA01freeB0[1].symbol_id, alice_a1_bob_a1->Id());

        auto fixA01freeB1 = cgForm.get({0, 3}, {1, -1});
        ASSERT_EQ(fixA01freeB1.size(), 2);
        EXPECT_EQ(fixA01freeB1[0].symbol_id, alice_a1_bob_b0->Id());
        EXPECT_EQ(fixA01freeB1[1].symbol_id, alice_a1_bob_b1->Id());

        auto fixA10freeB0 = cgForm.get({1, 2}, {0, -1});
        ASSERT_EQ(fixA10freeB0.size(), 2);
        EXPECT_EQ(fixA10freeB0[0].symbol_id, alice_b0_bob_a0->Id());
        EXPECT_EQ(fixA10freeB0[1].symbol_id, alice_b0_bob_a1->Id());

        auto fixA10freeB1 = cgForm.get({1, 3}, {0, -1});
        ASSERT_EQ(fixA10freeB1.size(), 2);
        EXPECT_EQ(fixA10freeB1[0].symbol_id, alice_b0_bob_b0->Id());
        EXPECT_EQ(fixA10freeB1[1].symbol_id, alice_b0_bob_b1->Id());

        auto fixA11freeB0 = cgForm.get({1, 2}, {1, -1});
        ASSERT_EQ(fixA11freeB0.size(), 2);
        EXPECT_EQ(fixA11freeB0[0].symbol_id, alice_b1_bob_a0->Id());
        EXPECT_EQ(fixA11freeB0[1].symbol_id, alice_b1_bob_a1->Id());

        auto fixA11freeB1 = cgForm.get({1, 3}, {1, -1});
        ASSERT_EQ(fixA11freeB1.size(), 2);
        EXPECT_EQ(fixA11freeB1[0].symbol_id, alice_b1_bob_b0->Id());
        EXPECT_EQ(fixA11freeB1[1].symbol_id, alice_b1_bob_b1->Id());

        auto freeA0fixB00 = cgForm.get({0, 2}, {-1, 0});
        ASSERT_EQ(freeA0fixB00.size(), 2);
        EXPECT_EQ(freeA0fixB00[0].symbol_id, alice_a0_bob_a0->Id());
        EXPECT_EQ(freeA0fixB00[1].symbol_id, alice_a1_bob_a0->Id());

        auto freeA0fixB01 = cgForm.get({0, 2}, {-1, 1});
        ASSERT_EQ(freeA0fixB01.size(), 2);
        EXPECT_EQ(freeA0fixB01[0].symbol_id, alice_a0_bob_a1->Id());
        EXPECT_EQ(freeA0fixB01[1].symbol_id, alice_a1_bob_a1->Id());

        auto freeA0fixB10 = cgForm.get({0, 3}, {-1, 0});
        ASSERT_EQ(freeA0fixB10.size(), 2);
        EXPECT_EQ(freeA0fixB10[0].symbol_id, alice_a0_bob_b0->Id());
        EXPECT_EQ(freeA0fixB10[1].symbol_id, alice_a1_bob_b0->Id());

        auto freeA0fixB11 = cgForm.get({0, 3}, {-1, 1});
        ASSERT_EQ(freeA0fixB11.size(), 2);
        EXPECT_EQ(freeA0fixB11[0].symbol_id, alice_a0_bob_b1->Id());
        EXPECT_EQ(freeA0fixB11[1].symbol_id, alice_a1_bob_b1->Id());

        auto freeA1fixB00 = cgForm.get({1, 2}, {-1, 0});
        ASSERT_EQ(freeA1fixB00.size(), 2);
        EXPECT_EQ(freeA1fixB00[0].symbol_id, alice_b0_bob_a0->Id());
        EXPECT_EQ(freeA1fixB00[1].symbol_id, alice_b1_bob_a0->Id());

        auto freeA1fixB01 = cgForm.get({1, 2}, {-1, 1});
        ASSERT_EQ(freeA1fixB01.size(), 2);
        EXPECT_EQ(freeA1fixB01[0].symbol_id, alice_b0_bob_a1->Id());
        EXPECT_EQ(freeA1fixB01[1].symbol_id, alice_b1_bob_a1->Id());

        auto freeA1fixB10 = cgForm.get({1, 3}, {-1, 0});
        ASSERT_EQ(freeA1fixB10.size(), 2);
        EXPECT_EQ(freeA1fixB10[0].symbol_id, alice_b0_bob_b0->Id());
        EXPECT_EQ(freeA1fixB10[1].symbol_id, alice_b1_bob_b0->Id());

        auto freeA1fixB11 = cgForm.get({1, 3}, {-1, 1});
        ASSERT_EQ(freeA1fixB11.size(), 2);
        EXPECT_EQ(freeA1fixB11[0].symbol_id, alice_b0_bob_b1->Id());
        EXPECT_EQ(freeA1fixB11[1].symbol_id, alice_b1_bob_b1->Id());
    }

    TEST(ExplicitSymbols, VariedOutcomes_52_22_32) {
        std::vector<Party> partyList;
        partyList.emplace_back(0, "a", std::vector{Measurement{"a", 5}, Measurement{"b",2}});
        partyList.emplace_back(1, "b", std::vector{Measurement{"a", 2}, Measurement{"b",2}});
        partyList.emplace_back(2, "c", std::vector{Measurement{"a", 3}, Measurement{"b",2}});

        LocalityMatrixSystem system{std::make_unique<LocalityContext>(std::move(partyList))};
        const auto& context = system.localityContext;

        ASSERT_EQ(context.Parties.size(), 3);
        const auto &alice = context.Parties[0];
        const auto &bob = context.Parties[1];
        const auto &charlie = context.Parties[2];
        ASSERT_EQ(alice.Measurements.size(), 2);
        ASSERT_EQ(bob.Measurements.size(), 2);
        ASSERT_EQ(charlie.Measurements.size(), 2);


        auto [id, momentMatrix] = system.create_moment_matrix(2);
        const auto& explSymb = system.ExplicitSymbolTable();

        auto aaa = explSymb.get({0, 2, 4});
        EXPECT_EQ(aaa.size(), 4*1*2); // 5, 2, 3

        auto a0axax = explSymb.get({0, 2, 4}, {3, -1, -1});
        ASSERT_EQ(a0axax.size(), 2); // [a0a0a0, a0a0a1]

    }

    TEST(ExplicitSymbols, InflationVersion) {

        auto icPtr = std::make_unique<InflationContext>(CausalNetwork{{2, 2, 3}, {{0, 1}, {1, 2}}}, 2);
        InflationMatrixSystem ims{std::move(icPtr)};
        auto [id, momentMatrix] = ims.create_moment_matrix(2);
        const auto& context = ims.InflationContext();
        const auto& symbols = ims.Symbols();

        // Operator IDs
        ASSERT_EQ(context.Observables().size(), 3);
        const auto a0 = context.Observables()[0].operator_offset;
        const auto b0 = context.Observables()[1].operator_offset;
        const auto c0 = context.Observables()[2].operator_offset;
        const auto c1 = context.Observables()[2].operator_offset + 1;

        const auto* a0Ptr = symbols.where(OperatorSequence{{a0}, context});
        const auto* b0Ptr = symbols.where(OperatorSequence{{b0}, context});
        const auto* c0Ptr = symbols.where(OperatorSequence{{c0}, context});
        const auto* c1Ptr = symbols.where(OperatorSequence{{c1}, context});

        const auto* a0b0Ptr = symbols.where(OperatorSequence{{a0, b0}, context});
        const auto* a0c0Ptr = symbols.where(OperatorSequence{{a0, c0}, context});
        const auto* a0c1Ptr = symbols.where(OperatorSequence{{a0, c1}, context});
        const auto* b0c0Ptr = symbols.where(OperatorSequence{{b0, c0}, context});
        const auto* b0c1Ptr = symbols.where(OperatorSequence{{b0, c1}, context});

        const auto* a0b0c0Ptr = symbols.where(OperatorSequence{{a0, b0, c0}, context});
        const auto* a0b0c1Ptr = symbols.where(OperatorSequence{{a0, b0, c1}, context});

        ASSERT_NE(a0Ptr, nullptr);
        ASSERT_NE(b0Ptr, nullptr);
        ASSERT_NE(c0Ptr, nullptr);
        ASSERT_NE(c1Ptr, nullptr);

        ASSERT_NE(a0b0Ptr, nullptr);
        ASSERT_NE(a0c0Ptr, nullptr);
        ASSERT_NE(a0c1Ptr, nullptr);
        ASSERT_NE(b0c0Ptr, nullptr);
        ASSERT_NE(b0c1Ptr, nullptr);

        ASSERT_NE(a0b0c0Ptr, nullptr);
        ASSERT_NE(a0b0c1Ptr, nullptr);


        const auto& explicit_symbols = ims.ExplicitSymbolTable();

        // A
        auto s_a = explicit_symbols.get({0});
        ASSERT_EQ(s_a.size(), 1);
        EXPECT_EQ(s_a[0].symbol_id, a0Ptr->Id());

        // B
        auto s_b = explicit_symbols.get({1});
        ASSERT_EQ(s_b.size(), 1);
        EXPECT_EQ(s_b[0].symbol_id, b0Ptr->Id());

        // C
        auto s_c = explicit_symbols.get({2});
        ASSERT_EQ(s_c.size(), 2);
        EXPECT_EQ(s_c[0].symbol_id, c0Ptr->Id());
        EXPECT_EQ(s_c[1].symbol_id, c1Ptr->Id());

        // AB
        auto s_ab = explicit_symbols.get({0, 1});
        ASSERT_EQ(s_ab.size(), 1);
        EXPECT_EQ(s_ab[0].symbol_id, a0b0Ptr->Id());

        // AC
        auto s_ac = explicit_symbols.get({0, 2});
        ASSERT_EQ(s_ac.size(), 2);
        EXPECT_EQ(s_ac[0].symbol_id, a0c0Ptr->Id());
        EXPECT_EQ(s_ac[1].symbol_id, a0c1Ptr->Id());

        // BC
        auto s_bc = explicit_symbols.get({1, 2});
        ASSERT_EQ(s_bc.size(), 2);
        EXPECT_EQ(s_bc[0].symbol_id, b0c0Ptr->Id());
        EXPECT_EQ(s_bc[1].symbol_id, b0c1Ptr->Id());

        // ABC
        auto s_abc = explicit_symbols.get({0, 1, 2});
        ASSERT_EQ(s_abc.size(), 2);
        EXPECT_EQ(s_abc[0].symbol_id, a0b0c0Ptr->Id());
        EXPECT_EQ(s_abc[1].symbol_id, a0b0c1Ptr->Id());
    }
}
