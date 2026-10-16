#pragma once

#include <unordered_map>

#include <caf/all.hpp>
#include <caf/io/all.hpp>

CAF_PUSH_WARNINGS
#include <QFuture>
#include <QList>
#include <QUuid>
#include <QVariantList>
#include <QQmlExtensionPlugin>
#include <QAbstractItemModel>
CAF_POP_WARNINGS

#include "xstudio/ui/qml/helper_ui.hpp"
#include "xstudio/utility/xs_actor.hpp"

namespace xstudio::ui::qml {

/*
SessionModel_v2 class.

We derive from QAbstractItemModelto expose our simple database data in
a tree-like structure that can be used to instance QML elements
efficiently and recursively and is the recommended way to display lists,
tables and trees of data in the xSTUDIO UI (and QML in general).

This class is also a caf::actor / QObject mixin. This means that it
can send and receive CAF messages to interact with other components of
xSTUDIO. The limitation of the Qt 'mixin' approach is that the CAF
messages are received via the Qt event loop. Thus the message handlers
are always executed in the main Qt UI event loop(s) rather than the CAF
actor system threadpool. We need to keep that in mind when processing

There are special QML types that are used to build a UI from a
QtAbstractItemModel. Typically ListView and Repeater and their
derivatives. Refer to the Qt docs for more information as well as the
demo QML code that is part of this project.

*/
class SessionModel_v2 : public caf::mixin::actor_object<QAbstractItemModel> {

    Q_OBJECT

    QML_NAMED_ELEMENT(DemoPluginSessionModelV2)

  public:

    using super = caf::mixin::actor_object<QAbstractItemModel>;

    SessionModel_v2(QObject *parent = nullptr);
    ~SessionModel_v2() override = default;

    void init(caf::actor_system &system);

    [[nodiscard]] caf::actor_system &system() const {
        return const_cast<caf::actor_companion *>(self())->home_system();
    }

    /* These pure virtual methods from QAbstractItemModel need to be overriden*/
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QModelIndex
    index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QModelIndex parent(const QModelIndex &child) const override;
    [[nodiscard]] QVariant
    data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] bool canFetchMore(const QModelIndex &parent) const override;

    // When roleData is set in QML, this method is invoked
    [[nodiscard]] bool
    setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    // Replaces the whole tree. The model takes (shared) ownership of the nodes.
    void set_root(const utility::ObjectDataTreePtr &root);

  signals:

  private:

    // QModelIndex::internalId() is a never-reused id from nodes_by_id_, not a pointer, so a
    // stale index can never resolve to a freed (or address-recycled) node.
    [[nodiscard]] const utility::ObjectDataTree *node_from_index(const QModelIndex &index) const;
    [[nodiscard]] quintptr id_of(const utility::ObjectDataTree *node) const;
    void register_subtree(const utility::ObjectDataTree *node);
    // Call for every subtree removed, inside beginRemoveRows/endRemoveRows.
    void unregister_subtree(const utility::ObjectDataTree *node);

    utility::ObjectDataTreePtr root_;
    std::unordered_map<quintptr, const utility::ObjectDataTree *> nodes_by_id_;
    std::unordered_map<const utility::ObjectDataTree *, quintptr> ids_by_node_;
    quintptr next_id_ = 1;

    caf::actor backend_plugin_;
    bool is_versions_list_ = {false};
};
} // namespace xstudio::ui::qml