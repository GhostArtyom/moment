/**
 * moment_substitution_rule_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/context.h"
#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"

#include "symbolic/symbol_table.h"
#include "symbolic/moment_substitution_rule.h"
#include "symbolic/order_symbols_by_hash.h"


namespace Moment::Tests {
    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_Trivial) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        ByIDPolynomialFactory factory{table};

        auto zero = Polynomial::Zero();
        EXPECT_EQ(MomentSubstitutionRule::get_difficulty(zero), MomentSubstitutionRule::PolynomialDifficulty::Trivial);
        MomentSubstitutionRule msr{factory, Polynomial{zero}};

        EXPECT_EQ(msr.LHS(), 0);
        EXPECT_EQ(msr.RHS(), Polynomial());
        EXPECT_TRUE(msr.is_trivial());
    }

    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_ThreeToZero) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        ByIDPolynomialFactory factory{table};

        Polynomial combo{Monomial{3, 1.0}}; // #2 + 0.5 = 0
        EXPECT_EQ(MomentSubstitutionRule::get_difficulty(combo), MomentSubstitutionRule::PolynomialDifficulty::Simple);
        MomentSubstitutionRule msr{factory, std::move(combo)};

        EXPECT_EQ(msr.LHS(), 3);
        EXPECT_EQ(msr.RHS(), Polynomial());
        EXPECT_FALSE(msr.is_trivial());
    }

    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_TwoToScalar) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        ByIDPolynomialFactory factory{table};

        Polynomial combo = factory({Monomial{2, 1.0}, Monomial{1, -0.5}}); // #2 + 0.5 = 0
        EXPECT_EQ(MomentSubstitutionRule::get_difficulty(combo), MomentSubstitutionRule::PolynomialDifficulty::Simple);
        MomentSubstitutionRule msr{factory, std::move(combo)};

        EXPECT_EQ(msr.LHS(), 2);
        EXPECT_EQ(msr.RHS(), Polynomial::Scalar(0.5));
        EXPECT_FALSE(msr.is_trivial());
    }

    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_ThreeToTwoPlusOne) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        ByIDPolynomialFactory factory{table};

        Polynomial combo = factory({Monomial{3, -1.0}, Monomial{2, 1.0},
                                    Monomial{1, 1.0}}); // -#3 + #2 + 1 = 0
        EXPECT_EQ(MomentSubstitutionRule::get_difficulty(combo), MomentSubstitutionRule::PolynomialDifficulty::Simple);
        MomentSubstitutionRule msr{factory, std::move(combo)};

        EXPECT_EQ(msr.LHS(), 3);
        EXPECT_EQ(msr.RHS(), Polynomial({Monomial{2, 1.0}, Monomial{1, 1.0}}));
        EXPECT_FALSE(msr.is_trivial());
    }

    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_HalfThreeStarToTwo) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        ByIDPolynomialFactory factory{table};

        Polynomial combo{Monomial{3, 0.5, true}, Monomial{2, 1.0}}; // 0.5#3* + #2 = 0
        EXPECT_EQ(MomentSubstitutionRule::get_difficulty(combo), MomentSubstitutionRule::PolynomialDifficulty::Simple);
        MomentSubstitutionRule msr{factory, std::move(combo)};

        EXPECT_EQ(msr.LHS(), 3);
        EXPECT_EQ(msr.RHS(), Polynomial(Monomial{2, -2.0, true}));
        EXPECT_FALSE(msr.is_trivial());
    }

    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_HorriblyComplex) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        ByIDPolynomialFactory factory{table};

        // (0.5 + i) #3* + (1-3i) #2 = 0
        Polynomial combo{Monomial{3, std::complex{0.5, 1.0}, true}, Monomial{2, std::complex{1.0, -3.0}}};
        EXPECT_EQ(MomentSubstitutionRule::get_difficulty(combo), MomentSubstitutionRule::PolynomialDifficulty::Simple);
        MomentSubstitutionRule msr{factory, std::move(combo)};

        std::complex<double> expected_prefactor = std::conj(-std::complex{1.0, -3.0} / std::complex{0.5, 1.0}); // 2-2i
        EXPECT_EQ(msr.LHS(), 3);
        ASSERT_EQ(msr.RHS().size(), 1);
        EXPECT_TRUE(approximately_equal(msr.RHS()[0].factor, expected_prefactor, 100));
        EXPECT_FALSE(msr.is_trivial());
    }

    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_ErrorBadScalar) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        ByIDPolynomialFactory factory{table};

        Polynomial combo{Monomial{1, 2.5}}; // #2 + 0.5 = 0
        EXPECT_EQ(MomentSubstitutionRule::get_difficulty(combo),
                  MomentSubstitutionRule::PolynomialDifficulty::Contradiction);
        EXPECT_THROW([[maybe_unused]] auto msr = MomentSubstitutionRule(factory, std::move(combo)),
                     errors::invalid_moment_rule);
    }

    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_HardToOrient) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(2, true, true); // #2 and #3, complex.
        ByIDPolynomialFactory factory{table, 10};


        for (int index = 1; index < 10; ++index) { // try values 0.1 ... 0.9
            std::complex<double> factor_k = {static_cast<double>(index)*0.1, 0};

            // Analytic solution: <Z> -> 1/(k'k-1) <Y> - k/(k'k-1) <Y'>
            const Polynomial trickyPoly = factory(
                    {Monomial{3, 1.0, false}, Monomial{3, factor_k, true}, Monomial{2, 1.0}});
            EXPECT_EQ(MomentSubstitutionRule::get_difficulty(trickyPoly),
                      MomentSubstitutionRule::PolynomialDifficulty::NeedsReorienting) << " k = " << factor_k;

            MomentSubstitutionRule trickyRule(factory, Polynomial{trickyPoly});
            const std::complex<double> expected_y_coef =
                    std::complex<double>{1.0, 0.0} / (factor_k * std::conj(factor_k) - std::complex{1.0,0.0});
            const std::complex<double> expected_ystar_coef = -factor_k * expected_y_coef;
            const Polynomial expectedRHS = factory({Monomial{2, expected_y_coef, false},
                                                    Monomial{2, expected_ystar_coef, true}});

            EXPECT_EQ(trickyRule.LHS(), 3) << " k = " << factor_k;
            EXPECT_TRUE(trickyRule.RHS().approximately_equals(expectedRHS, factory.zero_tolerance))
                << " LHS = " << trickyRule.RHS() << " RHS = " << expectedRHS
                << ", k = " << factor_k;
        }
    }


    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_ImpossibleToOrient) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(2, true, true); // #2 and #3, complex.
        ByIDPolynomialFactory factory{table, 10};

        for (int index = 0; index < 12; ++index) {
            std::complex<double> factor = std::polar(1.0, 3.141592653589793238 * static_cast<double>(index)/12.0);

            const Polynomial impossiblePoly = factory(
                    {Monomial{3, 1.0, false}, Monomial{3, factor, true}, Monomial{2, 1.0}});
            EXPECT_EQ(MomentSubstitutionRule::get_difficulty(impossiblePoly),
                      MomentSubstitutionRule::PolynomialDifficulty::NonorientableRule)
                      << "theta = " << index << "*PI/6)";
            EXPECT_THROW(MomentSubstitutionRule impossible_rule(factory, Polynomial{impossiblePoly}),
                         Moment::errors::nonorientable_rule);
        }
    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_TwoToZero) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        ByIDPolynomialFactory factory{table};

        MomentSubstitutionRule msr{2, Polynomial()}; // #2 -> 0.
        ASSERT_EQ(msr.LHS(), 2);
        ASSERT_EQ(msr.RHS(), Polynomial());

        const Polynomial input_two{{Monomial(2, 1.0)}};
        EXPECT_TRUE(msr.matches(input_two));
        EXPECT_EQ(msr.reduce(factory, input_two), Polynomial());

        const Polynomial input_two_plus_scalar{{Monomial{2, 1.0}, Monomial{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_two_plus_scalar));
        EXPECT_EQ(msr.reduce(factory, input_two_plus_scalar), Polynomial::Scalar(3.0));

        const Polynomial input_three_plus_two{{Monomial{3, 1.0}, Monomial{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_two), Polynomial({Monomial{3, 1.0}}));

        const Polynomial input_two_plus_two_star{{Monomial(2, 1.0), Monomial(2, 1.0, true)}};
        EXPECT_EQ(msr.reduce(factory, input_two_plus_two_star), Polynomial());

        const Polynomial noMatch{{Monomial{3, 1.0}, Monomial{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(factory, noMatch), noMatch);
    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_TwoToScalar) {
        // Fake context/table with 4 symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        ByIDPolynomialFactory factory{table};

        MomentSubstitutionRule msr{2, Polynomial::Scalar(0.5)}; // #2 -> 0.5#1.
        ASSERT_EQ(msr.LHS(), 2);
        ASSERT_EQ(msr.RHS(), Polynomial::Scalar(0.5));

        const Polynomial input_two{{Monomial(2, 2.0)}};
        EXPECT_TRUE(msr.matches(input_two));
        EXPECT_EQ(msr.reduce(factory, input_two), Polynomial::Scalar(1.0));

        const Polynomial input_two_conj{{Monomial(2, 2.0, true)}};
        EXPECT_TRUE(msr.matches(input_two_conj));
        EXPECT_EQ(msr.reduce(factory, input_two_conj), Polynomial::Scalar(1.0));

        const Polynomial input_two_plus_scalar{{Monomial{2, 1.0}, Monomial{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_two_plus_scalar));
        EXPECT_EQ(msr.reduce(factory, input_two_plus_scalar), Polynomial::Scalar(3.5));

        const Polynomial input_three_plus_two{{Monomial{3, 1.0}, Monomial{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_two),
                  Polynomial({Monomial{3, 1.0}, Monomial{1, 1.5}}));

        const Polynomial input_two_minus_half{{Monomial{2, 1.0}, Monomial{1, -0.5}}};
        EXPECT_TRUE(msr.matches(input_two_minus_half));
        EXPECT_EQ(msr.reduce(factory, input_two_minus_half), Polynomial());

        const Polynomial input_two_plus_two_star{{Monomial{2, 1.0}, Monomial{2, 1.0, true}}};
        EXPECT_EQ(msr.reduce(factory, input_two_plus_two_star), Polynomial::Scalar(1.0));

        const Polynomial noMatch{{Monomial{3, 1.0}, Monomial{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(factory, noMatch), noMatch);
    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_ThreeToTwo) {
        // Fake context/table with 4 symbols
        Context contxt{2};
        SymbolTable table{contxt};
        table.create(4, true, true);
        ByIDPolynomialFactory factory{table};

        MomentSubstitutionRule msr{3, Polynomial(Monomial{2, 1.0})}; // #3 -> #2
        ASSERT_EQ(msr.LHS(), 3);
        ASSERT_EQ(msr.RHS(), Polynomial(Monomial{2, 1.0}));

        const Polynomial input_three{{Monomial(3, 2.0)}};
        EXPECT_TRUE(msr.matches(input_three));
        EXPECT_EQ(msr.reduce(factory, input_three), Polynomial(Monomial{2, 2.0}));

        const Polynomial input_three_conj{{Monomial(3, 2.0, true)}};
        EXPECT_TRUE(msr.matches(input_three_conj));
        EXPECT_EQ(msr.reduce(factory, input_three_conj), Polynomial(Monomial{2, 2.0, true}));

        const Polynomial input_three_plus_scalar{{Monomial{3, 1.0}, Monomial{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_scalar));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_scalar),
                  Polynomial({Monomial{2, 1.0}, Monomial{1, 3.0}}));

        const Polynomial input_three_plus_two{{Monomial{3, 1.0}, Monomial{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_two), Polynomial({Monomial{2, 4.0}}));

        const Polynomial input_three_minus_two{{Monomial{3, 1.0}, Monomial{2, -1.0}}};
        EXPECT_TRUE(msr.matches(input_three_minus_two));
        EXPECT_EQ(msr.reduce(factory, input_three_minus_two), Polynomial());

        const Polynomial input_three_plus_three_star{{Monomial{3, 1.0}, Monomial{3, 1.0, true}}};
        EXPECT_EQ(msr.reduce(factory, input_three_plus_three_star), Polynomial({Monomial{2, 1.0},
                                                                                Monomial{2, 1.0, true}}));

        const Polynomial noMatch{{Monomial{2, 1.0}, Monomial{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(factory, noMatch), noMatch);


    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_ThreeToHalfTwoStar) {
        // Fake context/table with 4 symbols
        Context contxt{2};
        SymbolTable table{contxt};
        table.create(4, true, true);
        ByIDPolynomialFactory factory{table};

        MomentSubstitutionRule msr{3, Polynomial(Monomial{2, 0.5, true})}; // #3 -> 0.5#2*.
        ASSERT_EQ(msr.LHS(), 3);
        ASSERT_EQ(msr.RHS(), Polynomial(Monomial{2, 0.5, true}));

        const Polynomial input_three{{Monomial(3, 2.0)}};
        EXPECT_TRUE(msr.matches(input_three));
        EXPECT_EQ(msr.reduce(factory, input_three), Polynomial(Monomial{2, 1.0, true}));

        const Polynomial input_three_conj{{Monomial(3, 2.0, true)}};
        EXPECT_TRUE(msr.matches(input_three_conj));
        EXPECT_EQ(msr.reduce(factory, input_three_conj), Polynomial(Monomial{2, 1.0, false}));

        const Polynomial input_three_plus_scalar{{Monomial{3, 1.0}, Monomial{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_scalar));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_scalar),
                  Polynomial({Monomial{2, 0.5, true}, Monomial{1, 3.0}}));

        const Polynomial input_three_plus_two{{Monomial{3, 1.0}, Monomial{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_two),
                  Polynomial({Monomial{2, 3.0}, Monomial{2, 0.5, true}}));

        const Polynomial input_three_minus_half_two_star{{Monomial{3, 1.0}, Monomial{2, -0.5, true}}};
        EXPECT_TRUE(msr.matches(input_three_minus_half_two_star));
        EXPECT_EQ(msr.reduce(factory, input_three_minus_half_two_star), Polynomial());

        const Polynomial noMatch{{Monomial{2, 1.0}, Monomial{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(factory, noMatch), noMatch);
    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_ThreeToTwoPlusOne) {
        // Fake context/table with 4 symbols
        Context contxt{2};
        SymbolTable table{contxt};
        table.create(4, true, true);
        ByIDPolynomialFactory factory{table};


        MomentSubstitutionRule msr{3,
                                   Polynomial({Monomial{2, 1.0}, Monomial{1, 1.0}})}; // #3 -> #2 + 1
        ASSERT_EQ(msr.LHS(), 3);
        ASSERT_EQ(msr.RHS(), Polynomial({Monomial{2, 1.0}, Monomial{1, 1.0}}));

        const Polynomial input_three{{Monomial(3, 2.0)}};
        EXPECT_TRUE(msr.matches(input_three));
        EXPECT_EQ(msr.reduce(factory, input_three), Polynomial({Monomial{2, 2.0}, Monomial{1, 2.0}}));

        const Polynomial input_three_conj{{Monomial(3, 2.0, true)}};
        EXPECT_TRUE(msr.matches(input_three_conj));
        EXPECT_EQ(msr.reduce(factory, input_three_conj),
                  Polynomial({Monomial{2, 2.0, true}, Monomial{1, 2.0}}));

        const Polynomial input_three_plus_scalar{{Monomial{3, 1.0}, Monomial{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_scalar));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_scalar),
                  Polynomial({Monomial{2, 1.0}, Monomial{1, 4.0}}));

        const Polynomial input_three_plus_two{{Monomial{3, 1.0}, Monomial{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_two),
                  Polynomial({Monomial{2, 4.0}, Monomial{1, 1.0}}));

        const Polynomial input_three_minus_two_minus_one{{Monomial{3, 1.0},
                                                          Monomial{2, -1.0},
                                                          Monomial{1, -1.0}}};
        EXPECT_TRUE(msr.matches(input_three_minus_two_minus_one));
        EXPECT_EQ(msr.reduce(factory, input_three_minus_two_minus_one), Polynomial());

        const Polynomial noMatch{{Monomial{2, 1.0}, Monomial{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(factory, noMatch), noMatch);
    }


    TEST(Symbolic_MomentSubstitutionRule, Reduce_RealToImaginary) {
        // Fake context/table with 3 symbols
        Context context{2};
        SymbolTable table{context};
        table.create(1, true, false); // #2 is  Hermitian
        table.create(2, true, true); // #3, #4 are not Hermitian.
        ByIDPolynomialFactory factory{table};

        ASSERT_EQ(factory({Monomial{2, 1.0}}), factory({Monomial{2, 1.0, true}}));
        ASSERT_NE(factory({Monomial{3, 1.0}}), factory({Monomial{3, 1.0, true}}));

        MomentSubstitutionRule msr{3, factory({Monomial{2, std::complex{0.0, 1.0}}, Monomial{1, 1.0}})}; // #3 -> i #2 + 1

        Polynomial input_three_three_star = factory({Monomial{3, 1.0}, Monomial{3, 1.0, true}});
        EXPECT_EQ(msr.reduce(factory, input_three_three_star), Polynomial::Scalar(2.0));

        Polynomial input_three_minus_three_star = factory({Monomial{3, 1.0}, Monomial{3, -1.0, true}});
        EXPECT_EQ(msr.reduce(factory, input_three_minus_three_star),
                  factory({Monomial{2, std::complex{0.0, 2.0}, false}}));

    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_WithOpOrderFactory) {
        // Fake context/table with 4 non-trivial symbols
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto &table = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        ByHashPolynomialFactory factory{table, 1.0, table};

        MomentSubstitutionRule msr{5, factory({Monomial{2, 0.5}})}; // #5 -> 0.5#2 (<ab> -> <a>).
        ASSERT_EQ(msr.LHS(), 5);
        ASSERT_EQ(msr.RHS(), factory({Monomial{2, 0.5}}));

        const Polynomial input_five = factory({{Monomial(5, 2.0)}});
        EXPECT_TRUE(msr.matches(input_five));
        EXPECT_EQ(msr.reduce(factory, input_five), factory({Monomial{2, 1.0}}));

        const Polynomial input_five_conj = factory({{Monomial(5, 2.0, true)}});
        EXPECT_TRUE(msr.matches(input_five_conj));
        EXPECT_EQ(msr.reduce(factory, input_five_conj),
                  factory({Monomial{2, 1.0, false}}));

        const Polynomial input_five_plus_scalar =
                factory({Monomial{5, 1.0}, Monomial{1, 3.0}});
        EXPECT_TRUE(msr.matches(input_five_plus_scalar));
        EXPECT_EQ(msr.reduce(factory, input_five_plus_scalar),
                factory({Monomial{2, 0.5}, Monomial{1, 3.0}}));

        const Polynomial input_five_plus_two
            = factory({Monomial{5, 1.0}, Monomial{2, 3.0}});
        EXPECT_TRUE(msr.matches(input_five_plus_two));
        EXPECT_EQ(msr.reduce(factory, input_five_plus_two),
                  factory({Monomial{2, 3.5}}));

        const Polynomial input_five_minus_half_two
            = factory({{Monomial{5, 1.0}, Monomial{2, -0.5}}});
        EXPECT_TRUE(msr.matches(input_five_minus_half_two));
        EXPECT_EQ(msr.reduce(factory, input_five_minus_half_two), Polynomial());

        const Polynomial noMatch = factory({{Monomial{2, 1.0}, Monomial{4, -1.0}}});
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(factory, noMatch), noMatch);
    }

    TEST(Symbolic_MomentSubstitutionRule, AsPolynomial_Trivial) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        ByIDPolynomialFactory factory{table};

        MomentSubstitutionRule msr{factory, Polynomial::Zero()};

        EXPECT_TRUE(msr.is_trivial());
        EXPECT_EQ(msr.as_polynomial(factory), Polynomial::Zero());
    }

    TEST(Symbolic_MomentSubstitutionRule, AsPolynomial_ThreeToZero) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        ByIDPolynomialFactory factory{table};

        MomentSubstitutionRule msr{factory, factory({Monomial{3, 1.0}})};
        EXPECT_EQ(msr.as_polynomial(factory), factory({Monomial{3, -1.0}}));
    }

    TEST(Symbolic_MomentSubstitutionRule, AsPolynomial_TwoToScalar) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        ByIDPolynomialFactory factory{table};

        MomentSubstitutionRule msr{factory, factory({Monomial{2, 1.0}, Monomial{1, -0.5}})};

        EXPECT_EQ(msr.as_polynomial(factory), factory({Monomial{2, -1.0}, Monomial{1, 0.5}}));
    }

    TEST(Symbolic_MomentSubstitutionRule, AsPolynomial_ThreeToTwoPlusOne) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        ByIDPolynomialFactory factory{table};

        MomentSubstitutionRule msr{factory, factory({Monomial{3, -1.0}, Monomial{2, 1.0}, Monomial{1, 1.0}})};

        EXPECT_EQ(msr.as_polynomial(factory), factory({Monomial{3, -1.0}, Monomial{2, 1.0}, Monomial{1, 1.0}}));

    }

    TEST(Symbolic_MomentSubstitutionRule, AsPolynomial_HalfThreeStarToTwo) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        ByIDPolynomialFactory factory{table};

        MomentSubstitutionRule msr{factory, factory({Monomial{3, 0.5, true}, Monomial{2, 1.0}})};
        EXPECT_EQ(msr.as_polynomial(factory), factory({Monomial{3, -1.0}, Monomial{2, -2.0, true}}));
    }

}
