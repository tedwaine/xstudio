// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <caf/actor_companion.hpp>

CAF_PUSH_WARNINGS

#include <QCursor>
#include <QObject>
#include <QPointF>
#include <QQuickItem>
#include <QVector3D>
#include <QtQml>
#include <QScreen>

CAF_POP_WARNINGS

#include <xstudio/ui/mouse.hpp>

namespace xstudio {
namespace ui {

    namespace qt {
        class OffscreenViewport;
    }

    namespace qml {

        class QMLViewportRenderer;

        class QMLViewport : public QQuickItem {
            Q_OBJECT

            Q_PROPERTY(int mouseButtons READ mouseButtons NOTIFY mouseButtonsChanged)
            Q_PROPERTY(QPoint mouse READ mouse NOTIFY mouseChanged)
            Q_PROPERTY(QRectF imageBoundaryInViewport READ imageBoundaryInViewport NOTIFY
                           imageBoundaryInViewportChanged)
            Q_PROPERTY(QSize imageResolution READ imageResolution NOTIFY imageResolutionChanged)
            Q_PROPERTY(QString name READ name NOTIFY nameChanged)
            Q_PROPERTY(QUuid playheadUuid READ playheadUuid NOTIFY playheadUuidChanged)

          public:
            QMLViewport(QQuickItem *parent = nullptr);
            virtual ~QMLViewport();

            [[nodiscard]] QUuid playheadUuid() const { return playhead_uuid_; }
            [[nodiscard]] QString name() const;
            [[nodiscard]] int mouseButtons() const { return mouse_buttons; }
            [[nodiscard]] QPoint mouse() const { return mouse_position; }

            QMLViewportRenderer *viewportActor() { return renderer_actor; }
            void deleteRendererActor();

            void setPlayheadUuid(const QUuid &uuid) {
                if (uuid != playhead_uuid_) {
                    playhead_uuid_ = uuid;
                    emit playheadUuidChanged();
                }
            }

          protected:
            void hoverEnterEvent(QHoverEvent *event) override;
            void hoverLeaveEvent(QHoverEvent *event) override;
            void mousePressEvent(QMouseEvent *event) override;
            void mouseReleaseEvent(QMouseEvent *event) override;
            void hoverMoveEvent(QHoverEvent *event) override;
            void mouseMoveEvent(QMouseEvent *event) override;
            void wheelEvent(QWheelEvent *event) override;
            void mouseDoubleClickEvent(QMouseEvent *event) override;
            bool event(QEvent *event) override;
            void keyPressEvent(QKeyEvent *event) override;
            void keyReleaseEvent(QKeyEvent *event) override;

          public slots:

            void sync();
            void cleanup();
            void handleScreenChanged(QScreen *screen);

            void hideCursor();
            void showCursor();
            QSize imageResolution();
            QVector2D bboxCornerInViewport(const int min_x, const int min_y);
            QRectF imageBoundaryInViewport();
            void sendResetShortcut();
            void setOverrideCursor(const QString &name, const bool centerOffset);
            void setOverrideCursor(const Qt::CursorShape cname);
            void setPlayhead(const QString actorAddress);
            void reset();
            QString playheadActorAddress();
            void onVisibleChanged();

          private slots:

            void handleWindowChanged(QQuickWindow *win);

          signals:

            void mouseButtonsChanged();
            void mouseChanged();
            void imageBoundaryInViewportChanged();
            void imageResolutionChanged();
            void doSnapshot(QString, QString, int, int, bool);
            void nameChanged();
            void quickViewSource(
                QStringList mediaActors, QString compareMode, int in_pt, int out_pt);
            void quickViewBackendRequest(QStringList mediaActors, QString compareMode);
            void quickViewBackendRequestWithSize(
                QStringList mediaActors, QString compareMode, QPoint position, QSize size);
            void snapshotRequestResult(QString resultMessage);
            void pointerEntered();
            void pointerExited();
            void playheadUuidChanged();

          private:
            void releaseResources() override;
            PointerEvent makePointerEvent(
                Signature::EventType t, QMouseEvent *event, int force_modifiers = 0);
            PointerEvent makePointerEvent(
                Signature::EventType t, int buttons, int x, int y, int w, int h, int modifiers);

            QQuickWindow *m_window = {nullptr};
            QMLViewportRenderer *renderer_actor{nullptr};

            bool connected_{false};
            QCursor cursor_;
            bool cursor_hidden{false};
            int mouse_buttons = {0};
            QPoint mouse_position;
            bool is_quick_viewer_ = {false};
            QUuid playhead_uuid_;

            caf::actor keypress_monitor_;
        };

    } // namespace qml
} // namespace ui
} // namespace xstudio