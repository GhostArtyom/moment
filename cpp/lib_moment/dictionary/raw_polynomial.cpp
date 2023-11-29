/**
 * raw_polynomial.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "raw_polynomial.h"

#include "scenarios/contextual_os.h"

#include "utilities/format_factor.h"
#include "utilities/float_utils.h"

#include <sstream>

namespace Moment {

    std::string RawPolynomial::to_string(const Context& context) const {
        std::stringstream ss;
        ContextualOS cSS{ss, context};
        cSS.format_info.show_braces = true;

        bool done_once = false;
        for (const auto& elem : this->data) {
            format_factor(cSS, elem.weight, false, done_once);
            cSS << elem.sequence;
            done_once = true;
        }

        return ss.str();
    }
}