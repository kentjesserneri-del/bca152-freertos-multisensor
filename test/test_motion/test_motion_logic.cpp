#include <unity.h>
#include "motion_logic.h"

void setUp(void) {}
void tearDown(void) {}

void test_motion_now_is_always_active(void) {
    TEST_ASSERT_EQUAL(SystemState::ACTIVE, evaluateSystemState(true, 0, 15000));
}

void test_motion_now_overrides_large_idle_time(void) {
    TEST_ASSERT_EQUAL(SystemState::ACTIVE, evaluateSystemState(true, 999999, 15000));
}

void test_idle_below_timeout_is_active(void) {
    TEST_ASSERT_EQUAL(SystemState::ACTIVE, evaluateSystemState(false, 14999, 15000));
}

void test_idle_exactly_at_timeout_is_inactive(void) {
    TEST_ASSERT_EQUAL(SystemState::INACTIVE, evaluateSystemState(false, 15000, 15000));
}

void test_idle_beyond_timeout_is_inactive(void) {
    TEST_ASSERT_EQUAL(SystemState::INACTIVE, evaluateSystemState(false, 20000, 15000));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_motion_now_is_always_active);
    RUN_TEST(test_motion_now_overrides_large_idle_time);
    RUN_TEST(test_idle_below_timeout_is_active);
    RUN_TEST(test_idle_exactly_at_timeout_is_inactive);
    RUN_TEST(test_idle_beyond_timeout_is_inactive);
    return UNITY_END();
}