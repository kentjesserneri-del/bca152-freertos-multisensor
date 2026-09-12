#include <unity.h>
#include "display_logic.h"

void setUp(void) {}
void tearDown(void) {}

void test_next_temperature_goes_to_humidity(void) {
    TEST_ASSERT_EQUAL(DisplayMode::HUMIDITY, nextDisplayMode(DisplayMode::TEMPERATURE));
}

void test_next_wraps_from_motion_to_temperature(void) {
    TEST_ASSERT_EQUAL(DisplayMode::TEMPERATURE, nextDisplayMode(DisplayMode::MOTION));
}

void test_previous_wraps_from_temperature_to_motion(void) {
    TEST_ASSERT_EQUAL(DisplayMode::MOTION, previousDisplayMode(DisplayMode::TEMPERATURE));
}

void test_previous_humidity_goes_to_temperature(void) {
    TEST_ASSERT_EQUAL(DisplayMode::TEMPERATURE, previousDisplayMode(DisplayMode::HUMIDITY));
}

void test_full_forward_cycle_returns_to_start(void) {
    DisplayMode m = DisplayMode::TEMPERATURE;
    for (int i = 0; i < 4; i++) m = nextDisplayMode(m);
    TEST_ASSERT_EQUAL(DisplayMode::TEMPERATURE, m);
}

void test_full_backward_cycle_returns_to_start(void) {
    DisplayMode m = DisplayMode::TEMPERATURE;
    for (int i = 0; i < 4; i++) m = previousDisplayMode(m);
    TEST_ASSERT_EQUAL(DisplayMode::TEMPERATURE, m);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_next_temperature_goes_to_humidity);
    RUN_TEST(test_next_wraps_from_motion_to_temperature);
    RUN_TEST(test_previous_wraps_from_temperature_to_motion);
    RUN_TEST(test_previous_humidity_goes_to_temperature);
    RUN_TEST(test_full_forward_cycle_returns_to_start);
    RUN_TEST(test_full_backward_cycle_returns_to_start);
    return UNITY_END();
}