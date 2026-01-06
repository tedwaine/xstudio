// SPDX-License-Identifier: Apache-2.0
#include <caf/policy/select_all.hpp>

#include "xstudio/atoms.hpp"
#include "xstudio/timeline/clip_actor.hpp"
#include "xstudio/timeline/stack_actor.hpp"
#include "xstudio/timeline/gap_actor.hpp"
#include "xstudio/timeline/track_actor.hpp"
#include "xstudio/utility/helpers.hpp"
#include "xstudio/utility/logging.hpp"

using namespace xstudio;
using namespace xstudio::utility;
using namespace xstudio::timeline;

caf::actor StackActor::deserialise(const JsonStore &value, const bool replace_item) {

    auto key   = Uuid(value.at("base").contains("item") ? value.at("base").at("item").at("uuid") : value.at("base").at("uuid"));
    auto actor = caf::actor();

    const std::string type = value.at("base").contains("container") ? value.at("base").at("container").at("type") : value.at("base").at("item_type");

    auto item = Item();

    if (type == "Track" || type == "Video Track" || type == "Audio Track") {
        actor = spawn<TrackActor>(static_cast<JsonStore>(value), item);
    } else if (type == "Clip") {
        actor = spawn<ClipActor>(static_cast<JsonStore>(value), item);
    } else if (type == "Gap") {
        actor = spawn<GapActor>(static_cast<JsonStore>(value), item);
    } else if (type == "Stack") {
        actor = spawn<StackActor>(static_cast<JsonStore>(value), item);
    }

    if (actor) {
        add_child_item(UuidActor(key, actor));
        if (replace_item) {
            auto itemit = find_uuid(Stack::children(), key);
            if (itemit != Stack::end()) {
                (*itemit) = item;
            } else {
                spdlog::warn(
                    "{} Invalid item to replace {} {}",
                    __PRETTY_FUNCTION__,
                    to_string(key),
                    value.dump(2));
            }
        }
    }

    return actor;
}

void StackActor::deserialise() {
    for (auto &i : Stack::children()) {
        switch (i.item_type()) {
        case IT_CLIP: {
            auto actor = spawn<ClipActor>(i, i);
            add_child_item(UuidActor(i.uuid(), actor));
        } break;
        case IT_GAP: {
            auto actor = spawn<GapActor>(i, i);
            add_child_item(UuidActor(i.uuid(), actor));
        } break;
        case IT_STACK: {
            auto actor = spawn<StackActor>(i, i);
            add_child_item(UuidActor(i.uuid(), actor));
        } break;
        case IT_AUDIO_TRACK:
        case IT_VIDEO_TRACK: {
            auto actor = spawn<TrackActor>(i, i);
            add_child_item(UuidActor(i.uuid(), actor));
        } break;
        default:
            break;
        }
    }
}


StackActor::StackActor(caf::actor_config &cfg, const JsonStore &jsn)
    : ItemActor2<Stack>(cfg, static_cast<JsonStore>(jsn.at("base"))) {

    Stack::bind_item_post_event_func(
        [this](const JsonStore &event, Item &item) { item_event_callback(event, item); });

    init();
}

StackActor::StackActor(caf::actor_config &cfg, const JsonStore &jsn, Item &pitem)
    : ItemActor2<Stack>(cfg, static_cast<JsonStore>(jsn.at("base"))) {

    Stack::bind_item_post_event_func(
        [this](const JsonStore &event, Item &item) { item_event_callback(event, item); });
    pitem = Stack::clone();

    init();
}

StackActor::StackActor(
    caf::actor_config &cfg, const std::string &name, const FrameRate &rate, const Uuid &uuid)
    : ItemActor2<Stack>(cfg, name, rate, uuid) {

    Stack::set_name(name);
    Stack::bind_item_post_event_func(
        [this](const JsonStore &event, Item &item) { item_event_callback(event, item); });
    init();
}

StackActor::StackActor(caf::actor_config &cfg, const Item &item)
    : ItemActor2<Stack>(cfg, item) {
    init();
}

StackActor::StackActor(caf::actor_config &cfg, const Item &item, Item &nitem)
    : StackActor(cfg, item) {
    nitem = Stack::clone();
}

