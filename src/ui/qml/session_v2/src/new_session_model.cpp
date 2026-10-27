// SPDX-License-Identifier: Apache-2.0
#include "xstudio/ui/qml/new_session_model_ui.hpp"

using namespace xstudio::ui::qml;

// SPDX-License-Identifier: Apache-2.0
#include <caf/actor_registry.hpp>

#include "xstudio/ui/qml/helper_ui.hpp"
#include "xstudio/thumbnail/thumbnail.hpp"

CAF_PUSH_WARNINGS
#include <QQmlExtensionPlugin>
CAF_POP_WARNINGS

using namespace xstudio;
using namespace xstudio::ui::qml;
using namespace xstudio::utility;

namespace {

    static const std::map<int, QString> sessionRoleNames({
        {session::activeDurationRole, "activeDurationRole"},
        {session::activeRangeValidRole, "activeRangeValidRole"},
        {session::activeStartRole, "activeStartRole"},
        {session::actorRole, "actorRole"},
        {session::actorUuidRole, "actorUuidRole"},
        {session::audioActorUuidRole, "audioActorUuidRole"},
        {session::audioModeRole, "audioModeRole"},
        {session::availableDurationRole, "availableDurationRole"},
        {session::availableStartRole, "availableStartRole"},
        {session::bitDepthRole, "bitDepthRole"},
        {session::bookmarkUuidsRole, "bookmarkUuidsRole"},
        {session::busyRole, "busyRole"},
        {session::clipMediaUuidRole, "clipMediaUuidRole"},
        {session::clipEditedStatusRole, "clipEditedStatusRole"},
        {session::containerUuidRole, "containerUuidRole"},
        {session::enabledRole, "enabledRole"},
        {session::errorRole, "errorRole"},
        {session::expandedRole, "expandedRole"},
        {session::flagColourRole, "flagColourRole"},
        {session::flagTextRole, "flagTextRole"},
        {session::formatRole, "formatRole"},
        {session::groupActorRole, "groupActorRole"},
        {session::imageActorUuidRole, "imageActorUuidRole"},
        {session::lockedRole, "lockedRole"},
        {session::markersRole, "markersRole"},
        {session::mediaCountRole, "mediaCountRole"},
        {session::mediaDisplayInfoRole, "mediaDisplayInfoRole"},
        {session::mediaStatusRole, "mediaStatusRole"},
        {session::metadataChangedRole, "metadataChangedRole"},
        {session::mtimeRole, "mtimeRole"},
        {session::nameRole, "nameRole"},
        {session::notificationRole, "notificationRole"},
        {session::pathRole, "pathRole"},
        {session::pathShakeRole, "pathShakeRole"},
        {session::pixelAspectRole, "pixelAspectRole"},
        {session::placeHolderRole, "placeHolderRole"},
        {session::propertyRole, "propertyRole"},
        {session::qmlCodeRole, "qmlCodeRole"},
        {session::rateFPSRole, "rateFPSRole"},
        {session::rateFPSStringRole, "rateFPSStringRole"},
        {session::resolutionRole, "resolutionRole"},
        {session::rotationRole, "rotationRole"},
        {session::selectionRole, "selectionRole"},
        {session::thumbnailImageRole, "thumbnailImageRole"},
        {session::thumbnailURLRole, "thumbnailURLRole"},
        {session::timecodeAsFramesRole, "timecodeAsFramesRole"},
        {session::trackIndexRole, "trackIndexRole"},
        {session::trimmedDurationRole, "trimmedDurationRole"},
        {session::trimmedStartRole, "trimmedStartRole"},
        {session::typeRole, "typeRole"},
        {session::uuidRole, "uuidRole"},
        {session::userDataRole, "userDataRole"}
    });

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
        if (a.type() == typeid(std::string))
            return QString::fromStdString(std::any_cast<const std::string &>(a));
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
        if (a.type() == typeid(JsonStore) || a.type() == typeid(nlohmann::json))
            return json_to_qvariant(std::any_cast<const JsonStore &>(a));
        if (a.type() == typeid(thumbnail::ThumbnailBufferPtr)) {
            const auto &buf = std::any_cast<const thumbnail::ThumbnailBufferPtr &>(a);
            std::cerr << "Creating QImage from ThumbnailBufferPtr" << std::endl;
            return QImage(
                    (uchar *)&(buf->data()[0]),
                    buf->width(),
                    buf->height(),
                    3 * buf->width(),
                    QImage::Format_RGB888);
        }

