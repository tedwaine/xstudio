// SPDX-License-Identifier: Apache-2.0
#include "xstudio/atoms.hpp"
#include "xstudio/global/xstudio_actor_system.hpp"
#include "xstudio/json_store/json_store_actor.hpp"
#include "xstudio/ui/qml/module_data_ui.hpp"
#include "xstudio/plugin_manager/plugin_base.hpp"
#include "xstudio/ui/qml/helper_ui.hpp"
#include "xstudio/ui/qml/studio_ui.hpp"
#include <gtest/gtest.h>

#include <QApplication>
CAF_PUSH_WARNINGS
#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
CAF_POP_WARNINGS


using namespace xstudio;

class TestPlugin : public plugin::StandardPlugin {
public:
    TestPlugin(caf::actor_config &cfg) : plugin::StandardPlugin(cfg, "Test Plugin", utility::JsonStore()) {

        my_float_attr_ = add_float_attribute(
            "My Float Attribute", 
            "My Float Attribute",
            8.0f);
        my_float_attr_->expose_in_ui_attrs_group("Attrs Group");
        my_float_attr_->expose_in_ui_attrs_group("Attrs Group2");
        std::cerr << "TestPlugin constructed\n";

        my_string_attr_ = add_string_attribute(
            "My String Attribute", 
            "My String Attribute",
            "Hello");
        my_string_attr_->expose_in_ui_attrs_group("Attrs Group");

        my_json_attr_ = add_json_attribute(
            "My JSON Attribute", 
            "My JSON Attribute",
            utility::JsonStore(nlohmann::json::parse(R"({"key": "value"})")));
        my_json_attr_->expose_in_ui_attrs_group("Attrs Group");
        std::cerr << "TestPlugin constructed\n";

    }

    xstudio::module::FloatAttribute * my_float_attr_;
    xstudio::module::StringAttribute * my_string_attr_;
    xstudio::module::JsonAttribute * my_json_attr_;

};

static const QByteArray qml_code = R"(
import QtQuick 2.15

import xstudio.qml.models 1.0
import xstudio.qml.helpers 1.0
import xStudio 1.0

Item {
    Component.onCompleted: {
        console.log("Component completed")
    }
    XsModuleData {
        id: moduleData
        modelDataName: "Attrs Group"
    }

    XsModuleData {
        id: moduleData2
        modelDataName: "Attrs Group2"
    }

    XsAttributeValue {
        id: float_val
        attributeTitle: "My Float Attribute"
        model: moduleData
        property bool trigger: false
        onTriggerChanged: {
            if (trigger) {
                value = 81.0
            }
        }
    }

    XsAttributeValue {
        id: string_val
        attributeTitle: "My String Attribute"
        model: moduleData
        property bool trigger: false
        onTriggerChanged: {
            if (trigger) {
                value = "Hello World"
            }
        }
    }

    XsAttributeValue {
        id: json_val
        attributeTitle: "My JSON Attribute"
        model: moduleData
        property bool trigger: false
        onTriggerChanged: {
            if (trigger) {
                value = {"key": 42}
            }
        }
    }

    XsAttributeValue {
        id: float_val2
        attributeTitle: "My Float Attribute"
        model: moduleData2
        property bool trigger: false
        onTriggerChanged: {
            if (trigger) {
                value = 4.0
            }
        }
    }


}
)";

QObject * float_val_obj = nullptr;
QObject * float_val2_obj = nullptr;
QObject * string_val_obj = nullptr;
QObject * json_val_obj = nullptr;
caf::actor test_plugin;

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
        qDebug() << "Notifying receiver:" << receiver << "event:" << event;
        return QGuiApplication::notify(receiver, event);
    }
};

