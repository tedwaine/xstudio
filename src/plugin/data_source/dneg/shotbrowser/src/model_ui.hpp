// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <caf/all.hpp>

CAF_PUSH_WARNINGS
#include <QAbstractItemModel>
#include <QAbstractProxyModel>
CAF_POP_WARNINGS

#include "xstudio/ui/qml/helper_ui.hpp"
#include "xstudio/ui/qml/json_tree_model_ui.hpp"
#include "xstudio/shotgun_client/shotgun_client.hpp"
#include "xstudio/utility/json_store.hpp"

namespace xstudio {
// using namespace shotgun_client;
namespace ui::qml {

    class ShotBrowserListModel : public JSONTreeModel {
        Q_OBJECT

      public:
        const static inline std::vector<std::string> RoleNames = {"nameRole", "typeRole"};

        enum Roles { nameRole = JSONTreeModel::Roles::LASTROLE, typeRole, LASTROLE };

        ShotBrowserListModel(QObject *parent = nullptr) : JSONTreeModel(parent) {
            setRoleNames(RoleNames);
        }

        [[nodiscard]] QVariant
        data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    };

    class ShotBrowserSequenceModel : public ShotBrowserListModel {

      public:
        const static inline std::vector<std::string> RoleNames = {"statusRole"};

        enum Roles { statusRole = ShotBrowserListModel::Roles::LASTROLE };

        ShotBrowserSequenceModel(QObject *parent = nullptr) : ShotBrowserListModel(parent) {
            auto roles = ShotBrowserListModel::RoleNames;
            roles.insert(roles.end(), RoleNames.begin(), RoleNames.end());
            setRoleNames(roles);
        }

        static nlohmann::json flatToTree(const nlohmann::json &src);

        [[nodiscard]] QVariant
        data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

      private:
        static nlohmann::json sortByName(const nlohmann::json &json);
    };

    class ShotBrowserSequenceFilterModel : public QSortFilterProxyModel {
        Q_OBJECT

        Q_PROPERTY(bool showOmit READ showOmit WRITE setShowOmit NOTIFY showOmitChanged)

      public:
        ShotBrowserSequenceFilterModel(QObject *parent = nullptr)
            : QSortFilterProxyModel(parent) {}

        Q_INVOKABLE [[nodiscard]] QVariant
        get(const QModelIndex &item, const QString &role = "display") const;

        Q_INVOKABLE QModelIndex searchRecursive(
            const QVariant &value,
            const QString &role       = "display",
            const QModelIndex &parent = QModelIndex(),
            const int start           = 0,
            const int depth           = -1);

        bool showOmit() const { return show_omit_; }

        void setShowOmit(const bool value);

      signals:
        void showOmitChanged();

      protected:
        [[nodiscard]] bool
        filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;


      private:
        bool show_omit_{false};
    };


    class ShotBrowserFilterModel : public QSortFilterProxyModel {
        Q_OBJECT

        Q_PROPERTY(int length READ length NOTIFY lengthChanged)
        Q_PROPERTY(int count READ length NOTIFY lengthChanged)

        Q_PROPERTY(QItemSelection selectionFilter READ selectionFilter WRITE setSelectionFilter
                       NOTIFY selectionFilterChanged)

      public:
        ShotBrowserFilterModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {
            connect(
                this,
                &QAbstractListModel::rowsInserted,
                this,
                &ShotBrowserFilterModel::lengthChanged);
            connect(
                this,
                &QAbstractListModel::modelReset,
                this,
                &ShotBrowserFilterModel::lengthChanged);
            connect(
                this,
                &QAbstractListModel::rowsRemoved,
                this,
                &ShotBrowserFilterModel::lengthChanged);
        }

        Q_INVOKABLE [[nodiscard]] QVariant
        get(const QModelIndex &item, const QString &role = "display") const;

        [[nodiscard]] int length() const { return rowCount(); }

        Q_INVOKABLE QModelIndex searchRecursive(
            const QVariant &value,
            const QString &role       = "display",
            const QModelIndex &parent = QModelIndex(),
            const int start           = 0,
            const int depth           = -1);


        [[nodiscard]] QItemSelection selectionFilter() const { return selection_filter_; }

        void setSelectionFilter(const QItemSelection &selection) {
            if (selection_filter_ != selection) {
                selection_filter_ = selection;
                emit selectionFilterChanged();
                invalidateFilter();
                // setDynamicSortFilter(false);
                // sort(0, sortOrder());
                // setDynamicSortFilter(true);
            }
        }

      signals:
        void lengthChanged();
        void selectionFilterChanged();

      protected:
        [[nodiscard]] bool
        filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
        // [[nodiscard]] bool
        // lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;

      private:
        QItemSelection selection_filter_;
    };


} // namespace ui::qml
} // namespace xstudio