// SPDX-License-Identifier: Apache-2.0
#include "shotgun_preset_model_ui.hpp"
#include "../data_source_shotgun.hpp"
#include "../data_source_shotgun_query_engine.hpp"

#include "xstudio/utility/string_helpers.hpp"
#include "xstudio/ui/qml/helper_ui.hpp"
#include "xstudio/global_store/global_store.hpp"
#include "xstudio/atoms.hpp"

#include <QQmlExtensionPlugin>
#include <QDateTime>
#include <qdebug.h>

using namespace xstudio;
using namespace xstudio::utility;
using namespace xstudio::shotgun_client;
using namespace xstudio::ui::qml;
using namespace std::chrono_literals;
using namespace xstudio::global_store;

QVariant ShotgunPresetModel::data(const QModelIndex &index, int role) const {
    auto result = QVariant();

    try {
        const auto &j = indexToData(index);

        switch (role) {
        case JSONTreeModel::Roles::JSONRole:
            result = QVariantMapFromJson(indexToFullData(index));
            break;

        case JSONTreeModel::Roles::JSONTextRole:
            result = QString::fromStdString(indexToFullData(index).dump(2));
            break;

        case JSONTreeModel::Roles::JSONPathRole:
            result = QString::fromStdString(getIndexPath(index).to_string());
            break;

        case Roles::enabledRole:
            result = j.at("enabled").get<bool>();
            break;

        case Roles::hiddenRole:
            result = j.at("hidden").get<bool>();
            break;

        case Roles::updateRole:
            result = j.at("update").get<bool>();
            break;

        case Roles::userdataRole:
            result = QString::fromStdString(j.at("userdata"));
            break;

        case Roles::termRole:
            result = QString::fromStdString(j.at("term"));
            break;

        case Roles::valueRole:
            result = QString::fromStdString(j.at("value"));
            break;

        case Roles::entityRole:
            result = QString::fromStdString(j.at("entity"));
            break;

        case Roles::livelinkRole:
            try {
                result = j.at("livelink").get<bool>();
            } catch (...) {
            }
            break;

        case Roles::negatedRole:
            try {
                result = j.at("negated").get<bool>();
            } catch (...) {
            }
            break;

        case JSONTreeModel::Roles::idRole:
            result = QString::fromStdString(to_string(j.at("id").get<Uuid>()));
            break;

        case Roles::nameRole:
        case Qt::DisplayRole:
            result = QString::fromStdString(j.at("name"));
            break;

        case Roles::typeRole:
            result = QString::fromStdString(j.at("type"));
            break;

        default:
            break;
        }

    } catch (const std::exception &err) {

        spdlog::warn(
            "{} {} {} {} {}",
            __PRETTY_FUNCTION__,
            err.what(),
            role,
            index.row(),
            index.internalId());
    }

    return result;
}

void ShotgunPresetModel::setModelPathData(const std::string &path, const JsonStore &data) {
    if (path == "")
        setModelData(data);
    else {
    }
}
