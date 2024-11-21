
#ifndef CONFORM_QML_EXPORT_H
#define CONFORM_QML_EXPORT_H

#ifdef CONFORM_QML_STATIC_DEFINE
#  define CONFORM_QML_EXPORT
#  define CONFORM_QML_NO_EXPORT
#else
#  ifndef CONFORM_QML_EXPORT
#    ifdef conform_qml_EXPORTS
        /* We are building this library */
#      define CONFORM_QML_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define CONFORM_QML_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef CONFORM_QML_NO_EXPORT
#    define CONFORM_QML_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef CONFORM_QML_DEPRECATED
#  define CONFORM_QML_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef CONFORM_QML_DEPRECATED_EXPORT
#  define CONFORM_QML_DEPRECATED_EXPORT CONFORM_QML_EXPORT CONFORM_QML_DEPRECATED
#endif

#ifndef CONFORM_QML_DEPRECATED_NO_EXPORT
#  define CONFORM_QML_DEPRECATED_NO_EXPORT CONFORM_QML_NO_EXPORT CONFORM_QML_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef CONFORM_QML_NO_DEPRECATED
#    define CONFORM_QML_NO_DEPRECATED
#  endif
#endif

#endif /* CONFORM_QML_EXPORT_H */
