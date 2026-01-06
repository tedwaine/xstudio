// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <list>
#include <memory>
#include <string>

#include "xstudio/utility/container.hpp"
#include "xstudio/utility/json_store.hpp"
#include "xstudio/utility/uuid.hpp"
#include "xstudio/utility/frame_range.hpp"
#include "xstudio/timeline/item.hpp"

namespace xstudio {
namespace timeline {
    static const std::set<std::string> TIMELINE_TYPES(
        {"Clip",
         "Track",
         "Video Track",
         "Audio Track",
         "Gap",
         "Stack",
         "TimelineItem",
         "Timeline"});


    class Timeline : public Item {
      public:
        Timeline(
            const std::string &name        = "Timeline",
            const utility::FrameRate &rate = utility::FrameRate(),
            const utility::Uuid &uuid      = utility::Uuid::generate());
        Timeline(const utility::JsonStore &jsn);

        [[nodiscard]] utility::JsonStore serialise() const override;
        [[nodiscard]] Timeline duplicate() const;

        // media list operations.
        // so we can behave like a subset.
        [[nodiscard]] bool media_list_empty() const { return media_list_.empty(); }
        void clear_media_list() { media_list_.clear(); }

        [[nodiscard]] utility::UuidList media() const { return media_list_.uuids(); }
        [[nodiscard]] utility::UuidVector media_vector() const {
            return media_list_.uuid_vector();
        }
        void insert_media(
            const utility::Uuid &uuid, const utility::Uuid &uuid_before = utility::Uuid()) {
            media_list_.insert(uuid, uuid_before);
        }
        bool remove_media(const utility::Uuid &uuid) { return media_list_.remove(uuid); }
        bool move_media(
            const utility::Uuid &uuid, const utility::Uuid &uuid_before = utility::Uuid()) {
            return media_list_.move(uuid, uuid_before);
        }
        bool swap_media(const utility::Uuid &from, const utility::Uuid &to) {
            return media_list_.swap(from, to);
        }
        [[nodiscard]] bool contains_media(const utility::Uuid &uuid) const {
            return media_list_.contains(uuid);
        }

        [[nodiscard]] utility::UuidSet &focus_list() { return focus_list_; }
        [[nodiscard]] utility::UuidSet focus_list() const { return focus_list_; }

        void set_focus_list(const utility::UuidSet &list) { focus_list_ = list; }
        void set_focus_list(const utility::UuidVector &list) {
            focus_list_ = utility::UuidSet(list.begin(), list.end());
        }

        // [[nodiscard]] utility::UuidList tracks() const { return tracks_.uuids(); }
        // void insert_track(
        //     const utility::Uuid &uuid, const utility::Uuid &uuid_before =
        //     utility::Uuid()) { tracks_.insert(uuid, uuid_before);
        // }
        // bool remove_track(const utility::Uuid &uuid) { return tracks_.remove(uuid); }
        // bool
        // move_track(const utility::Uuid &uuid, const utility::Uuid &uuid_before =
        // utility::Uuid())
        // {
        //     return tracks_.move(uuid, uuid_before);
        // }

        // void clear() { tracks_.clear(); }

        // [[nodiscard]] utility::FrameRate rate() const { return start_time_.rate(); }
        // [[nodiscard]] utility::FrameRateDuration start_time() const { return start_time_;
        // } void set_rate(const utility::FrameRate &rate) { start_time_.set_rate(rate,
        // true); }

        // [[nodiscard]] bool contains(const utility::Uuid &uuid) const {
        //     return tracks_.contains(uuid);
        // }

      private:

      utility::UuidListContainer media_list_;
        utility::UuidSet focus_list_;

        // utility::UuidListContainer tracks_;
        // utility::FrameRateDuration start_time_;
    };
} // namespace timeline
} // namespace xstudio