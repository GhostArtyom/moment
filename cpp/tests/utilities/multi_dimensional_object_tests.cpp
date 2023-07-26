/**
 * multi_dimensional_object_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "utilities/multi_dimensional_object.h"

namespace Moment::Tests {

    using MDO = MultiDimensionalObject<int, std::vector<int>, std::span<const int>, true>;
    using RowMajorMDO = MultiDimensionalObject<int, std::vector<int>, std::span<const int>, false>;

    TEST(Utilities_MultiDimensionalObject, Empty) {
        MDO empty{MDO::Index{}};

        EXPECT_EQ(empty.Dimensions.size(), 0);
        EXPECT_EQ(empty.Strides.size(), 0);
        EXPECT_EQ(empty.DimensionCount, 0);
        EXPECT_EQ(empty.ElementCount, 0);
    }

    TEST(Utilities_MultiDimensionalObject, Vector) {
        MDO vec{{3}};

        ASSERT_EQ(vec.Dimensions.size(), 1);
        EXPECT_EQ(vec.Dimensions[0], 3);
        ASSERT_EQ(vec.Strides.size(), 1);
        EXPECT_EQ(vec.Strides[0], 1);
        EXPECT_EQ(vec.DimensionCount, 1);
        EXPECT_EQ(vec.ElementCount, 3);

        EXPECT_EQ(vec.index_to_offset_no_checks(MDO::Index{0}), 0);
        EXPECT_EQ(vec.index_to_offset_no_checks(MDO::Index{1}), 1);
        EXPECT_EQ(vec.index_to_offset_no_checks(MDO::Index{2}), 2);
        EXPECT_EQ(vec.offset_to_index_no_checks(0), MDO::Index{0});
        EXPECT_EQ(vec.offset_to_index_no_checks(1), MDO::Index{1});
        EXPECT_EQ(vec.offset_to_index_no_checks(2), MDO::Index{2});
    }

    TEST(Utilities_MultiDimensionalObject, Matrix) {
        MDO mat{{3, 2}}; // 3 rows, 2 cols

        ASSERT_EQ(mat.Dimensions.size(), 2);
        EXPECT_EQ(mat.Dimensions[0], 3);
        EXPECT_EQ(mat.Dimensions[1], 2);
        EXPECT_EQ(mat.DimensionCount, 2);
        ASSERT_EQ(mat.Strides.size(), 2);
        EXPECT_EQ(mat.Strides[0], 1);
        EXPECT_EQ(mat.Strides[1], 3);
        EXPECT_EQ(mat.ElementCount, 6);

        EXPECT_EQ(mat.index_to_offset_no_checks(MDO::Index{0, 0}), 0);
        EXPECT_EQ(mat.index_to_offset_no_checks(MDO::Index{1, 0}), 1);
        EXPECT_EQ(mat.index_to_offset_no_checks(MDO::Index{2, 0}), 2);
        EXPECT_EQ(mat.index_to_offset_no_checks(MDO::Index{0, 1}), 3);
        EXPECT_EQ(mat.index_to_offset_no_checks(MDO::Index{1, 1}), 4);
        EXPECT_EQ(mat.index_to_offset_no_checks(MDO::Index{2, 1}), 5);
        EXPECT_EQ(mat.offset_to_index_no_checks(0), (MDO::Index{0, 0}));
        EXPECT_EQ(mat.offset_to_index_no_checks(1), (MDO::Index{1, 0}));
        EXPECT_EQ(mat.offset_to_index_no_checks(2), (MDO::Index{2, 0}));
        EXPECT_EQ(mat.offset_to_index_no_checks(3), (MDO::Index{0, 1}));
        EXPECT_EQ(mat.offset_to_index_no_checks(4), (MDO::Index{1, 1}));
        EXPECT_EQ(mat.offset_to_index_no_checks(5), (MDO::Index{2, 1}));
    }

    TEST(Utilities_MultiDimensionalObject, RowMajorMatrix) {
        RowMajorMDO mat{{3, 2}}; // 3 rows, 2 cols

        ASSERT_EQ(mat.Dimensions.size(), 2);
        EXPECT_EQ(mat.Dimensions[0], 3);
        EXPECT_EQ(mat.Dimensions[1], 2);
        EXPECT_EQ(mat.DimensionCount, 2);
        ASSERT_EQ(mat.Strides.size(), 2);
        EXPECT_EQ(mat.Strides[0], 2);
        EXPECT_EQ(mat.Strides[1], 1);
        EXPECT_EQ(mat.ElementCount, 6);

        EXPECT_EQ(mat.index_to_offset_no_checks(RowMajorMDO::Index{0, 0}), 0);
        EXPECT_EQ(mat.index_to_offset_no_checks(RowMajorMDO::Index{0, 1}), 1);
        EXPECT_EQ(mat.index_to_offset_no_checks(RowMajorMDO::Index{1, 0}), 2);
        EXPECT_EQ(mat.index_to_offset_no_checks(RowMajorMDO::Index{1, 1}), 3);
        EXPECT_EQ(mat.index_to_offset_no_checks(RowMajorMDO::Index{2, 0}), 4);
        EXPECT_EQ(mat.index_to_offset_no_checks(RowMajorMDO::Index{2, 1}), 5);
        EXPECT_EQ(mat.offset_to_index_no_checks(0), (RowMajorMDO::Index{0, 0}));
        EXPECT_EQ(mat.offset_to_index_no_checks(1), (RowMajorMDO::Index{0, 1}));
        EXPECT_EQ(mat.offset_to_index_no_checks(2), (RowMajorMDO::Index{1, 0}));
        EXPECT_EQ(mat.offset_to_index_no_checks(3), (RowMajorMDO::Index{1, 1}));
        EXPECT_EQ(mat.offset_to_index_no_checks(4), (RowMajorMDO::Index{2, 0}));
        EXPECT_EQ(mat.offset_to_index_no_checks(5), (RowMajorMDO::Index{2, 1}));
    }

    TEST(Utilities_MultiDimensionalObject, Tensor) {
        MDO tensor{{10, 5, 4}};
        ASSERT_EQ(tensor.Dimensions.size(), 3);
        EXPECT_EQ(tensor.Dimensions[0], 10);
        EXPECT_EQ(tensor.Dimensions[1], 5);
        EXPECT_EQ(tensor.Dimensions[2], 4);
        EXPECT_EQ(tensor.DimensionCount, 3);
        ASSERT_EQ(tensor.Strides.size(), 3);
        EXPECT_EQ(tensor.Strides[0], 1);
        EXPECT_EQ(tensor.Strides[1], 10);
        EXPECT_EQ(tensor.Strides[2], 50);
        EXPECT_EQ(tensor.ElementCount, 200);

        EXPECT_EQ(tensor.index_to_offset_no_checks(MDO::Index{1, 2, 3}), 171);
        EXPECT_EQ(tensor.offset_to_index_no_checks(171), (MDO::Index{1, 2 ,3}));
    }
}