// trigger actor creation
void StackActor::item_event_callback(const JsonStore &event, Item &item) {

    switch (static_cast<ItemAction>(event.at("action"))) {
    case IA_INSERT: {
        auto cuuid = Uuid(event.at("item").at("uuid"));
        // spdlog::warn("{} {} {} {}", find_uuid(Stack::children(), cuuid) !=
        // Stack::cend(), child_item_actors_.count(cuuid), not event["blind"].is_null(),
        // event.dump(2)); needs to be child..
        auto child_item_it = find_uuid(Stack::children(), cuuid);
        if (child_item_it != Stack::end() and not child_item_actors_.count(cuuid) and
            not event.at("blind").is_null()) {
            // our child
            // spdlog::warn("RECREATE MATCH");

            auto actor = deserialise(JsonStore(event.at("blind")), false);
            // spdlog::warn("{}",to_string(caf::actor_cast<caf::actor_addr>(actor)));
            // spdlog::warn("{}",to_string(caf::actor_cast<caf::actor_addr>(child_item_it->actor())));
            child_item_it->set_actor_addr(actor);
            // change item actor addr
            // spdlog::warn("{}",to_string(caf::actor_cast<caf::actor_addr>(child_item_it->actor())));

            // item actor_addr will be wrong.. in ancestors
            // send special update..
            mail(event_atom_v, item_atom_v, child_item_it->make_actor_addr_update(), true)
                .send(event_group());
        }
    } break;

    case IA_REMOVE: {
        auto cuuid = Uuid(event.at("item_uuid"));
        // child destroyed
        if (child_item_actors_.count(cuuid)) {
            // spdlog::warn("destroy
            // {}",to_string(caf::actor_cast<caf::actor_addr>(child_item_actors_[cuuid])));
            if (auto mit = monitor_.find(caf::actor_cast<caf::actor_addr>(child_item_actors_[cuuid]));
                mit != std::end(monitor_)) {
                mit->second.dispose();
                monitor_.erase(mit);
            }

            send_exit(child_item_actors_[cuuid], caf::exit_reason::user_shutdown);
            child_item_actors_.erase(cuuid);
        }
    } break;

    case IA_LOCK:
    case IA_ENABLE:
    case IA_ACTIVE:
    case IA_RANGE:
    case IA_AVAIL:
    case IA_SPLICE:
    case IA_ADDR:
    case IA_NONE:
    default:
        break;
    }
}

