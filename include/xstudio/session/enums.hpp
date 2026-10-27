// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <map>

namespace xstudio::session {
typedef enum {
    EF_UNDEFINED = 0,
    EF_CSV,
    EF_CSV_WITH_ANNOTATIONS,
    EF_CSV_WITH_IMAGES,
    EF_DIGEST_WITH_ANNOTATIONS,
    EF_LAST
} ExportFormat;

typedef enum {
    activeDurationRole = 4096,
    activeRangeValidRole,
    activeStartRole,
    actorRole,
    actorUuidRole,
    audioActorUuidRole,
    audioModeRole,
    availableDurationRole,
    availableStartRole,
    bitDepthRole,
    bookmarkUuidsRole,
    busyRole,
    clipMediaUuidRole,
    clipEditedStatusRole,
    containerUuidRole,
    enabledRole,
    errorRole,
    expandedRole,
    flagColourRole,
    flagTextRole,
    formatRole,
    groupActorRole,
    imageActorUuidRole,
    lockedRole,
    markersRole,
    mediaCountRole,
    mediaDisplayInfoRole,
    mediaStatusRole,
    metadataChangedRole,
    mtimeRole,
    nameRole,
    notificationRole,
    pathRole,
    pathShakeRole,
    pixelAspectRole,
    placeHolderRole,
    propertyRole,
    qmlCodeRole,
    rateFPSRole,
    rateFPSStringRole,
    resolutionRole,
    rotationRole,
    selectionRole,
    thumbnailImageRole,
    thumbnailURLRole,
    timecodeAsFramesRole,
    trackIndexRole,
    trimmedDurationRole,
    trimmedStartRole,
    typeRole,
    uuidRole,
    userDataRole
} SessionRoles;

}
