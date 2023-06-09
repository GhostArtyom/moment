/**
 * moment_substitution_rule_split_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/context.h"
#include "scenarios/imported/imported_matrix_system.h"

#include "symbolic/symbol_table.h"
#include "symbolic/moment_substitution_rule.h"
#include "symbolic/order_symbols_by_hash.h"



namespace Moment::Tests {

    class Symbolic_MomentSubstitutionRule_Split : public ::testing::Test {
    private:
        std::unique_ptr<Imported::ImportedMatrixSystem> ims_ptr;
        std::unique_ptr<PolynomialFactory> factory_ptr;

    protected:
        symbol_name_t id, re_a, comp_b, im_c, re_d, comp_e, im_f;

    protected:
        void SetUp() override {

            ims_ptr = std::make_unique<Imported::ImportedMatrixSystem>();
            auto &symbols = ims_ptr->Symbols();
            symbols.create(1, true, false); // a = 2 real
            symbols.create(1, true, true); // b = 3  complex
            symbols.create(1, false, true); // c = 4 imaginary
            symbols.create(1, true, false); // a = 5 real
            symbols.create(1, true, true); // b = 6  complex
            symbols.create(1, false, true); // c = 7 imaginary

            factory_ptr = std::make_unique<ByIDPolynomialFactory>(ims_ptr->Symbols());

            this->id = 1;
            this->re_a = 2;
            this->comp_b = 3;
            this->im_c = 4;
            this->re_d = 5;
            this->comp_e = 6;
            this->im_f = 7;

            // Real
            ASSERT_TRUE(symbols[this->re_a].is_hermitian());
            ASSERT_FALSE(symbols[this->re_a].is_antihermitian());
            ASSERT_TRUE(symbols[this->re_d].is_hermitian());
            ASSERT_FALSE(symbols[this->re_d].is_antihermitian());

            // Complex
            ASSERT_FALSE(symbols[this->comp_b].is_hermitian());
            ASSERT_FALSE(symbols[this->comp_b].is_antihermitian());
            ASSERT_FALSE(symbols[this->comp_e].is_hermitian());
            ASSERT_FALSE(symbols[this->comp_e].is_antihermitian());

            // Imaginary
            ASSERT_FALSE(symbols[this->im_c].is_hermitian());
            ASSERT_TRUE(symbols[this->im_c].is_antihermitian());
            ASSERT_FALSE(symbols[this->im_f].is_hermitian());
            ASSERT_TRUE(symbols[this->im_f].is_antihermitian());
        }

        [[nodiscard]] Imported::ImportedMatrixSystem& get_system() const noexcept {
            return *this->ims_ptr;
        }


        [[nodiscard]] SymbolTable& get_symbols() noexcept { return this->ims_ptr->Symbols(); };

        [[nodiscard]] const PolynomialFactory& get_factory() const noexcept { return *this->factory_ptr; };

        void expect_approximately_equal(const Polynomial& LHS, const Polynomial& RHS) {
            EXPECT_TRUE(LHS.approximately_equals(RHS, this->factory_ptr->zero_tolerance))
                << "LHS = \n" << LHS << "\n RHS = \n" << RHS;
        }

    };

    TEST_F(Symbolic_MomentSubstitutionRule_Split, NoSplit_Trivial) {
        const auto& factory = this->get_factory();
        MomentSubstitutionRule msr{factory, Polynomial::Zero()};
        auto split = msr.impose_hermicity_of_LHS(factory);
        EXPECT_FALSE(split.has_value());
        EXPECT_EQ(msr.LHS(), 0);
        expect_approximately_equal(msr.RHS(), Polynomial::Zero());
    }

    TEST_F(Symbolic_MomentSubstitutionRule_Split, NoSplit_SimpleEqualsZero) {
        const auto& factory = this->get_factory();
        MomentSubstitutionRule msr{factory, factory({Monomial{comp_b, 1.0}})};
        auto split = msr.impose_hermicity_of_LHS(factory);
        EXPECT_FALSE(split.has_value());
        EXPECT_EQ(msr.LHS(), comp_b);
        expect_approximately_equal(msr.RHS(), Polynomial::Zero());
    }

    TEST_F(Symbolic_MomentSubstitutionRule_Split, NoSplit_SimpleEqualsNonzero) {
        const auto& factory = this->get_factory();
        MomentSubstitutionRule msr{factory, factory({Monomial{comp_b, 1.0}, Monomial{re_a, -1.0}})};
        auto split = msr.impose_hermicity_of_LHS(factory);
        EXPECT_FALSE(split.has_value());
        EXPECT_EQ(msr.LHS(), comp_b);
        expect_approximately_equal(msr.RHS(), factory({Monomial{re_a, 1.0}}));
    }

    TEST_F(Symbolic_MomentSubstitutionRule_Split, NoSplit_HermitianEqualsScalar) {
        const auto& factory = this->get_factory();
        MomentSubstitutionRule msr{factory, factory({Monomial{re_a, 1.0}, Monomial{id, -5.0}})};
        auto split = msr.impose_hermicity_of_LHS(factory);
        EXPECT_FALSE(split.has_value());
        EXPECT_EQ(msr.LHS(), re_a);
        expect_approximately_equal(msr.RHS(), factory({Monomial{id, 5.0}}));
    }

    TEST_F(Symbolic_MomentSubstitutionRule_Split, BadSplit_HermitianEqualsComplexScalar) {
        const auto& factory = this->get_factory();
        MomentSubstitutionRule msr{factory, factory({Monomial{re_a, 1.0}, Monomial{id, std::complex{0.0, -5.0}}})};
        auto split = msr.impose_hermicity_of_LHS(factory);
        ASSERT_TRUE(split.has_value());
        EXPECT_EQ(msr.LHS(), re_a);
        expect_approximately_equal(msr.RHS(), Polynomial::Zero());
        EXPECT_EQ(MomentSubstitutionRule::get_difficulty(split.value()),
                  MomentSubstitutionRule::PolynomialDifficulty::Contradiction); // Split rule is 0 = 5
    }


    TEST_F(Symbolic_MomentSubstitutionRule_Split, Split_HermitianEqualsComplex) {
        const auto& factory = this->get_factory();
        MomentSubstitutionRule msr{factory, factory({Monomial{re_d, 1.0}, Monomial{comp_b, -1.0}, Monomial{id, -1.0}})};
        auto split = msr.impose_hermicity_of_LHS(factory);
        EXPECT_EQ(msr.LHS(), re_d);
        // d = Re(d) = Re(b) + 1
        expect_approximately_equal(msr.RHS(),
                                   factory({Monomial{comp_b, 0.5}, Monomial{comp_b, 0.5, true}, Monomial{id, 1.0}}));
        // D -> Re(B) + 1
        ASSERT_TRUE(split.has_value()); // split should be Im(D) = 0 = Im(B) ->
        expect_approximately_equal(split.value(),
                                   factory({Monomial{comp_b, std::complex{0.0, -0.5}},
                                            Monomial{comp_b, std::complex{0.0, 0.5}, true}})); // Im(B) = 0.

        EXPECT_EQ(MomentSubstitutionRule::get_difficulty(split.value()),
                  MomentSubstitutionRule::PolynomialDifficulty::NonorientableRule);
    }

    TEST_F(Symbolic_MomentSubstitutionRule_Split, Split_AntiHermitianEqualsComplex) {
        const auto& factory = this->get_factory();
        MomentSubstitutionRule msr{factory, factory({Monomial{im_f, 1.0}, Monomial{comp_b, -1.0}, Monomial{id, -1.0}})};
        auto split = msr.impose_hermicity_of_LHS(factory);
        EXPECT_EQ(msr.LHS(), im_f);
        expect_approximately_equal(msr.RHS(),
                                   factory({Monomial{comp_b, std::complex{0.5, 0.0}},
                                            Monomial{comp_b, std::complex{-0.5, 0.0}, true}}));// f = i Im(f) = i Im(b).
        // F -> Re(B) + 1
        ASSERT_TRUE(split.has_value()); // split should be Im(f) = 0 = Im(B) ->
        expect_approximately_equal(split.value(),
                                   factory({Monomial{comp_b, 0.5}, Monomial{comp_b, 0.5, true},
                                            Monomial{id, 1.0}})); // 0 = Re(f) = Re(b) + 1.

        EXPECT_EQ(MomentSubstitutionRule::get_difficulty(split.value()),
                  MomentSubstitutionRule::PolynomialDifficulty::NonorientableRule);
    }


}
