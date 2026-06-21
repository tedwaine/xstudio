// SPDX-License-Identifier: Apache-2.0
#include "xstudio/media_reader/media_reader.hpp"
#include "xstudio/ui/qt/metal/viewport_widget.hpp"
#include "xstudio/ui/qml/helper_ui.hpp"
#include <caf/actor_registry.hpp>

CAF_PUSH_WARNINGS
#include <QMouseEvent>
#include <QWheelEvent>
CAF_POP_WARNINGS

using namespace xstudio::ui::qt;
using namespace xstudio::ui::qml;
using namespace xstudio::ui;

namespace {

int qtModifierToOurs(const Qt::KeyboardModifiers qt_modifiers) {
    int result = Signature::Modifier::NoModifier;

    if (qt_modifiers & Qt::ShiftModifier)
        result |= Signature::Modifier::ShiftModifier;
    if (qt_modifiers & Qt::ControlModifier)
        result |= Signature::Modifier::ControlModifier;
    if (qt_modifiers & Qt::AltModifier)
        result |= Signature::Modifier::AltModifier;
    if (qt_modifiers & Qt::MetaModifier)
        result |= Signature::Modifier::MetaModifier;
    if (qt_modifiers & Qt::KeypadModifier)
        result |= Signature::Modifier::KeypadModifier;
    if (qt_modifiers & Qt::GroupSwitchModifier)
        result |= Signature::Modifier::GroupSwitchModifier;

    return result;
}

} // namespace

ViewportMetalWidget::ViewportMetalWidget(
    QWidget *parent,
    const bool live_viewport,
    const QString window_name,
    const QString viewport_name)
    : super(parent),
      live_viewport_(live_viewport),
      window_name_(StdFromQString(window_name)),
      viewport_name_(StdFromQString(viewport_name)) {

    if (live_viewport) {
        /*QObject::connect(
            this, &QOpenGLWidget::frameSwapped, this, &ViewportMetalWidget::frameBufferSwapped);*/
        setFocusPolicy(Qt::StrongFocus);
    }
}

ViewportMetalWidget::~ViewportMetalWidget() { keypress_monitor_ = caf::actor(); }

void ViewportMetalWidget::set_playhead(caf::actor playhead) {
    if (the_viewport_)
        the_viewport_->set_playhead(playhead);
}

/*void ViewportMetalWidget::resizeGL(int w, int h) {

    anon_mail(
        ui::viewport::viewport_set_scene_coordinates_atom_v,
        0.0f,
        0.0f,
        float(w),
        float(h),
        float(w),
        float(h),
        1.0f)
        .send(self());
}*/


void ViewportMetalWidget::frameBufferSwapped() {
    if (live_viewport_) {
        // this call is crucial for stable playback - we tell the playback engine
        // when the last image was put on the screen so it can infer the video
        // refresh beat and work out which image should be put on screen at the
        // next redraw
        the_viewport_->framebuffer_swapped(utility::clock::now());
    }
}


void ViewportMetalWidget::receive_change_notification(viewport::Viewport::ChangeCallbackId) {
    update();
}

void ViewportMetalWidget::init(caf::actor_system &system) {

    super::init(system);

    utility::JsonStore jsn;
    jsn["base"]      = utility::JsonStore();
    jsn["window_id"] = window_name_;

    the_viewport_.reset(new ui::viewport::Viewport(jsn, as_actor(), true));

    auto callback = [this](auto &&PH1) {
        receive_change_notification(std::forward<decltype(PH1)>(PH1));
    };

    the_viewport_->set_change_callback(callback);

    set_message_handler([=](caf::actor_companion * /*self*/) -> caf::message_handler {
        return the_viewport_->message_handler();
    });

    keypress_monitor_ = system.registry().template get<caf::actor>(keyboard_events);
}

QString ViewportMetalWidget::name() { return QString::fromUtf8(the_viewport_->name().c_str()); }

void ViewportMetalWidget::mousePressEvent(QMouseEvent *event) {

    sendPointerEvent(EventType::ButtonDown, event);
}

void ViewportMetalWidget::mouseReleaseEvent(QMouseEvent *event) {

    sendPointerEvent(EventType::ButtonRelease, event);
}

void ViewportMetalWidget::mouseMoveEvent(QMouseEvent *event) {

    sendPointerEvent(event->buttons() ? EventType::Drag : EventType::Move, event, 0);
}

void ViewportMetalWidget::mouseDoubleClickEvent(QMouseEvent *event) {

    sendPointerEvent(EventType::DoubleClick, event);
}

