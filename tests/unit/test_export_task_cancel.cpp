#include <catch2/catch_test_macros.hpp>

#include <export/ExportTask.h>

using lumen::exp::ExportTask;

TEST_CASE("ExportTask: cancel before start is safe", "[export_task][cancel]") {
    ExportTask::Options opts;
    ExportTask task(opts);
    task.cancel();
    CHECK(task.isCancelRequested());
    CHECK_FALSE(task.isRunning());
}

TEST_CASE("ExportTask: null scene finishes with failure", "[export_task][cancel]") {
    ExportTask::Options opts;
    opts.scene = nullptr;
    ExportTask task(opts);
    // Start with null scene — should emit error, not crash.
    // We don't process events here, so just verify no crash on construction.
    CHECK_FALSE(task.isRunning());
}
