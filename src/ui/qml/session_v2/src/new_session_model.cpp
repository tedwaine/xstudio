// SPDX-License-Identifier: Apache-2.0
#include "xstudio/ui/qml/new_session_model_ui.hpp"

using namespace xstudio::ui::qml;

// SPDX-License-Identifier: Apache-2.0
#include <caf/actor_registry.hpp>

#include "xstudio/ui/qml/helper_ui.hpp"

CAF_PUSH_WARNINGS
#include <QQmlExtensionPlugin>
CAF_POP_WARNINGS

using namespace xstudio;
using namespace xstudio::ui::qml;
using namespace xstudio::utility;

SessionModel_v2::SessionModel_v2(QObject *parent) : super(parent) {

    init(CafSystemObject::get_actor_system());
}

void SessionModel_v2::init(caf::actor_system &_system) {

    super::init(_system);

    // We can defined custom message handlers here so that messages can be
    // received from other components (actors) in xSTUDIO. In this case we will
    // be interfacing with the plugin backend
    set_message_handler([=](caf::actor_companion * /*self*/) -> caf::message_handler {
        return {

            [=](property_atom, std::vector<int> index_in_tree, const std::string &key, const AnyPtr &value) {
            },
            [=](property_atom, const ObjectDataTreePtr &child_data) {
            },
            [=](property_atom, std::vector<int> index_in_tree, const ObjectDataTreePtr &child_data) {
            },
            [=](property_atom, const std::string &key, const AnyPtr &value) {
            },
            [=](caf::message &m) {
                spdlog::warn(
                    "{} : unrecognised message received. Message content: {}",
                    __PRETTY_FUNCTION__,
                    to_string(m));
            }};
    });
}

QHash<int, QByteArray> SessionModel_v2::roleNames() const {

    QHash<int, QByteArray> roles;
    int i = Qt::UserRole;
    for (const auto &p : ObjectDataTree::property_names()) {
        roles[i++] = p.c_str();
    }

    return roles;
}

void SessionModel_v2::set_root(const ObjectDataTreePtr &root) {
    beginResetModel();
    nodes_by_id_.clear();
    ids_by_node_.clear();
    root_ = root;
    if (root_)
        register_subtree(root_.get());
    endResetModel();
}

void SessionModel_v2::register_subtree(const ObjectDataTree *node) {
    const quintptr id = next_id_++;
    nodes_by_id_[id]  = node;
    ids_by_node_[node] = id;
    for (const auto &child : node->children_data_)
        register_subtree(child.second.get());
}

void SessionModel_v2::unregister_subtree(const ObjectDataTree *node) {
    for (const auto &child : node->children_data_)
        unregister_subtree(child.second.get());
    auto it = ids_by_node_.find(node);
    if (it != ids_by_node_.end()) {
        nodes_by_id_.erase(it->second);
        ids_by_node_.erase(it);
    }
}

const ObjectDataTree *SessionModel_v2::node_from_index(const QModelIndex &index) const {
    if (!index.isValid())
        return root_.get();
    auto it = nodes_by_id_.find(index.internalId());
    return it != nodes_by_id_.end() ? it->second : nullptr;
}

quintptr SessionModel_v2::id_of(const ObjectDataTree *node) const {
    auto it = ids_by_node_.find(node);
    return it != ids_by_node_.end() ? it->second : 0;
}

int SessionModel_v2::rowCount(const QModelIndex &parent) const {

    const ObjectDataTree *node = node_from_index(parent);
    return node ? static_cast<int>(node->children_data_.size()) : 0;
}

int SessionModel_v2::columnCount(const QModelIndex &parent) const { return 1; }

bool SessionModel_v2::hasChildren(const QModelIndex &parent) const { return rowCount(parent) != 0; }


QModelIndex SessionModel_v2::index(int row, int column, const QModelIndex &parent) const {

    const ObjectDataTree *node = node_from_index(parent);
    if (!node || column != 0)
        return {};

    auto it = node->children_data_.find(row);
    if (it == node->children_data_.end())
        return {};

    const quintptr id = id_of(it->second.get());
    return id ? createIndex(row, column, id) : QModelIndex();
}


QModelIndex SessionModel_v2::parent(const QModelIndex &child) const {

    // The tree root has no parent_ and is represented by the invalid QModelIndex. A node's
    // row is its key in the parent's children_data_.
    if (!child.isValid())
        return {};

    const ObjectDataTree *node = node_from_index(child);
    if (!node)
        return {};

    const ObjectDataTree *parent_node = node->parent_;
    if (!parent_node || !parent_node->parent_)
        return {};

    const quintptr parent_id = id_of(parent_node);
    if (!parent_id)
        return {};

    for (const auto &[row, sibling] : parent_node->parent_->children_data_) {
        if (sibling.get() == parent_node)
            return createIndex(row, 0, parent_id);
    }
    return {};
}

QVariant SessionModel_v2::data(const QModelIndex &index, int role) const {

    // no data ... YET!
    return QVariant(QString());

}


bool SessionModel_v2::setData(const QModelIndex &index, const QVariant &value, int role) {
    bool result = false;
    return result;
}

bool SessionModel_v2::canFetchMore(const QModelIndex &parent) const {
    auto result          = false;
    return true;
}

