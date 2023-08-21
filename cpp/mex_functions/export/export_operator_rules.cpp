/**
 * export_operator_rules.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "export_operator_rules.h"

#include "scenarios/algebraic/operator_rulebook.h"

#include "utilities/reporting.h"
#include "error_codes.h"

#include <algorithm>


namespace Moment::mex {
    matlab::data::CellArray
    OperatorRuleExporter::operator()(const Algebraic::OperatorRulebook& rules) {
        matlab::data::ArrayFactory factory;
        matlab::data::CellArray output = factory.createArray<matlab::data::Array>({1, rules.rules().size()});

        auto write_iter = output.begin();
        for (const auto& [lhs_hash, rule] : rules.rules()) {
            assert(write_iter != output.end());

            // Is rule a negation?
            const size_t rhs_index = rule.negated() ? 2 : 1;

            // Create cell array pair
            matlab::data::CellArray rule_pair = factory.createArray<matlab::data::Array>({1, (rule.negated() ? 3U : 2U)});

            // Copy LHS
            rule_pair[0] = factory.createArray<uint64_t>({1, rule.LHS().size()});
            matlab::data::TypedArrayRef<uint64_t> rule_lhs = rule_pair[0];
            auto lhs_write_iter = rule_lhs.begin();
            std::copy(rule.LHS().begin(), rule.LHS().end(), lhs_write_iter);
            if (this->matlab_indices) {
                for (auto &x: rule_lhs) {
                    ++x;
                }
            }

            // Add minus sign, if negated
            if (rule.negated()) {
                rule_pair[1] = factory.createCharArray("-");
            }

            // Copy RHS
            if (rule.implies_zero()) {
                if (this->matlab_indices) {
                    rule_pair[rhs_index] = factory.createScalar<uint64_t>(0);
                } else {
                    rule_pair[rhs_index] = factory.createScalar<int64_t>(-1);
                }

            } else {
                rule_pair[rhs_index] = factory.createArray<uint64_t>({1, rule.RHS().size()});
                matlab::data::TypedArrayRef<uint64_t> rule_rhs = rule_pair[rhs_index];
                auto rhs_write_iter = rule_rhs.begin();
                std::copy(rule.RHS().begin(), rule.RHS().end(), rhs_write_iter);

                // Adjust indices
                if (this->matlab_indices) {
                    for (auto &y: rule_rhs) {
                        ++y;
                    }
                }
            }

            // Move to outer array
            *write_iter = std::move(rule_pair);
            ++write_iter;
        }

        return output;
    }
}