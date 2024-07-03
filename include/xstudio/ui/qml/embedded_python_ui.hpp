// SPDX-License-Identifier: Apache-2.0
#pragma once

// include CMake auto-generated export hpp
#include "xstudio/ui/qml/embedded_python_qml_export.h"

#include <caf/all.hpp>
#include <caf/io/all.hpp>

// CAF_PUSH_WARNINGS
// #include <QFuture>
// #include <QList>
// #include <QUuid>
// #include <QtConcurrent>
// CAF_POP_WARNINGS

#include "xstudio/ui/qml/helper_ui.hpp"
#include "xstudio/ui/qml/json_tree_model_ui.hpp"

namespace xstudio {
namespace ui {
    namespace qml {

        class EMBEDDED_PYTHON_QML_EXPORT EmbeddedPythonUI : public caf::mixin::actor_object<JSONTreeModel> {

            Q_OBJECT
            Q_PROPERTY(bool waiting READ waiting NOTIFY waitingChanged)
            Q_PROPERTY(QUuid sessionId READ sessionId NOTIFY sessionIdChanged)

          public:
            using super = caf::mixin::actor_object<JSONTreeModel>;
            enum Roles {
                nameRole = JSONTreeModel::Roles::LASTROLE,
                menuPathRole,
                scriptPathRole,
                typeRole
            };

            explicit EmbeddedPythonUI(QObject *parent = nullptr);
            ~EmbeddedPythonUI() override = default;

            caf::actor_system &system() const {
                return const_cast<caf::actor_companion *>(self())->home_system();
            }

            void init(caf::actor_system &system);
            void set_backend(caf::actor backend);
            caf::actor backend() { return backend_; }

            [[nodiscard]] bool waiting() const { return waiting_; }
            [[nodiscard]] QUuid sessionId() const { return QUuidFromUuid(event_uuid_); }

            [[nodiscard]] QVariant
            data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

          public slots:
            void pyEvalFile(const QUrl &path);
            void pyExec(const QString &str) const;
            QVariant pyEval(const QString &str);
            QUuid createSession();
            bool sendInput(const QString &str);
            bool sendInterrupt();
            void reloadSnippets() const;
            bool saveSnippet(const QUrl &path, const QString &content) const;

          signals:
            void waitingChanged();
            void backendChanged();
            void sessionIdChanged();
            void stdoutEvent(const QString &str);
            void stderrEvent(const QString &str);

          private:
            caf::actor backend_;
            caf::actor backend_events_;
            bool waiting_{false};

            utility::Uuid event_uuid_;

            utility::Uuid snippet_uuid_{utility::Uuid::generate()};
        };
    } // namespace qml
} // namespace ui
} // namespace xstudio