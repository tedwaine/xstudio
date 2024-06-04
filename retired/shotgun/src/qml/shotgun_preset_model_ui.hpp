// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <caf/all.hpp>
#include <caf/io/all.hpp>

CAF_PUSH_WARNINGS
#include <QUrl>
#include <qqml.h>
#include <QQmlEngineExtensionPlugin>
#include <QAbstractListModel>
#include <QAbstractItemModel>
#include <QQmlApplicationEngine>
#include <QFuture>
#include <QtConcurrent>
#include <QAbstractProxyModel>
CAF_POP_WARNINGS

#include "xstudio/ui/qml/helper_ui.hpp"
#include "xstudio/ui/qml/json_tree_model_ui.hpp"
#include "xstudio/shotgun_client/shotgun_client.hpp"
#include "xstudio/utility/json_store.hpp"

namespace xstudio {
using namespace shotgun_client;
namespace ui::qml {
    class ShotgunPresetModel : public JSONTreeModel {
      public:
        enum Roles {
            enabledRole = JSONTreeModel::Roles::LASTROLE,
            entityRole,
            hiddenRole,
            livelinkRole,
            nameRole,
            negatedRole,
            termRole,
            typeRole,
            updateRole,
            userdataRole,
            valueRole
        };

        ShotgunPresetModel(QObject *parent = nullptr) : JSONTreeModel(parent) {
            setRoleNames(std::vector<std::string>(
                {"enabledRole",
                 "entityRole",
                 "hiddenRole",
                 "livelinkRole",
                 "nameRole",
                 "negatedRole",
                 "termRole",
                 "typeRole",
                 "updateRole",
                 "userdataRole",
                 "valueRole"}));
        }

        [[nodiscard]] QVariant
        data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        void setModelPathData(const std::string &path, const utility::JsonStore &data);
    };
} // namespace ui::qml
} // namespace xstudio
