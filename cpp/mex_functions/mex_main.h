/**
 * mex_main.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "MatlabDataArray.hpp"
#include "mex.hpp"

#include "error_codes.h"
#include "utilities/io_parameters.h"
#include "functions/function_list.h"

#include <memory>
#include <vector>
#include <iterator>

namespace NPATK::mex {

    class StorageManager;

    namespace functions {
        class MexFunction;
    }

    class MexMain  {
    private:
        std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr;
        StorageManager& persistentStorage;

    public:
        explicit MexMain(std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr);

        void operator()(IOArgumentRange outputs, IOArgumentRange inputs);


    private:
        /**
         * Ascertain the function being requested
         * @param inputs The input arguments. If function name is found, it is popped from front of this list.
         * @return The ID of the function implied by the input parameters.
         */
        [[nodiscard]] functions::MEXEntryPointID get_function_id(IOArgumentRange& inputs);

        /**
         * Transforms the raw inputs into a structured "SortedInputs" object
         * @param func The mex function
         * @param inputs The raw inputs
         * @return A structured SortedInputs object
         */
        [[nodiscard]] std::unique_ptr<SortedInputs> clean_inputs(const functions::MexFunction& func, IOArgumentRange& inputs);

        /**
         * Apply further function-specific individual transform of the structured inputs.
         * @param func The mex function
         * @param inputs Owning pointer to the structured inputs
         * @return Owning pointer to the possibly transformed inputs
         */
        [[nodiscard]] std::unique_ptr<SortedInputs> transform_and_validate(const functions::MexFunction& func,
                                                       std::unique_ptr<SortedInputs> inputs,
                                                       const IOArgumentRange& outputs);

        /**
         * Checks that the inputs match expected parameters (and only expected parameters), and that their number is
         * within a function-specified range
         * @param func The mex function
         * @param inputs The structured inputs
         */
        void validate_inputs(const functions::MexFunction& func, const SortedInputs& inputs);

        void validate_outputs(const functions::MexFunction& func, const IOArgumentRange& outputs);
    };
}