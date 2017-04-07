#pragma once

#define UNIT_TEST_BEG(module_name) namespace test { namespace module_name {

#define UNIT_TEST_END }}

#define UNIT_TEST(module_name) namespace test { namespace module_name { void test(); }}

#define RUN_TEST(module_name) test::module_name::test()