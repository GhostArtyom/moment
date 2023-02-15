/**
 * algebraic_matrix_system.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "algebraic_matrix_system.h"
#include "algebraic_context.h"

namespace Moment::Algebraic {

    AlgebraicMatrixSystem::AlgebraicMatrixSystem(std::unique_ptr<class AlgebraicContext> contextIn)
            : MatrixSystem{std::move(contextIn)},
              algebraicContext{dynamic_cast<class AlgebraicContext&>(this->Context())} {

    }

    AlgebraicMatrixSystem::AlgebraicMatrixSystem(std::unique_ptr<class Context> contextIn)
            : MatrixSystem{std::move(contextIn)},
              algebraicContext{dynamic_cast<class AlgebraicContext&>(this->Context())} {

    }
}
