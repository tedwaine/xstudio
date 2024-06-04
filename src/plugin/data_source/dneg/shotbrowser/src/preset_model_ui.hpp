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
#include <QSortFilterProxyModel>
CAF_POP_WARNINGS

#include "xstudio/ui/qml/helper_ui.hpp"
#include "xstudio/ui/qml/json_tree_model_ui.hpp"
#include "xstudio/shotgun_client/shotgun_client.hpp"
#include "xstudio/utility/json_store.hpp"

#include "query_engine.hpp"

namespace xstudio {
using namespace shotgun_client;

namespace ui::qml {


    class ShotBrowserPresetModel : public JSONTreeModel {
        Q_PROPERTY(QStringList entities READ entities NOTIFY entitiesChanged)
        Q_PROPERTY(QQmlPropertyMap *termLists READ termLists NOTIFY termListsChanged)

        Q_OBJECT
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

        ShotBrowserPresetModel(QueryEngine &query_engine, QObject *parent = nullptr);

        [[nodiscard]] QVariant
        data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        bool setData(
            const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

        void setModelPathData(const std::string &path, const utility::JsonStore &data);

        bool receiveEvent(const utility::JsonStore &event) override;

        Q_INVOKABLE QModelIndex insertGroup(const QString &entity, const int row);
        Q_INVOKABLE QModelIndex insertPreset(const int row, const QModelIndex &parent);
        Q_INVOKABLE QModelIndex
        insertTerm(const QString &term, const int row, const QModelIndex &parent);

        Q_INVOKABLE QModelIndex
        insertOperatorTerm(const bool anding, const int row, const QModelIndex &parent);

        [[nodiscard]] QStringList entities() const;

        QQmlPropertyMap *termLists() const { return term_lists_; }

        Q_INVOKABLE QObject *
        termModel(const QString &term, const QString &entity = "", const int project_id = -1);

        Q_INVOKABLE bool
        removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

        Q_INVOKABLE void resetPresets(const QModelIndexList &indexes);
        Q_INVOKABLE void resetPreset(const QModelIndex &index);
        Q_INVOKABLE QModelIndex duplicate(const QModelIndex &index);
        Q_INVOKABLE QVariant copy(const QModelIndexList &indexes) const;
        Q_INVOKABLE bool paste(const QVariant &data, const int row, const QModelIndex &parent);

        void updateTermModel(const std::string &key, const bool cache);

      signals:
        void entitiesChanged();
        void termListsChanged();
        void presetChanged(QModelIndex);
        void presetHidden(QModelIndex, bool);

      private:
        void markedAsUpdated(const QModelIndex &parent);

        const std::map<std::string, int> role_map_ = {
            {"update", Roles::updateRole},
            {"hidden", Roles::hiddenRole},
            {"enabled", Roles::enabledRole},
            {"term", Roles::termRole},
            {"value", Roles::valueRole},
            {"userdata", Roles::userdataRole},
            {"name", Roles::nameRole},
            {"negated", Roles::negatedRole}};
        QQmlPropertyMap *term_lists_{nullptr};

        QMap<QString, QObject *> term_models_;
        QueryEngine &query_engine_;
    };

    class ShotBrowserPresetFilterModel : public QSortFilterProxyModel {
        Q_OBJECT

        Q_PROPERTY(bool showHidden READ showHidden WRITE setShowHidden NOTIFY showHiddenChanged)
        Q_PROPERTY(QVariant filterUserData READ filterUserData WRITE setFilterUserData NOTIFY
                       filterUserDataChanged)

      public:
        ShotBrowserPresetFilterModel(QObject *parent = nullptr)
            : QSortFilterProxyModel(parent) {
            setDynamicSortFilter(true);
        }

        [[nodiscard]] bool showHidden() const { return show_hidden_; }
        [[nodiscard]] QVariant filterUserData() const { return filter_user_data_; }

        Q_INVOKABLE void setFilter(const QString &filter);
        Q_INVOKABLE void setFilterUserData(const QVariant &filter);
        Q_INVOKABLE void setShowHidden(const bool value);

      signals:
        void showHiddenChanged();
        void filterUserDataChanged();

      protected:
        [[nodiscard]] bool
        filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

      private:
        QVariant filter_;
        bool show_hidden_{false};
        QVariant filter_user_data_;
    };
} // namespace ui::qml
} // namespace xstudio
