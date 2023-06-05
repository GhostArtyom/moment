/**
 * create_moment_rules.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "../mex_function.h"
#include "integer_types.h"

#include "import/read_polynomial.h"

#include <map>
#include <memory>

namespace Moment {
    class MatrixSystem;
    class MomentSubstitutionRulebook;
    class Polynomial;
    class PolynomialFactory;
    class SymbolTable;
}

namespace Moment::mex::functions {

    class OpSeqRuleSpecification;

    class CreateMomentRulesParams : public SortedInputs {
    public:
        /** The matrix system the ruleset is associated with */
        uint64_t matrix_system_key = 0;

        /** The ID of an existing ruleset  */
        uint64_t existing_rule_key = 0;

        /** True = Also generate extra rules from currently-known factors. */
        bool infer_from_factors = true;

        /** True = Allow for the creation of new symbols, if an operator sequence is unrecognized */
        bool create_missing_symbols = true;

        /** True = Attempt to merge into existing ruleset */
        bool merge_into_existing = false;

        /** The name to give the rules object (if any) */
        std::string human_readable_name;

        /** The 'precision' limit, such that if a value is less than eps*mult it is treated as zero. */
        double zero_tolerance = 1.0;

        /** How the input to the create-rules command is supplied */
        enum class InputMode {
            Unknown,
            /** List of symbol ID / double value scalar substitutions */
            SubstitutionList,
            /** Polynomials, expressed as symbols. */
            FromSymbolIds,
            /** Polynomials, expressed as operator sequences. */
            FromOperatorSequences
        } input_mode = InputMode::SubstitutionList;

        /** How should symbol IDs be ordered */
        enum class SymbolOrdering {
            Unknown,
            /** Sort symbols by their ID (i.e. order of creation) */
            ById,
            /** Sort symbols by hash of their associated operators */
            ByOperatorHash
        } ordering = SymbolOrdering::ById;

        /** Direct substitutions, if specified. */
        std::map<symbol_name_t, std::complex<double>> sub_list;

        /** Direct set of symbol combos, if specified. */
        std::vector<std::vector<raw_sc_data>> raw_symbol_polynomials;

        /** Weighted operator sequences, if specified. */
        std::unique_ptr<OpSeqRuleSpecification> raw_op_seq_polynomials;

    public:
        /** Constructor */
        explicit CreateMomentRulesParams(SortedInputs &&rawInput);

        /** Destructor */
        ~CreateMomentRulesParams() noexcept;

    private:
        void parse_as_sublist(const matlab::data::Array& data);

        void parse_as_symbol_polynomials(const matlab::data::Array& data);

        void parse_as_op_seq_polynomials(const matlab::data::Array& data);

    };

    class CreateMomentRules : public ParameterizedMexFunction<CreateMomentRulesParams,
                                                              MEXEntryPointID::CreateMomentRules> {
    public:
        explicit CreateMomentRules(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage);

        void extra_input_checks(CreateMomentRulesParams &input) const override;

    protected:
        void operator()(IOArgumentRange output, CreateMomentRulesParams &input) override;

    private:
        /** Get appropriate Polynomial factory */
        [[nodiscard]] std::unique_ptr<PolynomialFactory>
        make_factory(SymbolTable &symbols, const CreateMomentRulesParams &input) const;

        [[nodiscard]] std::unique_ptr<MomentSubstitutionRulebook>
        create_rulebook(MatrixSystem &system, CreateMomentRulesParams &input) const;

        [[nodiscard]] std::unique_ptr<MomentSubstitutionRulebook>
        create_rulebook_from_sublist(MatrixSystem &system, CreateMomentRulesParams &input) const;

        [[nodiscard]] std::unique_ptr<MomentSubstitutionRulebook>
        create_rulebook_from_symbols(MatrixSystem &system, CreateMomentRulesParams &input) const;

        [[nodiscard]] std::unique_ptr<MomentSubstitutionRulebook>
        create_rulebook_from_existing_sequences(MatrixSystem &system, CreateMomentRulesParams &input) const;

        [[nodiscard]] std::unique_ptr<MomentSubstitutionRulebook>
        create_rulebook_from_new_sequences(MatrixSystem &system, CreateMomentRulesParams &input) const;
    };
}