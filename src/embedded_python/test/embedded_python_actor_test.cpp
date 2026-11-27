// SPDX-License-Identifier: Apache-2.0
#include <caf/all.hpp>
#include <gtest/gtest.h>

#include "xstudio/atoms.hpp"
#include "xstudio/embedded_python/embedded_python_actor.hpp"
#include "xstudio/playlist/playlist.hpp"


#include "xstudio/utility/serialise_headers.hpp"

#include "xstudio/utility/helpers.hpp"
#include "xstudio/utility/logging.hpp"

using namespace xstudio::utility;
using namespace xstudio::embedded_python;
using namespace xstudio;

using namespace caf;


ACTOR_TEST_SETUP()


TEST(EmbeddedPythonActorTest, Test) {
    fixture f;
    auto tmp = f.self->spawn<EmbeddedPythonActor>("Test");

    // auto pyactor = py::cast(actor_cast<actor_addr>(tmp));
    // spdlog::warn("{}", to_string(tmp));
    // spdlog::warn("{}", to_string(actor_cast<actor_addr>(tmp)));
    // py::print(pyactor);

    try {
        auto result = request_receive<std::string>(*(f.self), tmp, name_atom_v);
        EXPECT_EQ(result, "Test");
    } catch (const std::exception &) {
    }

    try {
        auto result = request_receive<JsonStore>(*(f.self), tmp, python_eval_atom_v, "1+1");
        EXPECT_EQ(result, 2);
    } catch (const std::exception &) {
    }

    try {
        f.self->mail(python_exec_atom_v, "a=3").send(tmp);
    } catch (const std::exception &) {
    }

    try {
        auto result = request_receive<JsonStore>(*(f.self), tmp, python_eval_atom_v, "a");
        EXPECT_EQ(result, 3);
    } catch (const std::exception &) {
    }

    try {
        auto result = request_receive<JsonStore>(
            *(f.self),
            tmp,
            python_eval_atom_v,
            "a+b",
            JsonStore(nlohmann::json({{"a", 1}, {"b", 2}})));
        EXPECT_EQ(result, 3);
    } catch (const std::exception &) {
    }

    try {
        auto result = request_receive<JsonStore>(
            *(f.self),
            tmp,
            python_eval_locals_atom_v,
            "c=locals()['a']+locals()['b']",
            JsonStore(nlohmann::json({{"a", 1}, {"b", 2}})));
        EXPECT_EQ(result["c"], 3);
    } catch (const std::exception &) {
    }

    f.self->mail(python_exec_atom_v, "print ('hello')", utility::Uuid()).send(tmp);

    try {
        auto result = request_receive<Uuid>(*(f.self), tmp, python_create_session_atom_v, true);
        EXPECT_FALSE(result.is_null());


        try {
            auto existsin = request_receive<JsonStore>(
                *(f.self),
                tmp,
                python_eval_atom_v,
                "'" + to_string(result) + "' in xstudio_sessions",
                utility::Uuid());
            EXPECT_EQ(existsin, true);
        } catch (const std::exception &) {
        }

        auto result2 =
            request_receive<Uuid>(*(f.self), tmp, python_remove_session_atom_v, result);
        EXPECT_TRUE(result2);

        try {
            auto existsin = request_receive<JsonStore>(
                *(f.self),
                tmp,
                python_eval_atom_v,
                "'" + to_string(result) + "' in xstudio_sessions",
                utility::Uuid());
            EXPECT_EQ(existsin, false);
        } catch (const std::exception &) {
        }

    } catch (const std::exception &) {
    }


    f.self->send_exit(tmp, caf::exit_reason::user_shutdown);
}

TEST(EmbeddedPythonActorTest, ConvertsUuidStringArraysInNamedPluginArguments) {
    fixture f;
    auto actor = f.self->spawn<EmbeddedPythonActor>("Test");
    f.self->mail(
        python_exec_atom_v,
        std::string(R"(
class TestConnection:
    def get_named_python_plugin_instance(self, name):
        return self
    def inspect(self, **kwargs):
        return {
            key: {
                "list": type(values) is list,
                "types": [type(value).__name__ for value in values],
                "values": [str(value) if isinstance(value, Uuid) else value for value in values],
            }
            for key, values in kwargs.items()
        }
    def disconnect(self):
        pass
XSTUDIO = TestConnection()
)"),
        Uuid())
        .send(actor);

    const std::string uuid = "12345678-1234-1234-1234-123456789abc";
    const std::string nil  = "00000000-0000-0000-0000-000000000000";
    const std::string embedded_null = uuid + std::string("\0suffix", 7);
    JsonStore args(nlohmann::json{
        {"valid", nlohmann::json::array({uuid, nil})},
        {"invalid", nlohmann::json::array({uuid, "not-a-uuid"})},
        {"mixed", nlohmann::json::array({uuid, 42})},
        {"malformed", nlohmann::json::array({uuid + "a", embedded_null})},
        {"empty", nlohmann::json::array()}});

    const auto result = request_receive<JsonStore>(
        *f.self, actor, python_exec_atom_v, std::string("test"), std::string("inspect"), args);
    EXPECT_EQ(result["valid"]["list"], true);
    EXPECT_EQ(result["valid"]["types"], nlohmann::json::array({"Uuid", "Uuid"}));
    EXPECT_EQ(result["valid"]["values"], nlohmann::json::array({uuid, nil}));
    EXPECT_EQ(result["invalid"]["types"], nlohmann::json::array({"str", "str"}));
    EXPECT_EQ(result["invalid"]["values"], args["invalid"]);
    EXPECT_EQ(result["mixed"]["types"], nlohmann::json::array({"str", "int"}));
    EXPECT_EQ(result["mixed"]["values"], args["mixed"]);
    EXPECT_EQ(result["malformed"]["types"], nlohmann::json::array({"str", "str"}));
    EXPECT_EQ(result["malformed"]["values"], args["malformed"]);
    EXPECT_EQ(result["empty"]["list"], true);
    EXPECT_EQ(result["empty"]["values"], nlohmann::json::array());

    f.self->send_exit(actor, caf::exit_reason::user_shutdown);
}

// TEST(SessionSerializerTest, Test) {
// 	fixture f;

// 	binary_serializer::container_type buf;
// 	binary_serializer bs{f.system, buf};

// 	std::vector<UuidActor> dt1{};
// 	std::vector<UuidActor> dt2{};

// 	auto e = bs.apply(dt1);
// 	EXPECT_FALSE(e) << "unable to serialize" << to_string(e) << std::endl;

// 	binary_deserializer bd{f.system, buf};
// 	e = bd.apply(dt2);
// 	EXPECT_FALSE(e) << "unable to deserialize" << to_string(e) << std::endl;

// 	EXPECT_EQ(dt1, dt2) << "Creation from string should be equal";
// }