caf::message_handler StackActor::message_handler() {
    return caf::message_handler{
        [=](xstudio::broadcast::broadcast_down_atom, const caf::actor_addr &) {
            // should we handle child down ?
        },

        [=](event_atom, notification_atom, const JsonStore &) {},

        [=](link_media_atom, const UuidActorMap &media, const bool force) -> result<bool> {
            auto rp = make_response_promise<bool>();

            if (child_item_actors_.empty()) {
                rp.deliver(true);
            } else {
                // pool direct children for state.
                fan_out_request<policy::select_all>(
                    map_value_to_vec(child_item_actors_), infinite, link_media_atom_v, media, force)
                    .await(
                        [=](std::vector<bool> items) mutable { rp.deliver(true); },
                        [=](error &err) mutable {
                            spdlog::warn(
                                "{} {} {}",
                                __PRETTY_FUNCTION__,
                                to_string(err),
                                Stack::name());
                            rp.deliver(false);
                        });
            }
            return rp;
        },

        [=](item_marker_atom, insert_item_atom, const Marker &value) -> JsonStore {
            auto markers = Stack::markers();
            markers.push_back(value);
            auto jsn = Stack::set_markers(markers);

            if (not jsn.is_null())
                mail(event_atom_v, item_atom_v, jsn, false).send(event_group());

            return jsn;
        },

        [=](item_marker_atom, insert_item_atom, const std::vector<Marker> &value) -> JsonStore {
            auto markers = Stack::markers();
            markers.insert(markers.end(), value.begin(), value.end());
            auto jsn = Stack::set_markers(markers);

            if (not jsn.is_null())
                mail(event_atom_v, item_atom_v, jsn, false).send(event_group());

            return jsn;
        },

        [=](item_marker_atom, const std::vector<Marker> &markers) -> JsonStore {
            auto jsn = Stack::set_markers(markers);

            if (not jsn.is_null())
                mail(event_atom_v, item_atom_v, jsn, false).send(event_group());

            return jsn;
        },

        [=](item_type_atom) -> ItemType { return Stack::item_type(); },

        [=](rate_atom) -> FrameRate { return Stack::rate(); },

        [=](rate_atom atom, const media::MediaType media_type) {
            return mail(atom).delegate(caf::actor_cast<caf::actor>(this));
        },

        [=](rate_atom,
            const utility::FrameRate &new_rate,
            const bool force_media_rate_to_match) -> bool { return true; },

        [=](item_marker_atom) -> std::vector<Marker> {
            std::vector<Marker> result(
                Stack::markers().begin(), Stack::markers().end());
            return result;
        },

        [=](item_atom) -> Item { return Stack::clone(); },

        [=](item_atom, const bool with_state) -> result<std::pair<JsonStore, Item>> {
            auto rp = make_response_promise<std::pair<JsonStore, Item>>();
            mail(serialise_atom_v)
                .request(caf::actor_cast<caf::actor>(this), infinite)
                .then(
                    [=](const JsonStore &jsn) mutable {
                        rp.deliver(std::make_pair(jsn, Stack::clone()));
                    },
                    [=](const caf::error &err) mutable { rp.deliver(err); });
            return rp;
        },

        [=](item_flag_atom, const std::string &value) -> JsonStore {
            auto jsn = Stack::set_flag(value);
            if (not jsn.is_null())
                mail(event_atom_v, item_atom_v, jsn, false).send(event_group());
            return jsn;
        },

        [=](item_lock_atom, const bool value) -> JsonStore {
            auto jsn = Stack::set_locked(value);
            if (not jsn.is_null())
                mail(event_atom_v, item_atom_v, jsn, false).send(event_group());
            return jsn;
        },

        [=](item_name_atom, const std::string &value) -> JsonStore {
            auto jsn = Stack::set_name(value);
            if (not jsn.is_null())
                mail(event_atom_v, item_atom_v, jsn, false).send(event_group());
            return jsn;
        },

        [=](item_atom, int index) -> result<Item> {
            if (static_cast<size_t>(index) >= Stack::size()) {
                return make_error(xstudio_error::error, "Invalid index");
            }
            auto it = Stack::cbegin();
            std::advance(it, index);
            return (*it).clone();
        },

        [=](item_prop_atom, const JsonStore &value, const bool merge) -> JsonStore {
            auto p = Stack::prop();
            p.update(value);
            auto jsn = Stack::set_prop(p);
            if (not jsn.is_null())
                mail(event_atom_v, item_atom_v, jsn, false).send(event_group());
            return jsn;
        },

        [=](item_prop_atom, const JsonStore &value) -> JsonStore {
            auto jsn = Stack::set_prop(value);
            if (not jsn.is_null())
                mail(event_atom_v, item_atom_v, jsn, false).send(event_group());
            return jsn;
        },

        [=](item_prop_atom, const JsonStore &value, const std::string &path) -> JsonStore {
            auto prop = Stack::prop();
            try {
                auto ptr = nlohmann::json::json_pointer(path);
                prop.at(ptr).update(value);
            } catch (const std::exception &err) {
                spdlog::warn("{} {}", __PRETTY_FUNCTION__, err.what());
            }
            auto jsn = Stack::set_prop(prop);
            if (not jsn.is_null())
                mail(event_atom_v, item_atom_v, jsn, false).send(event_group());
            return jsn;
        },

        [=](item_prop_atom) -> JsonStore { return Stack::prop(); },

        [=](plugin_manager::enable_atom, const bool value) -> JsonStore {
            auto jsn = Stack::set_enabled(value);
            if (not jsn.is_null())
                mail(event_atom_v, item_atom_v, jsn, false).send(event_group());
            return jsn;
        },

        [=](active_range_atom, const FrameRange &fr) -> JsonStore {
            auto jsn = Stack::set_active_range(fr);
            if (not jsn.is_null())
                mail(event_atom_v, item_atom_v, jsn, false).send(event_group());
            return jsn;
        },

        [=](available_range_atom, const FrameRange &fr) -> JsonStore {
            auto jsn = Stack::set_available_range(fr);
            if (not jsn.is_null())
                mail(event_atom_v, item_atom_v, jsn, false).send(event_group());
            return jsn;
        },

        [=](active_range_atom) -> std::optional<FrameRange> {
            return Stack::active_range();
        },

        [=](available_range_atom) -> std::optional<FrameRange> {
            return Stack::available_range();
        },

        [=](trimmed_range_atom) -> FrameRange { return Stack::trimmed_range(); },

        // should these be reflected upward ?
        [=](history::undo_atom, const JsonStore &hist) -> result<bool> {
            Stack::undo(hist);
            if (child_item_actors_.empty())
                return true;
            // push to children..
            auto rp = make_response_promise<bool>();

            fan_out_request<policy::select_all>(
                map_value_to_vec(child_item_actors_), infinite, history::undo_atom_v, hist)
                .then(
                    [=](std::vector<bool> updated) mutable { rp.deliver(true); },
                    [=](error &err) mutable { rp.deliver(std::move(err)); });

            return rp;
        },

        [=](history::redo_atom, const JsonStore &hist) -> result<bool> {
            Stack::redo(hist);
            if (child_item_actors_.empty())
                return true;
            // push to children..
            auto rp = make_response_promise<bool>();

            fan_out_request<policy::select_all>(
                map_value_to_vec(child_item_actors_), infinite, history::redo_atom_v, hist)
                .then(
                    [=](std::vector<bool> updated) mutable { rp.deliver(true); },
                    [=](error &err) mutable { rp.deliver(std::move(err)); });

            return rp;
        },

        // check events processes
        [=](item_atom, event_atom, const std::set<Uuid> &events) -> bool {
            auto result = true;
            for (const auto &i : events) {
                if (not events_processed_.contains(i)) {
                    result = false;
                    break;
                }
            }
            return result;
        },

        // handle child change events.
        [=](event_atom, item_atom, const JsonStore &update, const bool hidden) {
            auto event_ids = Stack::update(update);
            if (not event_ids.empty()) {
                events_processed_.insert(event_ids.begin(), event_ids.end());
                auto more = Stack::refresh();
                if (not more.is_null()) {
                    more.insert(more.begin(), update.begin(), update.end());
                    mail(event_atom_v, item_atom_v, more, hidden).send(event_group());
                    return;
                }
            }

            mail(event_atom_v, item_atom_v, update, hidden).send(event_group());
        },

        [=](insert_item_atom,
            const int index,
            const UuidActorVector &uav) -> result<JsonStore> {
            auto rp = make_response_promise<JsonStore>();
            insert_items(rp, index, uav);
            return rp;
        },

        [=](insert_item_atom,
            const Uuid &before_uuid,
            const UuidActorVector &uav) -> result<JsonStore> {
            auto rp = make_response_promise<JsonStore>();

            auto index = Stack::size();
            // find index. for uuid
            if (not before_uuid.is_null()) {
                auto it = find_uuid(Stack::children(), before_uuid);
                if (it == Stack::end())
                    rp.deliver(make_error(xstudio_error::error, "Invalid uuid"));
                else
                    index = std::distance(Stack::begin(), it);
            }
            if (rp.pending())
                insert_items(rp, index, uav);

            return rp;
        },

        [=](move_item_atom, const int src_index, const int count, const int dst_index)
            -> result<JsonStore> {
            auto rp = make_response_promise<JsonStore>();
            move_items(rp, src_index, count, dst_index);
            return rp;
        },

        [=](move_item_atom, const Uuid &src_uuid, const int count, const Uuid &before_uuid)
            -> result<JsonStore> {
            // check src is valid.
            auto rp   = make_response_promise<JsonStore>();
            auto sitb = find_uuid(Stack::children(), src_uuid);
            if (sitb == Stack::end())
                rp.deliver(make_error(xstudio_error::error, "Invalid src uuid"));


            if (rp.pending()) {
                auto dit = Stack::children().end();
                if (not before_uuid.is_null()) {
                    dit = find_uuid(Stack::children(), before_uuid);
                    if (dit == Stack::end())
                        rp.deliver(make_error(xstudio_error::error, "Invalid dst uuid"));
                }
                if (rp.pending())
                    move_items(
                        rp,
                        std::distance(Stack::begin(), sitb),
                        count,
                        std::distance(Stack::begin(), dit));
            }

            return rp;
        },

        [=](clear_atom) -> result<bool> {
            auto rp = make_response_promise<bool>();
            mail(remove_item_atom_v, 0, (int)Stack::size(), true)
                .request(caf::actor_cast<caf::actor>(this), infinite)
                .then(
                    [=](const std::pair<JsonStore, std::vector<Item>> &) mutable {
                        rp.deliver(true);
                    },
                    [=](caf::error &err) mutable { rp.deliver(err); });
            return rp;
        },

        [=](remove_item_atom,
            const int index,
            const bool) -> result<std::pair<JsonStore, std::vector<Item>>> {
            auto rp = make_response_promise<std::pair<JsonStore, std::vector<Item>>>();
            remove_items(rp, index);
            return rp;
        },

        [=](remove_item_atom, const int index, const int count, const bool)
            -> result<std::pair<JsonStore, std::vector<Item>>> {
            auto rp = make_response_promise<std::pair<JsonStore, std::vector<Item>>>();
            remove_items(rp, index, count);
            return rp;
        },

        [=](remove_item_atom,
            const Uuid &uuid,
            const bool) -> result<std::pair<JsonStore, std::vector<Item>>> {
            auto rp = make_response_promise<std::pair<JsonStore, std::vector<Item>>>();

            auto it = find_uuid(Stack::children(), uuid);

            if (it == Stack::end())
                rp.deliver(make_error(xstudio_error::error, "Invalid uuid"));

            if (rp.pending())
                remove_items(rp, std::distance(Stack::begin(), it));

            return rp;
        },

        [=](erase_item_atom, const int index, const bool) -> result<JsonStore> {
            auto rp = make_response_promise<JsonStore>();
            erase_items(rp, index);
            return rp;
        },

        [=](erase_item_atom, const int index, const int count, const bool)
            -> result<JsonStore> {
            auto rp = make_response_promise<JsonStore>();
            erase_items(rp, index, count);
            return rp;
        },

        [=](erase_item_atom, const Uuid &uuid, const bool) -> result<JsonStore> {
            auto rp = make_response_promise<JsonStore>();

            auto it = find_uuid(Stack::children(), uuid);

            if (it == Stack::end())
                rp.deliver(make_error(xstudio_error::error, "Invalid uuid"));

            if (rp.pending())
                erase_items(rp, std::distance(Stack::begin(), it));

            return rp;
        },

        [=](playhead::source_atom,
            const UuidUuidMap &swap,
            const UuidActorMap &media) -> result<bool> {
            auto rp = make_response_promise<bool>();

            if (child_item_actors_.empty()) {
                rp.deliver(true);
            } else {
                fan_out_request<policy::select_all>(
                    map_value_to_vec(child_item_actors_), infinite, playhead::source_atom_v, swap, media)
                    .await(
                        [=](std::vector<bool> items) mutable { rp.deliver(true); },
                        [=](error &err) mutable {
                            spdlog::warn(
                                "{} {} {}",
                                __PRETTY_FUNCTION__,
                                to_string(err),
                                Stack::name());
                            rp.deliver(err);
                        });
            }

            return rp;
        },

        [=](duplicate_atom) -> result<UuidActor> {

            auto rp = make_response_promise<UuidActor>();
            JsonStore jsn;
            auto dup = Stack::duplicate();
            jsn["base"]   = dup.serialise();
            auto actor    = spawn<StackActor>(jsn);
            rp.deliver(UuidActor(dup.uuid(), actor));
            return rp;

        },

        [=](event_atom, change_atom) {
            mail(event_atom_v, change_atom_v).send(event_group());
        },

        [=](serialise_atom) -> result<JsonStore> {
            if (not child_item_actors_.empty()) {
                auto rp = make_response_promise<JsonStore>();

                fan_out_request<policy::select_all>(
                    map_value_to_vec(child_item_actors_), infinite, serialise_atom_v)
                    .then(
                        [=](std::vector<JsonStore> json) mutable {
                            JsonStore jsn;
                            jsn["base"]   = Stack::serialise();
                            jsn["actors"] = {};
                            for (const auto &j : json) {
                                const utility::Uuid id = j.at("base").contains("container") ? j.at("base").at("container").at("uuid") : j.at("base").at("uuid");
                                jsn["actors"][to_string(id)] = j;
                            }
                            rp.deliver(jsn);
                        },
                        [=](error &err) mutable { rp.deliver(std::move(err)); });

                return rp;
            }
            JsonStore jsn;
            jsn["base"]   = Stack::serialise();
            jsn["actors"] = {};

            return result<JsonStore>(jsn);
        }};
}

