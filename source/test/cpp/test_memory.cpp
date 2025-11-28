#include "ccore/c_target.h"
#include "clibuv/uv.h"

#include "cunittest/cunittest.h"

using namespace ncore;

UNITTEST_SUITE_BEGIN(clibuv_memory)
{
    UNITTEST_FIXTURE(basic)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        UNITTEST_TEST(test)
        {
            uint64_t free_mem        = uv_get_free_memory();
            uint64_t total_mem       = uv_get_total_memory();
            uint64_t constrained_mem = uv_get_constrained_memory();
            uint64_t available_mem   = uv_get_available_memory();

            CHECK_GT(free_mem, (uint64_t)0);
            CHECK_GT(total_mem, (uint64_t)0);
            CHECK_EQUAL(constrained_mem, (uint64_t)0);
            CHECK_GT(available_mem, (uint64_t)0);
        }
    }
}
UNITTEST_SUITE_END
