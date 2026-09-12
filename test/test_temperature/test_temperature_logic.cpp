#include <unity.h>
#include "temperature_logic.h"

void setUp(void) {}
void tearDown(void) {}

void test_low_boundary_is_still_normal(void) {
    TEST_ASSERT_EQUAL(AlarmState::NORMAL, evaluateTemperature(18.0f));
}

void test_just_below_low_boundary_is_low(void) {
    TEST_ASSERT_EQUAL(AlarmState::LOW_TEMPERATURE, evaluateTemperature(17.9f));
}

void test_high_boundary_is_still_normal(void) {
    TEST_ASSERT_EQUAL(AlarmState::NORMAL, evaluateTemperature(30.0f));
}

void test_just_above_high_boundary_is_high(void) {
    TEST_ASSERT_EQUAL(AlarmState::HIGH_TEMPERATURE, evaluateTemperature(30.1f));
}

void test_midrange_value_is_normal(void) {
    TEST_ASSERT_EQUAL(AlarmState::NORMAL, evaluateTemperature(24.0f));
}

void test_extreme_low_is_low(void) {
    TEST_ASSERT_EQUAL(AlarmState::LOW_TEMPERATURE, evaluateTemperature(-10.0f));
}

void test_extreme_high_is_high(void) {
    TEST_ASSERT_EQUAL(AlarmState::HIGH_TEMPERATURE, evaluateTemperature(60.0f));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_low_boundary_is_still_normal);
    RUN_TEST(test_just_below_low_boundary_is_low);
    RUN_TEST(test_high_boundary_is_still_normal);
    RUN_TEST(test_just_above_high_boundary_is_high);
    RUN_TEST(test_midrange_value_is_normal);
    RUN_TEST(test_extreme_low_is_low);
    RUN_TEST(test_extreme_high_is_high);
    return UNITY_END();
}