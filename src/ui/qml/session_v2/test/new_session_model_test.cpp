// SPDX-License-Identifier: Apache-2.0
#include "xstudio/atoms.hpp"
#include "xstudio/global/xstudio_actor_system.hpp"
#include "xstudio/json_store/json_store_actor.hpp"
#include "xstudio/ui/qml/module_data_ui.hpp"
#include "xstudio/plugin_manager/plugin_base.hpp"
#include "xstudio/ui/qml/helper_ui.hpp"
#include "xstudio/ui/qml/studio_ui.hpp"
#include <gtest/gtest.h>

#ifdef __apple__
#include <mach-o/dyld.h>
#endif

#include <QApplication>
CAF_PUSH_WARNINGS
#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTimer>
CAF_POP_WARNINGS

using namespace xstudio;

static const QByteArray qml_code = R"(
import QtQuick 2.15

import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0
import xstudio.qml.session 1.0
import xStudio 1.0

Item {
    Component.onCompleted: {
        console.log("Component completed")
    }

    property int playlistCount: -1

    /* This model gives us access to the playlists */
    XsSessionModelNode {
        id: playlistsModelData    
        childSearchPropertyKey: "nameRole"
        childSearchPropertyValue: "PlaylistsRoot"
    }

    DelegateModel {
        id: playlistItemDelegate
        model: playlistsModelData
        onCountChanged: playlistCount = count
        delegate: Item {
            Text {
                text: model.nameRole
            }

            DelegateModel {
                model: playlistsModelData
                rootIndex: playlistsModelData.index(index, 0)
                delegate: Item {
                    Text {
                        text: nameRole
                        onTextChanged: {
                            console.log("Text changed to: " + text)
                        }
                    }
                }
            }
            
        }
    }

}
)";

void my_handler(int s) {
    // force error
    float * badPtr;
    badPtr = (float *)234549;
    delete badPtr;
}

class MyApp : public QGuiApplication {
public:
    MyApp(int &argc, char **argv) : QGuiApplication(argc, argv) {}
    bool notify(QObject *receiver, QEvent *event) override {
        //qDebug() << "Notifying receiver:" << receiver << "event:" << event;
        return QGuiApplication::notify(receiver, event);
    }
};

QQmlApplicationEngine *the_engine = nullptr;
caf::actor global_actor;

int main(int argc, char *argv[]) { 

    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, nullptr);

    std::string xstudio_root;
#if defined(__apple__)
    // Assuming binary is in MacOS folder in bundle
    uint32_t sz = 4096;
    std::array<char, 4096> bin_path;
    auto vv       = _NSGetExecutablePath(bin_path.data(), &sz);
    auto exePath  = fs::path(bin_path.data());
    xstudio_root = std::string(exePath.parent_path().parent_path().parent_path().parent_path().parent_path().parent_path().string()) + "/xSTUDIO.app/Contents";
    setenv("XSTUDIO_ROOT", xstudio_root.c_str(), 1);
#endif

    ::testing::InitGoogleTest(&argc, argv); 
    CafActorSystem::instance();

    using namespace xstudio;

    utility::start_logger();

    // this call sets up the CAF actor system

    // creates the 'global' actor that manages many of xstudio's core components
    global_actor = CafActorSystem::global_actor(false, false, "XStudio");
    std::cerr << "global_actor " << to_string(global_actor) << std::endl;


    MyApp app(argc, argv);

    // On macOS, calling QCoreApplication::processEvents() before
    // QCoreApplication::exec() has ever run triggers Qt's lazy
    // NSApplication bootstrap (ensureNSAppInitialized), which can hang
    // indefinitely. Running (and immediately quitting) a real event loop
    // once up front forces Qt through its normal startup path instead,
    // so the processEvents() calls further down and in the TEST bodies
    // below behave correctly.
    QTimer::singleShot(0, &app, &QCoreApplication::quit);
    app.exec();

    QQmlApplicationEngine engine;
    the_engine = &engine;
    //caf::actor viewport_actor = self->spawn<GLXWindowViewportActor>();

    ui::qml::setup_xstudio_qml_emgine(
        static_cast<QQmlEngine *>(&engine), CafActorSystem::system());

    engine.loadData(qml_code);

    auto result = RUN_ALL_TESTS();

    app.exit();

    global_actor = caf::actor();
    CafActorSystem::exit();
    utility::stop_logger();

    return result;
}


