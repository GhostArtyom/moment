/**
 * multi_dimensional_object.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "multi_dimensional_index_iterator.h"

#include <algorithm>
#include <concepts>
#include <iterator>
#include <numeric>
#include <span>
#include <vector>

namespace Moment {

    /**
     * An object with indexed dimensions, which can be converted to a numerical offset.
     * @tparam index_elem_t The integral type underlining the index.
     * @tparam index_storage_t The storage of multi-dimensional indices.
     * @tparam index_view_t A view to a multi-dimensional index.
     * @tparam last_index_major True if the offset is last-index-major (e.g. col-major in two dimensions).
     */
    template<std::integral index_elem_t = size_t,
            typename index_storage_t = std::vector<size_t>,
            typename index_view_t = std::span<const size_t>,
            bool last_index_major = true>
    struct MultiDimensionalObject {
    public:
        using IndexElement = index_elem_t;
        using Index = index_storage_t;
        using IndexView = index_view_t;
        using IndexIterator = MultiDimensionalIndexIterator<last_index_major, Index>;

        /** True if storage order is last-index-major (e.g. col-major in two dimensions). */
        constexpr static const bool LastIndexMajor = last_index_major;

        /** The dimensions of the object. */
        const Index Dimensions;

        /** The distance in offset represented by each dimension. */
        const Index Strides;

        /** The number of dimensions in the object */
        const IndexElement DimensionCount;

        /** The number of unique elements represented by the object. */
        const IndexElement ElementCount;

        explicit MultiDimensionalObject(Index dimensions)
            : Dimensions{std::move(dimensions)}, Strides{calculateStrides(Dimensions)},
              DimensionCount{static_cast<IndexElement>(Dimensions.size())},
              ElementCount{calculateNumberOfElements(Dimensions)} { }

        /**
         * Converts an index to its numerical offset within the tensor.
         */
        [[nodiscard]] constexpr IndexElement index_to_offset_no_checks(IndexView indices) const noexcept {
            return std::inner_product(indices.begin(), indices.end(),
                                      this->Strides.begin(), static_cast<IndexElement>(0));
        }

        /**
         * Converts a numerical offset to its index within the tensor.
         */
        [[nodiscard]] Index offset_to_index_no_checks(IndexElement offset) const {
            Index output;
            output.reserve(this->DimensionCount);
            if constexpr (LastIndexMajor) {
                for (size_t n = 0; n < this->DimensionCount; ++n) {
                    output.emplace_back(offset % this->Dimensions[n]);
                    offset /= this->Dimensions[n];
                }
            } else {
                for (size_t n = 0; n < this->DimensionCount; ++n) {
                    output.emplace_back(offset / this->Strides[n]);
                    offset %= this->Strides[n];
                }
            }
            return output;
        }


    private:
        [[nodiscard]] static constexpr Index calculateStrides(const IndexView dimensions) {
            Index strides;
            if constexpr (LastIndexMajor) {
                strides.reserve(dimensions.size());
                std::exclusive_scan(dimensions.begin(), dimensions.end(),
                                    std::back_inserter(strides),
                                    static_cast<IndexElement>(1), std::multiplies{});
            } else {
                strides.assign(dimensions.size(), 1);
                std::exclusive_scan(dimensions.rbegin(), dimensions.rend(),
                                    strides.rbegin(),
                                    static_cast<IndexElement>(1), std::multiplies{});
            }
            return strides;
        }

        [[nodiscard]] static constexpr IndexElement calculateNumberOfElements(const IndexView dimensions) {
            if (dimensions.empty()) {
                return 0;
            }
            return std::reduce(dimensions.begin(), dimensions.end(), static_cast<IndexElement>(1), std::multiplies{});
        }

    };
}