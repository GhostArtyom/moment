/**
 * inflation_context.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "../context.h"

#include "causal_network.h"

#include "utilities/dynamic_bitset.h"

#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace NPATK {


    class InflationContext : public Context {
    public:
        /** Extra operator information for inflation scenario */
        struct ICOperatorInfo {
            oper_name_t global_id;
            oper_name_t observable;
            oper_name_t flattenedSourceIndex;
            oper_name_t outcome;

        public:
            ICOperatorInfo(oper_name_t id, oper_name_t observable, oper_name_t flattenedIndex, oper_name_t outcome)
                : global_id{id}, observable{observable}, flattenedSourceIndex{flattenedIndex}, outcome{outcome} { }

            /**
            * Predicate: true if the operator id of LHS is less than that of RHS.
            */
            struct OrderByID {
                constexpr OrderByID() noexcept = default;
                constexpr bool operator()(const ICOperatorInfo &lhs, const ICOperatorInfo &rhs) const noexcept {
                    return lhs.global_id < rhs.global_id;
                }
            };

            /**
             * Predicate: true if lhs != rhs, but both are part of same observable.
             */
            struct IsOrthogonal {
                constexpr IsOrthogonal() noexcept = default;
                constexpr bool operator()(const ICOperatorInfo &lhs, const ICOperatorInfo &rhs) const noexcept {
                    // Not in same version of same observable, therefore not automatically orthogonal.
                    if ((lhs.observable != rhs.observable) || (lhs.flattenedSourceIndex != rhs.flattenedSourceIndex)) {
                        return false;
                    }
                    return (lhs.global_id != rhs.global_id);
                }
            };

            /**
             * Predicate: true if lhs == rhs, and both are part of same observable.
             */
            struct IsRedundant {
                constexpr IsRedundant() noexcept = default;
                constexpr bool operator()(const ICOperatorInfo &lhs, const ICOperatorInfo &rhs) const noexcept {
                    return (lhs.global_id == rhs.global_id);
                }
            };
        };

        /** Augment base-network observable with extra info regarding inflated variants */
        struct ICObservable : public Observable {
        public:
            struct Variant {
                oper_name_t flat_index;
                std::vector<oper_name_t> indices;
                std::map<oper_name_t, oper_name_t> source_variants;
                DynamicBitset<uint64_t> connected_sources;

                /** True, if no overlapping sources */
                [[nodiscard]] bool independent(const Variant& other) const noexcept;

            friend class ::NPATK::InflationContext::ICObservable;
            private:
                Variant(oper_name_t index,
                        std::vector<oper_name_t>&& vecIndex,
                        std::map<oper_name_t, oper_name_t>&& srcVariants,
                        const DynamicBitset<uint64_t>& connected_sources);
            };

        private:
            const InflationContext& context;

        public:
            const oper_name_t operator_offset;
            const oper_name_t variant_count;
            const std::vector<Variant> variants;


        public:
            ICObservable(const InflationContext& context, const Observable& baseObs,
                         size_t inflation_level, oper_name_t offset);

            static std::vector<Variant> make_variants(const CausalNetwork& network,
                                                      const Observable &baseObs,
                                                      size_t inflation_level);

            /** Get variant by non-flat index */
            const Variant& variant(std::span<const oper_name_t> indices) const;

        };

    private:
        CausalNetwork base_network;
        size_t inflation;
        std::vector<ICOperatorInfo> operator_info;

        std::vector<ICObservable> inflated_observables;

    public:
        bool additional_simplification(std::vector<oper_name_t> &op_sequence, bool &negate) const override;

        /**
         * Create a causal network context, for inflating.
         * @param network Causal network
         */
        InflationContext(CausalNetwork network, size_t inflation_level);

        /**
         * Vector of observables associated with context.
         */
        [[nodiscard]] const auto& Observables() const noexcept { return this->inflated_observables; }

        /**
         * Vector of sources associated with context.
         */
        [[nodiscard]] const auto& Sources() const noexcept { return this->base_network.Sources(); }

        /**
         * Level of inflation
         */
         [[nodiscard]] size_t Inflation() const noexcept { return this->inflation; }


        /**
         * Generates a formatted string representation of an operator sequence
         */
        [[nodiscard]] std::string format_sequence(const OperatorSequence& seq) const override;

         /**
          * Output inflation about context
          */
         [[nodiscard]] std::string to_string() const override;

         /**
          * Get operator associated with following triplet:
          * @param observable
          * @param variant
          * @param outcome
          * @return The raw operator ID
          */
        [[nodiscard]] oper_name_t operator_number(oper_name_t observable, oper_name_t variant,
                                                  oper_name_t outcome) const noexcept;

    };
}