void ViewportMetalWidget::keyPressEvent(QKeyEvent *key_event) {

    anon_mail(
        ui::keypress_monitor::text_entry_atom_v,
        StdFromQString(key_event->text()),
        the_viewport_->name(),
        window_name_)
        .send(keypress_monitor_);
}

void ViewportMetalWidget::keyReleaseEvent(QKeyEvent *key_event) {

    if (!key_event->isAutoRepeat()) {
        anon_mail(
            ui::keypress_monitor::key_up_atom_v,
            key_event->key(),
            the_viewport_->name(),
            window_name_)
            .send(keypress_monitor_);
    }
}

bool ViewportMetalWidget::event(QEvent *event) {

    if (event->type() == QEvent::KeyPress) {

        auto key_event = dynamic_cast<QKeyEvent *>(event);
        if (key_event) {
            anon_mail(
                ui::keypress_monitor::key_down_atom_v,
                key_event->key(),
                StdFromQString(key_event->text()),
                the_viewport_->name(),
                window_name_,
                key_event->isAutoRepeat())
                .send(keypress_monitor_);
        }
    } else if (event->type() == QEvent::KeyRelease) {

        auto key_event = dynamic_cast<QKeyEvent *>(event);
        if (key_event && !key_event->isAutoRepeat()) {
            anon_mail(
                ui::keypress_monitor::key_up_atom_v,
                key_event->key(),
                the_viewport_->name(),
                window_name_)
                .send(keypress_monitor_);
        }
    } else if (
        event->type() == QEvent::Leave || event->type() == QEvent::HoverLeave ||
        event->type() == QEvent::DragLeave || event->type() == QEvent::GraphicsSceneDragLeave ||
        event->type() == QEvent::GraphicsSceneHoverLeave) {
        // It's possible to receive a KeyPress but not key release if the mouse
        // leaves the window/widget before the user releases the key. This is
        // a headache for us because we need to track key pressed state, not
        // only key down. We have to assume all keys are released (even if they
        // aren't) when we the mouse leaves the focus as we won't get the
        // mouse release event. It doesn't matter if we have false negatives
        // (in terms of whether a key is held down) but a false positive
        // (i.e. we think a key is down when it isn't) could really mess up the
        // UI behaviour
        anon_mail(ui::keypress_monitor::all_keys_up_atom_v, the_viewport_->name())
            .send(keypress_monitor_);
    }

    return super::event(event);
}

void ViewportMetalWidget::wheelEvent(QWheelEvent *event) {

    // make a mouse wheel event and pass to viewport to process
    PointerEvent ev(
        EventType::MouseWheel,
        static_cast<Signature::Button>((int)event->buttons()),
        event->position().x(),
        event->position().y(),
        width(),  // FIXME should be width, but this function appears to never be called.
        height(), // FIXME should be height
        qtModifierToOurs(event->modifiers()),
        the_viewport_->name(),
        std::make_pair(event->angleDelta().rx(), event->angleDelta().ry()),
        PointerEvent::WheelDeltaType::Angle);

    if (!the_viewport_->process_pointer_event(ev) && the_viewport_->playhead()) {
        // If viewport hasn't acted on the mouse wheel event (because user pref
        // to zoom with mouse wheel is false), assume that we can instead use it
        // to step the playhead
        anon_mail(playhead::play_atom_v, false).send(the_viewport_->playhead());
        anon_mail(playhead::step_atom_v, event->angleDelta().ry() > 0 ? 1 : -1)
            .send(the_viewport_->playhead());
    }

    QWidget::wheelEvent(event);
}

void ViewportMetalWidget::sendPointerEvent(EventType t, QMouseEvent *event, int force_modifiers) {

    PointerEvent p(
        t,
        static_cast<Signature::Button>((int)event->buttons()),
        event->position().x(),
        event->position().y(),
        width(),
        height(),
        qtModifierToOurs(event->modifiers()) + force_modifiers,
        the_viewport_->name());

    // p.w_ = utility::clock::now();

    if (the_viewport_->process_pointer_event(p)) {
        update();
    } else {
        anon_mail(ui::keypress_monitor::mouse_event_atom_v, p).send(keypress_monitor_);
    }
}

void ViewportMetalWidget::sendPointerEvent(QHoverEvent *event) {

    PointerEvent p(
        EventType::Move,
        static_cast<Signature::Button>(0),
        event->position().x(),
        event->position().y(),
        width(),
        height(),
        Signature::Modifier::NoModifier,
        the_viewport_->name());

    // p.w_ = utility::clock::now();

    if (the_viewport_->process_pointer_event(p)) {
        update();
    } else {
        anon_mail(ui::keypress_monitor::mouse_event_atom_v, p).send(keypress_monitor_);
    }
}