        return QVariant();
    }
}

UIDataModel::UIDataModel(QObject *parent) : super(parent) {
}

void UIDataModel::init(caf::actor_system &_system) {

    super::init(_system);

    // We can defined custom message handlers here so that messages can be
    // received from other components (actors) in xSTUDIO. In this case we will
    // be interfacing with the plugin backend
    set_message_handler([=](caf::actor_companion * /*self*/) -> caf::message_handler {
        return message_handler().or_else(core_message_handler());
    });

    int i = Qt::UserRole;
    for (const auto &p : sessionRoleNames) {
        data_roles_[p.first] = p.second.toUtf8();
        data_roles_inverse_[p.second] = p.first;
    }

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
            if (node && node->children_data_.size() > row && node->children_data_.size() >= destination_row) {

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

                auto to_move = node->children_data_[row];
                node->children_data_.erase(node->children_data_.begin() + row);
                if (destination_row > row) {
                    node->children_data_.insert(node->children_data_.begin() + destination_row - 1, to_move);
                } else {
                    node->children_data_.insert(node->children_data_.begin() + destination_row, to_move);
                }
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
            if (node && node->children_data_.size() > row) {
                beginRemoveRows(parent_index, row, row);
                unregister_subtree(node->children_data_[row].get());
                node->children_data_.erase(node->children_data_.begin() + row);
                endRemoveRows();
            }

        },

        [=](property_atom, const std::vector<int> &index_in_tree, const int key, const AnyPtr &value) {

            QModelIndex parent_index = QModelIndex();
            for (const auto &i: index_in_tree) {
                parent_index = index(i, 0, parent_index);
            }
            qDebug() << "Updating property for key: " << key << " index " << parent_index;
            ObjectDataTree * node = const_cast<ObjectDataTree *>(UIDataModel::node_from_index(parent_index));
            if (node && data_roles_.contains(key)) {
                node->properties_[key] = value;
                std::cerr << "YUP\n";
                emit dataChanged(parent_index, parent_index, {key});
            }
        },
        [=](property_atom, const ObjectDataTreePtr &child_data) {
        },
        [=](const ObjectDataTreePtr &child_data) {

            if (self()->current_sender() != source_of_data_)
                return;
            auto oi = child_data->get_property<std::string>(session::nameRole);
            std::cerr << "Name ASFADSFADS role: " << this << " " << (oi ? *oi : "NOP") << std::endl;
            child_data->dump(3,2);
            set_root(child_data);
        },
        [=](property_atom, const std::vector<int> &index_in_tree, const ObjectDataTreePtr &child_data) {

            QModelIndex parent_index = QModelIndex();
            for (int i = 0; i < index_in_tree.size() - 1; ++i) {
                parent_index = index(index_in_tree[i], 0, parent_index);
            }

            int num_rows = rowCount(parent_index);

            // New row lands at index num_rows (the old row count), so begin/endInsertRows
            // must announce that same row, not num_rows+1 (which is out of bounds and
            // leaves views unable to resolve the new index).
            beginInsertRows(parent_index, num_rows, num_rows);
            ObjectDataTree * node = const_cast<ObjectDataTree *>(UIDataModel::node_from_index(parent_index));
            if (node) {
                register_subtree(child_data.get());
                node->children_data_.resize(std::max(node->children_data_.size(), size_t(num_rows + 1)));
                node->children_data_[num_rows] = child_data;
            }
            endInsertRows();

        },
        [=](property_atom, const int key, const AnyPtr &value) {
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
    beginResetModel();
    nodes_by_id_.clear();
    ids_by_node_.clear();
    root_ = root;
    if (root_)
        register_subtree(root_.get());
    endResetModel();
}

void UIDataModel::register_subtree(const ObjectDataTree *node) {
    const quintptr id = next_id_++;
    nodes_by_id_[id]  = node;
    ids_by_node_[node] = id;
    for (const auto &child : node->children_data_)
        register_subtree(child.get());
}

void UIDataModel::unregister_subtree(const ObjectDataTree *node) {
    for (const auto &child : node->children_data_)
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
    const int rt = node ? static_cast<int>(node->children_data_.size()) : 0;
     qDebug() << "Row count for parent" << parent << "is" << rt;
    return rt;
}

int UIDataModel::columnCount(const QModelIndex &parent) const { return 1; }

bool UIDataModel::hasChildren(const QModelIndex &parent) const { return rowCount(parent) != 0; }


QModelIndex UIDataModel::index(int row, int column, const QModelIndex &parent) const {

    const ObjectDataTree *node = node_from_index(parent);

    //qDebug() << "Creating index for row" << row << "column" << column << "parent" << parent << " node " << node;

    if (!node || column != 0)
        return {};

    if (node->children_data_.size() <= row)
        return {};

    const quintptr id = id_of(node->children_data_[row].get());
    return id ? createIndex(row, column, id) : QModelIndex();
}

QModelIndex UIDataModel::parent(const QModelIndex &child) const {

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

    auto p = std::find_if(parent_node->parent_->children_data_.begin(), parent_node->parent_->children_data_.end(), [parent_node](const ObjectDataTreePtr &c) {
        return c.get() == parent_node;
    });
    if (p != parent_node->parent_->children_data_.end())
        return createIndex(std::distance(parent_node->parent_->children_data_.begin(), p), 0, parent_id);
    return {};
}

QVariant UIDataModel::data(const QModelIndex &index, int role) const {

    // no data ... YET!
    const ObjectDataTree *node = node_from_index(index);
    if (!node || index.column() != 0)
        return {};
    if (!data_roles_.contains(role))
        return {};
    std::cerr << role << " ";
    if (node->property_missing(role)) {
        auto act = node->get_property< caf::actor_addr >(session::actorRole);
        if (act) {
            self()->mail(utility::property_atom_v, role, index.internalId(), as_actor()).send(caf::actor_cast<caf::actor>(*act));
        }
        auto pp = sessionRoleNames.find(role);
        qDebug() << "nah " << (pp != sessionRoleNames.end() ? pp->second : "unknown") << " ";
        return {};
    }
    QVariant r = qvariantFromAnyPtr(node->get_property(role));
    qDebug() << r << " ";
    return r;

}

QVariant UIDataModel::getData(const QModelIndex &index, const QString &role) const {

    const ObjectDataTree *node = node_from_index(index);
    if (!node || index.column() != 0)
        return {};

    const auto p = data_roles_inverse_.find(role);
    if (p == data_roles_inverse_.end())
        return {};
    return qvariantFromAnyPtr(node->get_property(p.value()));
}

bool UIDataModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    bool result = false;
    const ObjectDataTree *node = node_from_index(index);
    if (node && node->properties_.contains(session::actorRole)) {
        auto actor_addr = node->get_property< caf::actor_addr >(session::actorRole);
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

bool UIDataModel::canFetchMore(const QModelIndex &parent) const {
    auto result          = false;
    return true;
}

SessionModelRoot::SessionModelRoot(QObject *parent)
    : UIDataModel(parent) {

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

        try {

            root_actor_ = actorFromQString(system(), root_actor_as_string_);
            set_source_of_data(root_actor_);

        } catch (const std::exception &err) {
            spdlog::error("Failed to convert actor from QString: {}", err.what());
        }
    }
}


SessionModelNode::SessionModelNode(QObject *parent)
    : SessionModelRoot(parent) {

    QObject::connect(this, &SessionModelRoot::rootActorChanged, this, &SessionModelNode::doReload);
    QObject::connect(this, &SessionModelRoot::modelReset, this, &SessionModelNode::doReload);
    init(CafSystemObject::get_actor_system());
    connect_to_studio();
}

caf::message_handler SessionModelNode::message_handler() {

    return (caf::message_handler {
        [=](UuidActor &child_we_searched_for) {
            root_actor_ = child_we_searched_for.actor();
            set_source_of_data(root_actor_);
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

    if (child_search_property_key_.isEmpty() || child_search_property_value_.isNull() || !session_actor_) {
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
        self()->mail(find_children_atom_v, p.value(), searchValue, 1).send(session_actor_);

    } catch (const std::exception &err) {
        spdlog::error("Failed to perform action with child search properties: {}", err.what());
    }
}