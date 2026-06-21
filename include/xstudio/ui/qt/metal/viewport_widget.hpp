// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "xstudio/ui/opengl/opengl_viewport_renderer.hpp"
#include "xstudio/ui/qml/actor_object.hpp"

CAF_PUSH_WARNINGS
#include <QWidget>
CAF_POP_WARNINGS

namespace xstudio::ui::qt {

class ViewportMetalWidget : public caf::mixin::actor_object<QWidget> {

    Q_OBJECT

  public:
    using super = caf::mixin::actor_object<QWidget>;

    ViewportMetalWidget(
        QWidget *parent,
        const bool live_viewport    = false,
        const QString window_name   = "OffscreenViewport",
        const QString viewport_name = "");

    ~ViewportMetalWidget();

    virtual void init(caf::actor_system &system);

    void set_playhead(caf::actor playhead);

    QString name();

  public slots:
    void frameBufferSwapped();

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    bool event(QEvent *event) override;

    void receive_change_notification(viewport::Viewport::ChangeCallbackId id);

    void sendPointerEvent(EventType t, QMouseEvent *event, int force_modifiers = 0);
    void sendPointerEvent(QHoverEvent *event);

    std::shared_ptr<ui::viewport::Viewport> the_viewport_;
    const bool live_viewport_;
    caf::actor keypress_monitor_;
    std::string window_name_;
    std::string viewport_name_;
};

} // namespace xstudio::ui::qt
