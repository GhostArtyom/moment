/**
 * unique_operator_strings.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "operator_sequence.h"
#include "context.h"

#include <cassert>


namespace NPATK {
    /**
     * Range over all unique permutations of operators in the supplied context.
     */
    class OperatorSequenceGenerator {
    private:
        /** Context to pull operators from */
        const Context& context;

        /** List of unique sequences */
        std::vector<OperatorSequence> unique_sequences;

    public:
        /** The maximum length of operator sequence */
        const size_t sequence_length;

    public:
        /**
         * Generates all unique permutations of operator sequences, up to sequence_length.
         * @param operatorContext
         * @param sequence_length
         */
        OperatorSequenceGenerator(const Context& operatorContext, size_t sequence_length);

        /**
         * Create a generator with a list of pre-calculated operator sequences
         * @param operatorContext Reference to context
         * @param max_length The longest sequence in the pre-computed list
         * @param preComputedSequences An ordered list of operator sequences the generator will produce
         */
        OperatorSequenceGenerator(const Context& operatorContext, size_t max_length,
                                  std::vector<OperatorSequence>&& preComputedSequences)
                                  : context{operatorContext}, sequence_length{max_length},
                                    unique_sequences{std::move(preComputedSequences)} { }

        /**
         * Creates a generator for the piece-wise conjugated OperatorSequences of this generator.
         */
        [[nodiscard]] OperatorSequenceGenerator conjugate() const;


        [[nodiscard]] constexpr auto begin() const noexcept { return unique_sequences.begin(); }
        [[nodiscard]] constexpr auto end() const noexcept { return unique_sequences.end(); }
        [[nodiscard]] constexpr size_t size() const noexcept { return unique_sequences.size(); }
        [[nodiscard]] constexpr const OperatorSequence& operator[](size_t index) const noexcept {
            assert(index < unique_sequences.size());
            return this->unique_sequences[index];
        };

    };
}