void process_ui_events() {
    usleep(100000); // sleep for 100ms to allow the UI to update
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

TEST(UIModelDataTest, CreatePlaylistAndChildren) { 
    
    try {
        caf::scoped_actor sys{CafActorSystem::system()};

        std::cerr << "global_actor " << to_string(global_actor) << std::endl;
        auto session_actor = utility::request_receive<caf::actor>(*sys, global_actor, session::session_atom_v);
        ASSERT_TRUE(bool(session_actor));

        // create a playlist
        auto playlist_actor = utility::request_receive<std::pair<utility::Uuid, utility::UuidActor>>(*sys, session_actor, session::add_playlist_atom_v, "Playlist");
        process_ui_events();

        for (auto obj : the_engine->rootObjects()) {
            // dump_children(obj);

            QVariant playlistCount = obj->property("playlistCount");
            ASSERT_EQ(playlistCount.toInt(), 1);
        }

        // create a sequence on the playlist
        auto sequence_actor = utility::request_receive<utility::UuidUuidActor>(
            *sys,
            playlist_actor.second,
            playlist::create_session_object_atom_v,
            "Timeline",
            "Timeline 1",
            utility::FrameRate(24.0),
            utility::Uuid(),
            false,
            true
        );
        process_ui_events();

    } catch (const std::exception &e) {
        FAIL() << "Exception caught: " << e.what() << std::endl;
    }

}


/*TEST(UIModelDataTest, FloatValRead) { 

    process_ui_events();
    process_ui_events();
    ASSERT_EQ(float_val_obj->property("value").toFloat(), 8.0f);
    ASSERT_EQ(float_val2_obj->property("value").toFloat(), 8.0f);

}

TEST(UIModelDataTest, ChangeFloatValFromBackend) { 
    
    caf::scoped_actor self{CafActorSystem::system()};

    utility::request_receive<bool>(
        *self,
        test_plugin,
        module::change_attribute_value_atom_v,
        "My Float Attribute",
        int(module::Attribute::Value),
        true,
        utility::JsonStore(nlohmann::json(-8.0f)));

    process_ui_events();

    ASSERT_EQ(float_val_obj->property("value").toFloat(), -8.0f);
    ASSERT_EQ(float_val2_obj->property("value").toFloat(), -8.0f);
        
}

TEST(UIModelDataTest, ChangeFloatValFromFrontend) { 
    
    caf::scoped_actor self{CafActorSystem::system()};

    float_val_obj->setProperty("trigger", true);
    process_ui_events();

    auto val = utility::request_receive<utility::JsonStore>(
        *self,
        test_plugin,
        module::attribute_value_atom_v,
        "My Float Attribute");

    std::cerr << "val " << val.dump() << std::endl;
    ASSERT_TRUE(val.is_number_float());
    ASSERT_EQ(val.get<float>(), 81.0f);

    float_val2_obj->setProperty("trigger", true);
    process_ui_events();

    val = utility::request_receive<utility::JsonStore>(
        *self,
        test_plugin,
        module::attribute_value_atom_v,
        "My Float Attribute");

    ASSERT_TRUE(val.is_number_float());
    ASSERT_EQ(val.get<float>(), 4.0f);
    ASSERT_EQ(float_val2_obj->property("value").toFloat(), 4.0f);

}

TEST(UIModelDataTest, StringValRead) { 
    
    ASSERT_NE(string_val_obj, nullptr);
    ASSERT_EQ(string_val_obj->property("value").toString(), "Hello");

}

TEST(UIModelDataTest, ChangeStringValFromBackend) { 
    
    caf::scoped_actor self{CafActorSystem::system()};

    utility::request_receive<bool>(
        *self,
        test_plugin,
        module::change_attribute_value_atom_v,
        "My String Attribute",
        int(module::Attribute::Value),
        true,
        utility::JsonStore(nlohmann::json("World")));

    process_ui_events();

    ASSERT_EQ(string_val_obj->property("value").toString(), "World");
        
}

TEST(UIModelDataTest, ChangeStringValFromFrontend) { 
    
    caf::scoped_actor self{CafActorSystem::system()};

    string_val_obj->setProperty("trigger", true);
    process_ui_events();

    auto val = utility::request_receive<utility::JsonStore>(
        *self,
        test_plugin,
        module::attribute_value_atom_v,
        "My String Attribute");

    ASSERT_TRUE(val.is_string());
    ASSERT_EQ(val.get<std::string>(), "Hello World");

}

TEST(UIModelDataTest, JsonValRead) { 
    
    ASSERT_NE(json_val_obj, nullptr);
    ASSERT_EQ(ui::qml::qvariant_to_json(json_val_obj->property("value")), nlohmann::json::parse(R"({"key":"value"})"));

}

TEST(UIModelDataTest, ChangeJsonValFromBackend) { 
    
    caf::scoped_actor self{CafActorSystem::system()};

    utility::request_receive<bool>(
        *self,
        test_plugin,
        module::change_attribute_value_atom_v,
        "My JSON Attribute",
        int(module::Attribute::Value),
        true,
        utility::JsonStore(nlohmann::json::parse(R"({"something":"else"})")));

    process_ui_events();

    ASSERT_EQ(ui::qml::qvariant_to_json(json_val_obj->property("value")), nlohmann::json::parse(R"({"something":"else"})"));
        
}

TEST(UIModelDataTest, ChangeJsonValFromFrontend) { 
    
    caf::scoped_actor self{CafActorSystem::system()};

    json_val_obj->setProperty("trigger", true);
    process_ui_events();

    auto val = utility::request_receive<utility::JsonStore>(
        *self,
        test_plugin,
        module::attribute_value_atom_v,
        "My JSON Attribute");

    ASSERT_TRUE(val.is_object());
    ASSERT_EQ(val.get<nlohmann::json>(), nlohmann::json::parse(R"({"key":42})"));

}*/
    