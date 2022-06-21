/**
 * version.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "../../version.h"
#include "mex_function.h"

namespace NPATK::mex::functions {

    class Version : public MexFunction {
    public:
        explicit Version(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final {
            return std::move(input);
        }

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;
    };

}