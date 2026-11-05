// SPDX-License-Identifier: Apache-2.0
#include "xstudio/ui/qml/new_session_model_ui.hpp"

using namespace xstudio::ui::qml;

// SPDX-License-Identifier: Apache-2.0
#include <caf/actor_registry.hpp>
#include <cctype>
#include <optional>

#include "xstudio/ui/qml/helper_ui.hpp"
#include "xstudio/thumbnail/thumbnail.hpp"
#include "xstudio/module/attribute.hpp"

CAF_PUSH_WARNINGS
#include <QQmlExtensionPlugin>
CAF_POP_WARNINGS

using namespace xstudio;
using namespace xstudio::ui::qml;
using namespace xstudio::utility;

#define SETUP_ROLES(enumName)\
{\
    QStringList enumStrings(QString(enumName##Strings.data()).split(QRegularExpression("\\W+"), Qt::SkipEmptyParts));\
    assert((enumName##All.size()-1) == enumStrings.size());\
    int i = 0;\
    auto pp = enumName##All.begin();\
    pp++;\
    while (pp != enumName##All.end()) {\
        data_roles_[*pp] = enumStrings[i].toUtf8();\
        data_roles_inverse_[enumStrings[i++]] = (*(pp++));\
    }\
}

namespace {

    // Strictly matches the canonical 8-4-4-4-12 hex form, e.g.
    // '5f56abd3-7557-42b9-b19a-881609257dc1'
    bool is_canonical_uuid_string(const std::string &s) {
        if (s.size() != 36)
            return false;
        for (size_t i = 0; i < s.size(); ++i) {
            if (i == 8 || i == 13 || i == 18 || i == 23) {
                if (s[i] != '-')
                    return false;
            } else if (!std::isxdigit(static_cast<unsigned char>(s[i]))) {
                return false;
            }
        }
        return true;
    }

    std::optional<UuidVector> uuid_vector_from_json(const nlohmann::json &j) {
        if (!j.is_array() || j.empty())
            return std::nullopt;
        UuidVector result;
        result.reserve(j.size());
        for (const auto &el : j) {
            if (!el.is_string())
                return std::nullopt;
            const auto &s = el.get_ref<const std::string &>();
            if (!is_canonical_uuid_string(s))
                return std::nullopt;
            result.emplace_back(s);
        }
        return result;
    }

    AnyPtr anyPtrFromQVariant(const QVariant &var) {

        /*if (var.isNull() || !var.isValid())
            return nullptr;
        else if(var.canConvert<QColor>()) {
            const QColor c = var.value<QColor>();
            return std::make_shared<const std::any>(ColourTriplet(
                c.redF(),
                c.greenF(),
                c.blueF(),
                c.alphaF()
            )); }
        else if (var.canConvert<int>()) {
            const int i = var.value<int>();
            return std::make_shared<const std::any>(i);
        }
        else if (var.canConvert<bool>()) {
            const bool b = var.value<bool>();
            return std::make_shared<const std::any>(b);
        }
        else if (var.canConvert<float>()) {
            const float f = var.value<float>();
            return std::make_shared<const std::any>(f);

        }
        else if (var.canConvert<QUuid>()) {
            const QUuid u = var.value<QUuid>();
            return std::make_shared<const std::any>(UuidFromQUuid(u));
        }
        else if (var.canConvert<QString>()) {
            const QString s = var.value<QString>();
            return std::make_shared<const std::any>(s.toStdString());
        } */

        const nlohmann::json json_value = qvariant_to_json(var);
        if (json_value.is_boolean()) {
            return std::make_shared<const std::any>(json_value.get<bool>());
        } else if (json_value.is_number_integer()) {
            return std::make_shared<const std::any>(json_value.get<int>());
        } else if (json_value.is_number_float()) {
            return std::make_shared<const std::any>(json_value.get<float>());
        } else if (json_value.is_string()) {
            return std::make_shared<const std::any>(json_value.get<std::string>());
        } else if (auto uuids = uuid_vector_from_json(json_value)) {
            return std::make_shared<const std::any>(std::move(*uuids));
        }

         if(var.canConvert<QColor>()) {
            const QColor c = var.value<QColor>();
            return std::make_shared<const std::any>(ColourTriplet(
                c.redF(),
                c.greenF(),
                c.blueF(),
                c.alphaF()
            )); }

        return std::make_shared<const std::any>(json_value);
    }

    QVariant qvariantFromAnyPtr(const AnyPtr &var) {
        if (!var) {
            return QVariant();
        }
        const std::any &a = *var;
        if (a.type() == typeid(caf::actor_addr))
            return actorToQString(CafSystemObject::get_actor_system(), std::any_cast<const caf::actor_addr &>(a));
        if (a.type() == typeid(caf::actor))
            return actorToQString(CafSystemObject::get_actor_system(), std::any_cast<const caf::actor &>(a));
        if (a.type() == typeid(std::string))
            return QString::fromStdString(std::any_cast<const std::string &>(a));
        if (a.type() == typeid(int64_t))
            return std::any_cast<int64_t>(a);
        if (a.type() == typeid(int))
            return std::any_cast<int>(a);
        if (a.type() == typeid(bool))
            return std::any_cast<bool>(a);
        if (a.type() == typeid(float))
            return std::any_cast<float>(a);
        if (a.type() == typeid(Uuid))
            return QUuidFromUuid(std::any_cast<Uuid>(a));
        if (a.type() == typeid(ColourTriplet))
            return QColor(std::max(0, std::min(255, int(round(std::any_cast<const ColourTriplet &>(a).r * 255.0f)))),
                          std::max(0, std::min(255, int(round(std::any_cast<const ColourTriplet &>(a).g * 255.0f)))),
                          std::max(0, std::min(255, int(round(std::any_cast<const ColourTriplet &>(a).b * 255.0f)))),
                          std::max(0, std::min(255, int(round(std::any_cast<const ColourTriplet &>(a).a * 255.0f)))));
        if (a.type() == typeid(JsonStore))
            return json_to_qvariant(std::any_cast<const JsonStore &>(a));        
        if (a.type() == typeid(nlohmann::json))
            return json_to_qvariant(std::any_cast<const nlohmann::json &>(a));
        if (a.type() == typeid(thumbnail::ThumbnailBufferPtr)) {
            const auto &buf = std::any_cast<const thumbnail::ThumbnailBufferPtr &>(a);
            return QImage(
                    (uchar *)&(buf->data()[0]),
                    buf->width(),
                    buf->height(),
                    3 * buf->width(),
                    QImage::Format_RGB888);
        }
        if (a.type() == typeid(std::vector<float>)) {
            const auto &vec = std::any_cast<const std::vector<float> &>(a);
            QVariantList img(vec.size());
            for (size_t i = 0; i < vec.size(); ++i) {
                img[i] = vec[i];
            }
            return img;
        }        
        if (a.type() == typeid(std::vector<int>)) {
            const auto &vec = std::any_cast<const std::vector<int> &>(a);
            QVariantList img(vec.size());
            for (size_t i = 0; i < vec.size(); ++i) {
                img[i] = vec[i];
            }
            return img;
        }
        if (a.type() == typeid(std::vector<std::string>)) {
            const auto &vec = std::any_cast<const std::vector<std::string> &>(a);
            QVariantList img(vec.size());
            for (size_t i = 0; i < vec.size(); ++i) {
                img[i] = QString::fromStdString(vec[i]);
            }
            return img;
        }
        if (a.type() == typeid(utility::UuidVector)) {
            const auto &vec = std::any_cast<const utility::UuidVector &>(a);
            QVariantList img(vec.size());
            for (size_t i = 0; i < vec.size(); ++i) {
                img[i] = QUuidFromUuid(vec[i]);
            }
            return img;
        }
        std::cerr << "Bailing out, unknown type: " << a.type().name() << std::endl;

        return QVariant();
    }
}

UIDataModel::UIDataModel(QObject *parent) : super(parent) {
}

void UIDataModel::init(caf::actor_system &_system) {

    super::init(_system);

    values_ = new XsQmlPropertyMap(this);
    QObject::connect(values_, &XsQmlPropertyMap::updateValueRequest, this, &UIDataModel::handleUpdateValueRequest);

    // We can defined custom message handlers here so that messages can be
    // received from other components (actors) in xSTUDIO. In this case we will
    // be interfacing with the plugin backend
    set_message_handler([=](caf::actor_companion * /*self*/) -> caf::message_handler {
        return message_handler().or_else(core_message_handler());
    });

    for (const auto &p : data_roles_inverse_.keys()) {
        values_->insert(p, QVariant());
    }
    emit valuesChanged();

}

caf::message_handler UIDataModel::core_message_handler() {
    return caf::message_handler {
        [=](broadcast::broadcast_down_atom, const caf::actor_addr & /*downed_actor*/) {
        },
        [=](event_atom, move_children_atom, const std::vector<int> &index_in_tree, const int row, int destination_row) {
        
            QModelIndex parent_index = QModelIndex();
            for (const auto &i: index_in_tree) {
                parent_index = index(i, 0, parent_index);
            }
            ObjectDataTree * node = const_cast<ObjectDataTree *>(UIDataModel::node_from_index(parent_index));
            if (node && node->num_nodes() > row && node->num_nodes() >= destination_row) {

                if (destination_row > row) {
                    if (!beginMoveRows(parent_index, row, row, parent_index, destination_row)) {
                        spdlog::warn("Failed to begin move rows");
                        return;
                    }
                } else {
                    if (!beginMoveRows(parent_index, row, row, parent_index, destination_row)) {
                        spdlog::warn("Failed to begin move rows");
                        return;
                    }
                }

                node->move_node(row, destination_row);
                endMoveRows();

            }

        },
        [=](event_atom, remove_children_atom, std::vector<int> index_in_tree) {

            if (index_in_tree.empty())
                return;
            QModelIndex parent_index = QModelIndex();
            int row = index_in_tree.back();
            index_in_tree.pop_back();
            for (const auto &i: index_in_tree) {
                parent_index = index(i, 0, parent_index);
            }
            ObjectDataTree * node = const_cast<ObjectDataTree *>(UIDataModel::node_from_index(parent_index));
            if (node && node->num_nodes() > row) {
                beginRemoveRows(parent_index, row, row);
                unregister_subtree(node->get_node(row).get());
                node->remove_node(row);
                endRemoveRows();
            }

        },

        [=](event_atom, add_children_atom, std::vector<int> index_in_tree, const int insertion_index) {

            QModelIndex parent_index = QModelIndex();
            for (const auto &i: index_in_tree) {
                parent_index = index(i, 0, parent_index);
            }
            ObjectDataTree * node = const_cast<ObjectDataTree *>(UIDataModel::node_from_index(parent_index));
            if (node) {
                beginInsertRows(parent_index, insertion_index, insertion_index);
                endInsertRows();
                emit lengthChanged();
            }

        },

        [=](event_atom, property_atom, const std::vector<int> &index_in_tree, const int key, const AnyPtr &value) {

            QModelIndex parent_index = QModelIndex();
            for (const auto &i: index_in_tree) {
                parent_index = index(i, 0, parent_index);
            }
            ObjectDataTree * node = const_cast<ObjectDataTree *>(UIDataModel::node_from_index(parent_index));
            if (node && data_roles_.contains(key)) {
                try {
                    if (node->set_data_value_check(key, value)) {                        
                        emit dataChanged(parent_index, parent_index, {key});
                        if (node == root_.get()) {


                            auto p = data_roles_.find(key);
                            if (p != data_roles_.end()) {
                                values_->setPropertyFromBackend(p.value(), qvariantFromAnyPtr(value));
                                std::cerr << "Set property: " << p.value().toStdString() << std::endl;

                                QVariant variant = qvariantFromAnyPtr(value);
                                QVariant dat = values_->value(p.value());
                                qDebug() << "Variant: " << variant << " Dat: " << dat;

                            }
                        }
                    }
                } catch (const std::exception &e) {
                    std::cerr << "Exception in set_data_value_check: " << e.what() << std::endl;
                }
            }
        },
        [=](const ObjectDataTreePtr &child_data) {

            if (self()->current_sender() != source_of_data_)
                return;

            set_root(child_data);
            for (const auto &p : data_roles_inverse_.keys()) {
                values_->setPropertyFromBackend(p.toStdString().c_str(), qvariantFromAnyPtr(child_data->get_data_value(data_roles_inverse_[p])));
            }

        },
        [=](event_atom, add_children_atom, std::vector<int> index_in_tree, const int insertion_index, const ObjectDataTreePtr &child_data) {

            QModelIndex parent_index = QModelIndex();
            for (int i = 0; i < (static_cast<int>(index_in_tree.size()) - 1); ++i) {
                parent_index = index(index_in_tree[i], 0, parent_index);
            }

            // New row lands at index num_rows (the old row count), so begin/endInsertRows
            // must announce that same row, not num_rows+1 (which is out of bounds and
            // leaves views unable to resolve the new index).
            beginInsertRows(parent_index, insertion_index, insertion_index);
            ObjectDataTree * node = const_cast<ObjectDataTree *>(UIDataModel::node_from_index(parent_index));
            if (node) {
                register_subtree(child_data.get());
                node->insert_node(child_data, insertion_index);
            }
            endInsertRows();
            emit lengthChanged();

        },
        [=](model_data::watch_model_data_atom, caf::actor data_source) {
            set_source_of_data(data_source);
        },
        [=](property_atom, const AnyPtr &value, const int key, const uint64_t ui_layer_node_id) {

            // this is a response to a request for missing data

        },
        [=](bool value) {
        }/*,
        [=](caf::message &m) {
            spdlog::warn(
                "{} : unrecognised message received. Message content: {}",
                __PRETTY_FUNCTION__,
                to_string(m));
        }*/};
}

void UIDataModel::handleUpdateValueRequest(const QString &key, const QVariant &value) {

    if (!source_of_data_)
        return;
    auto p = data_roles_inverse_.find(key);
    if (p == data_roles_inverse_.end())
        return;
    qDebug() << "Updating value for key: " << key.toStdString() << " with value: " << value;
    self()->mail(property_atom_v, p.value(), anyPtrFromQVariant(value)).send(source_of_data_);

}

void UIDataModel::set_source_of_data(caf::actor source) {

    // No change?
    if (source_of_data_ == source)
        return;

    // Leave the previous source of data's broadcast group if we have one.
    if (source_of_data_)
        self()->mail(property_atom_v, broadcast::leave_broadcast_atom_v, as_actor()).send(source_of_data_);

    source_of_data_ = std::move(source);
    if (source_of_data_) {

        // JKoin the new source of data's broadcast group.
        self()->mail(property_atom_v, broadcast::join_broadcast_atom_v, as_actor()).send(source_of_data_);

        // Request the full model data of the new source
        self()->mail(property_atom_v).send(source_of_data_);
    }

}

void UIDataModel::set_root(const ObjectDataTreePtr &root) {

    auto nm = root->get_data_value<std::string>(session::nameRole);
    std::cerr << "Setting root with name: " << (nm ? *nm : "<none>") << std::endl;

    beginResetModel();
    nodes_by_id_.clear();
    ids_by_node_.clear();
    root_ = root;
    if (root_)
        register_subtree(root_.get());
    endResetModel();
    emit lengthChanged();
}

void UIDataModel::register_subtree(const ObjectDataTree *node) {
    if (!node) { 
        spdlog::warn("Attempted to register a null subtree");
        return;
    }
    const quintptr id = next_id_++;
    nodes_by_id_[id]  = node;
    ids_by_node_[node] = id;
    for (const auto &child : (*node))
        register_subtree(child.get());
}

void UIDataModel::unregister_subtree(const ObjectDataTree *node) {
    for (const auto &child : (*node))
        unregister_subtree(child.get());
    auto it = ids_by_node_.find(node);
    if (it != ids_by_node_.end()) {
        nodes_by_id_.erase(it->second);
        ids_by_node_.erase(it);
    }
}

const ObjectDataTree *UIDataModel::node_from_index(const QModelIndex &index) const {
    if (!index.isValid())
        return root_.get();
    auto it = nodes_by_id_.find(index.internalId());
    return it != nodes_by_id_.end() ? it->second : nullptr;
}

quintptr UIDataModel::id_of(const ObjectDataTree *node) const {
    auto it = ids_by_node_.find(node);
    return it != ids_by_node_.end() ? it->second : 0;
}

int UIDataModel::rowCount(const QModelIndex &parent) const {

    const ObjectDataTree *node = node_from_index(parent);
    const int rt = node ? static_cast<int>(node->num_nodes()) : 0;
    // qDebug() << "Row count for parent" << parent << "is" << rt;
    return rt;
}

int UIDataModel::columnCount(const QModelIndex &parent) const { return 1; }

bool UIDataModel::hasChildren(const QModelIndex &parent) const { return rowCount(parent) != 0; }


QModelIndex UIDataModel::index(int row, int column, const QModelIndex &parent) const {

    const ObjectDataTree *node = node_from_index(parent);

    //qDebug() << "Creating index for row" << row << "column" << column << "parent" << parent << " node " << node;

    if (!node || column != 0)
        return {};

    if (node->num_nodes() <= row)
        return {};

    const quintptr id = id_of(node->get_node(row).get());
    return id ? createIndex(row, column, id) : QModelIndex();
}

QModelIndex UIDataModel::parent(const QModelIndex &child) const {

    // The tree root has no parent() and is represented by the invalid QModelIndex. A node's
    // row is its key in the parent's children_data_.
    if (!child.isValid())
        return {};

    const ObjectDataTree *node = node_from_index(child);
    if (!node)
        return {};

    const ObjectDataTree *parent_node = node->parent();
    if (!parent_node || !parent_node->parent())
        return {};

    const quintptr parent_id = id_of(parent_node);
    if (!parent_id)
        return {};

    auto p = std::find_if(parent_node->parent()->begin(), parent_node->parent()->end(), [parent_node](const ObjectDataTreePtr &c) {
        return c.get() == parent_node;
    });
    if (p != parent_node->parent()->end())
        return createIndex(std::distance(parent_node->parent()->begin(), p), 0, parent_id);
    return {};
}

QVariant UIDataModel::data(const QModelIndex &index, int role) const {

    // no data ... YET!
    const ObjectDataTree *node = node_from_index(index);
    if (!node || index.column() != 0)
        return {};
    if (!data_roles_.contains(role))
        return {};
    if (!node->has_data_value(role)) {
        auto act = node->get_data_value< caf::actor_addr >(session::actorRole);
        if (act) {
            self()->mail(utility::property_atom_v, role, index.internalId(), as_actor()).send(caf::actor_cast<caf::actor>(*act));
        }
        return {};
    }
    QVariant r = qvariantFromAnyPtr(node->get_data_value(role));
    return r;

}

QVariant UIDataModel::getData(const QModelIndex &index, const QString &role) const {

    const ObjectDataTree *node = node_from_index(index);
    if (!node || index.column() != 0)
        return {};

    const auto p = data_roles_inverse_.find(role);
    if (p == data_roles_inverse_.end())
        return {};
    return qvariantFromAnyPtr(node->get_data_value(p.value()));
}

bool UIDataModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    bool result = false;
    const ObjectDataTree *node = node_from_index(index);
    if (node && node->has_data_value(session::actorRole)) {
        auto actor_addr = node->get_data_value< caf::actor_addr >(session::actorRole);
        caf::actor actor = actor_addr ? caf::actor_cast<caf::actor>(*actor_addr) : caf::actor();
        if (actor) {
            AnyPtr d = anyPtrFromQVariant(value);
            if (d) {
                self()->mail(property_atom_v, role, d).send(actor);
            }
        }
    }
    return false;
}

QModelIndex UIDataModel::searchRecursive(const QVariant &value, const QString &role, const QModelIndex &parent, const int depth) const {

    const auto p = data_roles_inverse_.find(role);
    if (p == data_roles_inverse_.end()) {
        return {};
    }
    const int role_id = p.value();

    const ObjectDataTree *node = node_from_index(parent);
    if (!node)
        return {};

    auto value_ptr = anyPtrFromQVariant(value);

    for (int i = 0; i < node->num_nodes(); ++i) {
        const auto &c = node->get_node(i);
        const auto r = c->get_data_value(role_id);
        try {
            if (r && ObjectDataTree::values_equal(r, value_ptr))
                return index(i, 0, parent);
        } catch (const std::exception &err) {
            qDebug() << "Exception while comparing values:" << err.what();
        }
    }
    int num_rows = rowCount(parent);
    if (depth == 0) {
        qDebug() << "Maximum search depth reached " << value << " for role " << num_rows << " " <<role;
        return {};
    }
    for (int row = 0; row < num_rows; ++row) {
        const auto idx = searchRecursive(value, role, index(row, 0, parent), depth-1);
        if (idx.isValid())
            return idx;
    }
    return {};
}


bool UIDataModel::canFetchMore(const QModelIndex &parent) const {
    auto result          = false;
    return true;
}

SessionModelRoot::SessionModelRoot(QObject *parent)
    : UIDataModel(parent) {

    SETUP_ROLES(session::SessionRoles)

}

caf::message_handler SessionModelRoot::message_handler() {

    return caf::message_handler {
        [=](event_atom, session::session_atom, caf::actor session) {
            bool session_is_root = session_actor_ == root_actor_;
            session_actor_ = session;
            if (!session_is_root) reload();
        },
        [=](event_atom,
            session::session_request_atom,
            const std::string &,
            const JsonStore &) {
            // ignored event from studio actor
        }
    };
}

void SessionModelRoot::connect_to_studio() {

    auto studio_actor = system().registry().template get<caf::actor>(studio_registry);

    caf::scoped_actor sys{system()};

    try {
        // join studio events, so we know when a new session has been created
        auto studio_events =
            request_receive<caf::actor>(*sys, studio_actor, get_event_group_atom_v);
        request_receive<bool>(*sys, studio_events, broadcast::join_broadcast_atom_v, caf::actor_cast<caf::actor>(self()));
    } catch (const std::exception &err) {
        spdlog::error("Failed to get studio events actor: {}", err.what());
    }

    try {
        session_actor_ = request_receive<caf::actor>(*sys, studio_actor, session::session_atom_v);
        root_actor_ = session_actor_;
        QString asString = actorToQString(system(), session_actor_);
        if (asString != root_actor_as_string_) {
            root_actor_as_string_ = asString;
            emit rootActorChanged();
        }
        set_source_of_data(session_actor_);
    } catch (const std::exception &err) {
        spdlog::error("Failed to get session actor: {}", err.what());
    }
}

void SessionModelRoot::setRootActor(const QString &actor) {

    if (root_actor_as_string_ != actor) {
        root_actor_as_string_ = actor;
        emit rootActorChanged();
        reload();
    }
}

void SessionModelRoot::reload() {

    try {

        root_actor_ = actorFromQString(system(), root_actor_as_string_);
        set_source_of_data(root_actor_);

    } catch (const std::exception &err) {
        spdlog::error("Failed to convert actor from QString: {}", err.what());
    }
}


SessionModelNode::SessionModelNode(QObject *parent)
    : SessionModelRoot(parent) {

    QObject::connect(this, &SessionModelRoot::modelReset, this, &SessionModelNode::doReload);
    init(CafSystemObject::get_actor_system());
    connect_to_studio();
}

caf::message_handler SessionModelNode::message_handler() {

    return (caf::message_handler {
        [=](UuidActor &child_we_searched_for) {

            // In 'reload' we search for a child mode matching the search
            // property value at the given key. If the child node is found
            // we set the source of data to the child node's actor.

            // This allows us to point 'rootActor' at a Playlist node and then
            // search for the 'Media List' named node (which is an immediate
            // child of the playlist) which is the node containing all the 
            // Media items in the playlist.

            if (child_we_searched_for) {
                set_source_of_data(child_we_searched_for.actor());
            } else {
                set_source_of_data(root_actor_);
            }

        }
    }).or_else(SessionModelRoot::message_handler());
}


void SessionModelNode::setChildSearchPropertyKey(const QString &key) {
    if (child_search_property_key_ != key) {
        child_search_property_key_ = key;
        emit childSearchPropertyKeyChanged();        
        reload();
    }
}

void SessionModelNode::setChildSearchPropertyValue(const QVariant &value) {
    if (child_search_property_value_ != value) {
        child_search_property_value_ = value;
        emit childSearchPropertyValueChanged();
        reload();
    }
}


void SessionModelNode::reload() {

    if (child_search_property_key_.isEmpty() || child_search_property_value_.isNull() || !root_actor_) {
        SessionModelRoot::reload();
        return;
    }

    try {        
        root_actor_ = actorFromQString(system(), root_actor_as_string_);
    } catch (const std::exception &err) {
        spdlog::error("Failed to convert actor from QString: {}", err.what());
        return;
    }

    try {

        // Perform some action when both key and value are set
        AnyPtr searchValue = anyPtrFromQVariant(child_search_property_value_);

        auto p = data_roles_inverse_.find(child_search_property_key_);
        if (p == data_roles_inverse_.end()) {
            throw std::runtime_error("Unknown role: " + child_search_property_key_.toStdString());
        }

        // we get a UuidActor in our message handler in response to this message
        self()->mail(find_children_atom_v, p.value(), searchValue, 2).send(root_actor_);

    } catch (const std::exception &err) {
        spdlog::error("Failed to perform action with child search properties: {}", err.what());
    }
}

AttributesModelData::AttributesModelData(QObject *parent)
    : SessionModelRoot(parent) 
{
    data_roles_.clear();
    data_roles_inverse_.clear();
    auto pp = module::Attribute::role_names.begin();
    while (pp != module::Attribute::role_names.end()) {
        data_roles_[pp->first] = pp->second.c_str();
        data_roles_inverse_[pp->second.c_str()] = pp->first;
        ++pp;
    }
    init(CafSystemObject::get_actor_system());
}

void AttributesModelData::setModelDataName(const QString &name) {

    if (group_name_ != name) {

        group_name_ = name;
        emit modelDataNameChanged();

        auto central_models_data_actor =
            system().registry().template get<caf::actor>(global_ui_model_data_registry);

        anon_mail(ui::model_data::watch_model_data_atom_v,
            group_name_.toStdString(),
            as_actor()).send(central_models_data_actor);


    }
}