int main(int argc, char *argv[]) { 

        struct sigaction sigIntHandler;
        sigIntHandler.sa_handler = my_handler;
        sigemptyset(&sigIntHandler.sa_mask);
        sigIntHandler.sa_flags = 0;
        sigaction(SIGINT, &sigIntHandler, nullptr);

    ::testing::InitGoogleTest(&argc, argv); 
    CafActorSystem::instance();

    using namespace xstudio;

    utility::start_logger();

    // this call sets up the CAF actor system

    // creates the 'global' actor that manages many of xstudio's core components
    auto global_actor = CafActorSystem::global_actor(false, false, "XStudio");

    caf::scoped_actor self{CafActorSystem::system()};

    // Create a session object
    test_plugin = self->spawn<TestPlugin>();
    self->send(test_plugin, module::connect_to_ui_atom_v);

    MyApp app(argc, argv);

    QQmlApplicationEngine engine;
    //caf::actor viewport_actor = self->spawn<GLXWindowViewportActor>();

    ui::qml::setup_xstudio_qml_emgine(
        static_cast<QQmlEngine *>(&engine), CafActorSystem::system());

    engine.loadData(qml_code);

    for (auto obj : engine.rootObjects()) {
        // dump_children(obj);
        auto js = obj->children();
        for (auto &js_obj : js) {
            if (!(js_obj->metaObject() && std::string(js_obj->metaObject()->className()).find("XsAttributeValue") == 0)) continue;
            QQmlContext* const context = qmlContext(js_obj);
            if (context && context->nameForObject(js_obj) == "float_val") {
                float_val_obj = js_obj;                
            }
            if (context && context->nameForObject(js_obj) == "float_val2") {
                float_val2_obj = js_obj;                
            }
            if (context && context->nameForObject(js_obj) == "string_val") {
                string_val_obj = js_obj;                
            }
            if (context && context->nameForObject(js_obj) == "json_val") {
                json_val_obj = js_obj;                
            }
        }
    }

    auto result = RUN_ALL_TESTS();

    app.exit();
    self->send_exit(test_plugin, caf::exit_reason::user_shutdown);
    test_plugin = caf::actor();

    CafActorSystem::exit();
    utility::stop_logger();

    return result;
}

TEST(UIModelDataTest, QmlItemsExist) { 
    
    ASSERT_NE(float_val_obj, nullptr);
    ASSERT_NE(float_val2_obj, nullptr);
    ASSERT_NE(string_val_obj, nullptr);
    ASSERT_NE(json_val_obj, nullptr);

}


TEST(UIModelDataTest, FloatValRead) { 
    
    ASSERT_NE(float_val_obj, nullptr);
    ASSERT_EQ(float_val_obj->property("value").toFloat(), 8.0f);
    ASSERT_NE(float_val2_obj, nullptr);
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

    usleep(100000); // sleep for 100ms to allow the UI to update
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    ASSERT_EQ(float_val_obj->property("value").toFloat(), -8.0f);
    ASSERT_EQ(float_val2_obj->property("value").toFloat(), -8.0f);
        
}

TEST(UIModelDataTest, ChangeFloatValFromFrontend) { 
    
    caf::scoped_actor self{CafActorSystem::system()};

    float_val_obj->setProperty("trigger", true);
    QCoreApplication::processEvents();
    usleep(100000); // sleep for 100ms to allow the UI to update

    auto val = utility::request_receive<utility::JsonStore>(
        *self,
        test_plugin,
        module::attribute_value_atom_v,
        "My Float Attribute");

    ASSERT_TRUE(val.is_number_float());
    ASSERT_EQ(val.get<float>(), 81.0f);

    float_val2_obj->setProperty("trigger", true);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    usleep(100000); // sleep for 100ms to allow the UI to update

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

    usleep(100000); // sleep for 100ms to allow the UI to update
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    ASSERT_EQ(string_val_obj->property("value").toString(), "World");
        
}

TEST(UIModelDataTest, ChangeStringValFromFrontend) { 
    
    caf::scoped_actor self{CafActorSystem::system()};

    string_val_obj->setProperty("trigger", true);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    usleep(100000); // sleep for 100ms to allow the UI to update

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

    usleep(100000); // sleep for 100ms to allow the UI to update
    QCoreApplication::processEvents();

    ASSERT_EQ(ui::qml::qvariant_to_json(json_val_obj->property("value")), nlohmann::json::parse(R"({"something":"else"})"));
        
}

TEST(UIModelDataTest, ChangeJsonValFromFrontend) { 
    
    caf::scoped_actor self{CafActorSystem::system()};

    json_val_obj->setProperty("trigger", true);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    usleep(100000); // sleep for 100ms to allow the UI to update

    auto val = utility::request_receive<utility::JsonStore>(
        *self,
        test_plugin,
        module::attribute_value_atom_v,
        "My JSON Attribute");

    ASSERT_TRUE(val.is_object());
    ASSERT_EQ(val.get<nlohmann::json>(), nlohmann::json::parse(R"({"key":42})"));

}
