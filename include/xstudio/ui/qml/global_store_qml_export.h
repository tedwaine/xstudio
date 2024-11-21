
#ifndef GLOBAL_STORE_QML_EXPORT_H
#define GLOBAL_STORE_QML_EXPORT_H

#ifdef GLOBAL_STORE_QML_STATIC_DEFINE
#  define GLOBAL_STORE_QML_EXPORT
#  define GLOBAL_STORE_QML_NO_EXPORT
#else
#  ifndef GLOBAL_STORE_QML_EXPORT
#    ifdef global_store_qml_EXPORTS
        /* We are building this library */
#      define GLOBAL_STORE_QML_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define GLOBAL_STORE_QML_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef GLOBAL_STORE_QML_NO_EXPORT
#    define GLOBAL_STORE_QML_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef GLOBAL_STORE_QML_DEPRECATED
#  define GLOBAL_STORE_QML_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef GLOBAL_STORE_QML_DEPRECATED_EXPORT
#  define GLOBAL_STORE_QML_DEPRECATED_EXPORT GLOBAL_STORE_QML_EXPORT GLOBAL_STORE_QML_DEPRECATED
#endif

#ifndef GLOBAL_STORE_QML_DEPRECATED_NO_EXPORT
#  define GLOBAL_STORE_QML_DEPRECATED_NO_EXPORT GLOBAL_STORE_QML_NO_EXPORT GLOBAL_STORE_QML_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef GLOBAL_STORE_QML_NO_DEPRECATED
#    define GLOBAL_STORE_QML_NO_DEPRECATED
#  endif
#endif

#endif /* GLOBAL_STORE_QML_EXPORT_H */
