// SPDX-License-Identifier: Apache-2.0
#include <gtest/gtest.h>
#include <pybind11/embed.h>

#include "xstudio/atoms.hpp"
#include "xstudio/embedded_python/embedded_python.hpp"
#include "xstudio/utility/caf_helpers.hpp"
#include "xstudio/utility/helpers.hpp"
#include "xstudio/utility/logging.hpp"
#include "xstudio/utility/uuid.hpp"

using namespace xstudio::utility;
using namespace xstudio::embedded_python;

TEST(EmbeddedPython, Test) {
    start_logger(spdlog::level::debug);
    EmbeddedPython s{"test"};

    s.exec("print ('hello')");

    auto a = s.eval("print ('hello')");

    auto b = s.eval("1+1");
    EXPECT_EQ(b, 2);

    auto c = s.eval("a+b", nlohmann::json({{"a", 1}, {"b", 2}}));
    EXPECT_EQ(c, 3);

    auto d =
        s.eval_locals("c=locals()['a']+locals()['b']", nlohmann::json({{"a", 1}, {"b", 2}}));
    EXPECT_EQ(d["c"], 3);

    EXPECT_FALSE(s.connect(14441));
    try {
        s.exec("print ('hello')");
    } catch (...) {
    }
    try {
        s.exec("print hello')");
    } catch (...) {
    }
    try {
        s.exec("print ('hello')");
    } catch (...) {
    }

    // try{
    // 	auto u = s.create_session(true);
    // 	auto c = s.eval("'"+to_string(u)+"' in xstudio_sessions");

    // }catch(...){}
}

TEST(EmbeddedPython, SpawnTrackThroughLocalConnection) {
    auto &system = xstudio::CafActorSystem::system();
    caf::scoped_actor self{system};
    EmbeddedPython python{"test"};
    python.setup();

    pybind11::globals()["test_actor"] = pybind11::cast(caf::actor_cast<caf::actor>(self));
    try {
        python.exec(
            "link = Link()\n"
            "assert link.connect_local(test_actor)\n"
            "track = link.remote_spawn('Track', 'Test Track', FrameRate(24), MediaType.MT_IMAGE)\n");
    } catch (const pybind11::error_already_set &error) {
        FAIL() << error.what();
    }

    auto track = pybind11::cast<caf::actor>(pybind11::globals()["track"]);
    ASSERT_TRUE(track);
    EXPECT_EQ(request_receive<std::string>(*self, track, name_atom_v), "Test Track");
    self->send_exit(track, caf::exit_reason::user_shutdown);
    python.exec("del track, link, test_actor");
}