void StackActor::init() {
    print_on_create(this, Stack::name());
    print_on_exit(this, Stack::name());
}

void StackActor::insert_items(
    caf::typed_response_promise<JsonStore> rp, const int index, const UuidActorVector &uav) {

    // this is called after setting the rate
    auto do_insertion = [=]() mutable {
        // validate items can be inserted.
        fan_out_request<policy::select_all>(
            vector_to_caf_actor_vector(uav), infinite, item_atom_v)
            .then(
                [=](std::vector<Item> items) mutable {


                    // items are valid for insertion ?
                    for (const auto &i : items) {

                        if (not Stack::valid_child(i))
                            return rp.deliver(
                                make_error(xstudio_error::error, "Invalid child type"));
                    }

                    // take ownership
                    for (const auto &ua : uav)
                        add_child_item(ua);

                    // find insertion point..
                    auto it = std::next(Stack::begin(), index);

                    // insert items..
                    // our list will be out of order..
                    auto changes = JsonStore(R"([])"_json);
                    for (auto uit = uav.rbegin(); uit != uav.rend(); ++uit) {
                        // find item..
                        auto found = false;
                        for (const auto &i : items) {
                            if (uit->uuid() == i.uuid()) {
                                auto tmp = Stack::insert(it, i);
                                it       = std::next(Stack::begin(), index);
                                changes.insert(changes.end(), tmp.begin(), tmp.end());
                                found = true;
                                break;
                            }
                        }

                        if (not found) {
                            spdlog::error("item not found for insertion ff");
                        }
                    }

                    // add changes to stack
                    auto more = Stack::refresh();
                    if (not more.is_null())
                        changes.insert(changes.end(), more.begin(), more.end());

                    mail(event_atom_v, item_atom_v, changes, false).send(event_group());
                    rp.deliver(changes);
                },
                [=](const caf::error &err) mutable { rp.deliver(err); });
    };

    // before adding clips, we must force their frame rate to match ours.
    fan_out_request<policy::select_all>(
        vector_to_caf_actor_vector(uav), infinite, rate_atom_v, Stack::rate(), false)
        .then(
            [=](std::vector<bool> r) mutable { do_insertion(); },
            [=](const caf::error &err) mutable { rp.deliver(err); });
}

