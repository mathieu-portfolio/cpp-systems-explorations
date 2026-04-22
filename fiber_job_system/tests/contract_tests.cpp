#include <gtest/gtest.h>

#include "fiber_job_system.hpp"

#include <functional>
#include <memory>
#include <stdexcept>

namespace
{
class DummyTask final : public FiberTask
{
public:
    FiberStepResult run() override
    {
        return FiberStepResult::Complete;
    }
};
}

TEST(FiberJobSystemContract, RejectsZeroWorkers)
{
    EXPECT_THROW(FiberJobSystem scheduler(0), std::invalid_argument);
}

TEST(FiberJobSystemContract, RejectsEmptySubmission)
{
    FiberJobSystem scheduler(2);
    EXPECT_THROW(scheduler.submit(std::function<void()>{}), std::invalid_argument);
}

TEST(FiberJobSystemContract, RejectsEmptyResumableSubmission)
{
    FiberJobSystem scheduler(2);
    EXPECT_THROW(scheduler.submit_resumable(std::function<FiberStepResult()>{}), std::invalid_argument);
}

TEST(FiberJobSystemContract, RejectsNullTaskSubmission)
{
    FiberJobSystem scheduler(2);
    EXPECT_THROW(scheduler.submit_task(nullptr), std::invalid_argument);
}

TEST(FiberJobSystemContract, RejectsYieldOutsideRunningFiber)
{
    FiberJobSystem scheduler(2);
    EXPECT_THROW(scheduler.yield_current(), std::logic_error);
}
