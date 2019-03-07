/*******************************************************************************
 * Copyright (C) 2018 Pavel Krupets                                            *
 *******************************************************************************/

#include "test.h"

#include "rolling_values.h"

void test_calculateRolling16BitValueDifference()
{
    START_TEST_SUITE();

    START_TEST(should_return_5_if_old_value_is_0_and_new_is_5);
        ASSERT_UINT32_EQUAL(5, calculateRolling16BitValueDifference(0, 5));
    END_TEST();

    START_TEST(should_return_3_if_old_value_is_65533_and_new_is_0);
        ASSERT_UINT32_EQUAL(3, calculateRolling16BitValueDifference(65533, 0));
    END_TEST();

    START_TEST(should_return_5_if_old_value_is_65533_and_new_is_2);
        ASSERT_UINT32_EQUAL(5, calculateRolling16BitValueDifference(65533, 2));
    END_TEST();

    END_TEST_SUITE();
}