void StackActor::remove_items(
    caf::typed_response_promise<std::pair<JsonStore, std::vector<Item>>> rp,
    const int index,
    const int count) {

    try {
        rp.deliver(remove_items(index, count));
    } catch (const std::exception &err) {
        rp.deliver(make_error(xstudio_error::error, err.what()));
    }
}

void StackActor::erase_items(
    caf::typed_response_promise<JsonStore> rp, const int index, const int count) {

    try {
        auto result = remove_items(index, count);
        for (const auto &i : result.second)
            send_exit(i.actor(), caf::exit_reason::user_shutdown);
        rp.deliver(result.first);

    } catch (const std::exception &err) {
        rp.deliver(make_error(xstudio_error::error, err.what()));
    }
}

std::pair<JsonStore, std::vector<Item>>
StackActor::remove_items(const int index, const int count) {
    std::vector<Item> items;
    JsonStore changes(R"([])"_json);

    if (index < 0 or index + count - 1 >= static_cast<int>(Stack::size()))
        throw std::runtime_error("Invalid index / count");
    else {
        scoped_actor sys{system()};

        for (int i = index + count - 1; i >= index; i--) {
            auto it = std::next(Stack::begin(), i);
            if (it != Stack::end()) {
                auto item = *it;

                if (auto mit = monitor_.find(caf::actor_cast<caf::actor_addr>(item.actor()));
                    mit != std::end(monitor_)) {
                    mit->second.dispose();
                    monitor_.erase(mit);
                }

                child_item_actors_.erase(item.uuid());

                auto blind = request_receive<JsonStore>(*sys, item.actor(), serialise_atom_v);

                auto tmp = Stack::erase(it, blind);
                changes.insert(changes.end(), tmp.begin(), tmp.end());
                items.push_back(item.clone());
            }
        }

        auto more = Stack::refresh();
        if (not more.is_null())
            changes.insert(changes.begin(), more.begin(), more.end());

        mail(event_atom_v, item_atom_v, changes, false).send(event_group());
    }

    return std::make_pair(changes, items);
}


void StackActor::move_items(
    caf::typed_response_promise<JsonStore> rp,
    const int src_index,
    const int count,
    const int dst_index) {

    // don't allow mixing audio / video tracks ?

    if (dst_index == src_index or not count)
        rp.deliver(make_error(xstudio_error::error, "Invalid Move"));
    else {
        auto sit = std::next(Stack::begin(), src_index);
        auto eit = std::next(sit, count);
        auto dit = std::next(Stack::begin(), dst_index);

        auto changes = Stack::splice(dit, Stack::children(), sit, eit);
        auto more    = Stack::refresh();
        if (not more.is_null())
            changes.insert(changes.begin(), more.begin(), more.end());

        mail(event_atom_v, item_atom_v, changes, false).send(event_group());
        rp.deliver(changes);
    }
}
