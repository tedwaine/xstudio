
#ifndef SESSION_QML_EXPORT_H
#define SESSION_QML_EXPORT_H

#ifdef SESSION_QML_STATIC_DEFINE
#  define SESSION_QML_EXPORT
#  define SESSION_QML_NO_EXPORT
#else
#  ifndef SESSION_QML_EXPORT
#    ifdef session_qml_EXPORTS
        /* We are building this library */
#      define SESSION_QML_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define SESSION_QML_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef SESSION_QML_NO_EXPORT
#    define SESSION_QML_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef SESSION_QML_DEPRECATED
#  define SESSION_QML_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef SESSION_QML_DEPRECATED_EXPORT
#  define SESSION_QML_DEPRECATED_EXPORT SESSION_QML_EXPORT SESSION_QML_DEPRECATED
#endif

#ifndef SESSION_QML_DEPRECATED_NO_EXPORT
#  define SESSION_QML_DEPRECATED_NO_EXPORT SESSION_QML_NO_EXPORT SESSION_QML_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SESSION_QML_NO_DEPRECATED
#    define SESSION_QML_NO_DEPRECATED
#  endif
#endif

#endif /* SESSION_QML_EXPORT_H */
