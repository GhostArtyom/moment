/**
 * operator_sequence.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "operator_sequence.h"
#include "context.h"

#include <algorithm>
#include <iostream>

namespace NPATK {


    std::ostream& operator<<(std::ostream &os, const NPATK::OperatorSequence &seq) {
        os << seq.context.format_sequence(seq);
        return os;
    }

    void OperatorSequence::to_canonical_form() noexcept {
        // Contextual simplifications
        bool simplify_to_zero = this->context.additional_simplification(this->constituents);
        if (simplify_to_zero) {
            this->constituents.clear();
            this->is_zero = true;
            return;
        }
    }

    OperatorSequence OperatorSequence::conjugate() const {
        OperatorSequence output{*this};
        std::reverse(output.constituents.begin(), output.constituents.end());
        output.to_canonical_form();
        return output;
    }

}