
#ifndef BOOKMARK_QML_EXPORT_H
#define BOOKMARK_QML_EXPORT_H

#ifdef BOOKMARK_QML_STATIC_DEFINE
#  define BOOKMARK_QML_EXPORT
#  define BOOKMARK_QML_NO_EXPORT
#else
#  ifndef BOOKMARK_QML_EXPORT
#    ifdef bookmark_qml_EXPORTS
        /* We are building this library */
#      define BOOKMARK_QML_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define BOOKMARK_QML_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef BOOKMARK_QML_NO_EXPORT
#    define BOOKMARK_QML_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef BOOKMARK_QML_DEPRECATED
#  define BOOKMARK_QML_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef BOOKMARK_QML_DEPRECATED_EXPORT
#  define BOOKMARK_QML_DEPRECATED_EXPORT BOOKMARK_QML_EXPORT BOOKMARK_QML_DEPRECATED
#endif

#ifndef BOOKMARK_QML_DEPRECATED_NO_EXPORT
#  define BOOKMARK_QML_DEPRECATED_NO_EXPORT BOOKMARK_QML_NO_EXPORT BOOKMARK_QML_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef BOOKMARK_QML_NO_DEPRECATED
#    define BOOKMARK_QML_NO_DEPRECATED
#  endif
#endif

#endif /* BOOKMARK_QML_EXPORT_H */
