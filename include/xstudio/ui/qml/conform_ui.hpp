// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <caf/all.hpp>
#include <caf/io/all.hpp>

CAF_PUSH_WARNINGS
#include <QFuture>
#include <QList>
#include <QUuid>
#include <QtConcurrent>
CAF_POP_WARNINGS

#include "xstudio/ui/qml/helper_ui.hpp"
#include "xstudio/ui/qml/json_tree_model_ui.hpp"

namespace xstudio {
namespace ui {
    namespace qml {

        class ConformEngineUI : public caf::mixin::actor_object<JSONTreeModel> {
            Q_OBJECT

          public:
            using super = caf::mixin::actor_object<JSONTreeModel>;
            enum Roles { nameRole = JSONTreeModel::Roles::LASTROLE };

            ConformEngineUI(QObject *parent = nullptr);
            ~ConformEngineUI() override = default;

            virtual void init(caf::actor_system &system);
            caf::actor_system &system() { return self()->home_system(); }

            [[nodiscard]] QVariant
            data(const QModelIndex &index, int role = Qt::DisplayRole) const override;


            Q_INVOKABLE QFuture<QList<QUuid>> conformMediaFuture(
                const QString &task,
                const QModelIndex &playlist,
                const QModelIndex &media,
                const bool remove = false);


          signals:

          private:
            utility::Uuid conform_uuid_;
            caf::actor conform_events_;
        };

    } // namespace qml
} // namespace ui
} // namespace xstudio