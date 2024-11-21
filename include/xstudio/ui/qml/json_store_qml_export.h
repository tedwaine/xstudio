
#ifndef JSON_STORE_QML_EXPORT_H
#define JSON_STORE_QML_EXPORT_H

#ifdef JSON_STORE_QML_STATIC_DEFINE
#  define JSON_STORE_QML_EXPORT
#  define JSON_STORE_QML_NO_EXPORT
#else
#  ifndef JSON_STORE_QML_EXPORT
#    ifdef json_store_qml_EXPORTS
        /* We are building this library */
#      define JSON_STORE_QML_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define JSON_STORE_QML_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef JSON_STORE_QML_NO_EXPORT
#    define JSON_STORE_QML_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef JSON_STORE_QML_DEPRECATED
#  define JSON_STORE_QML_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef JSON_STORE_QML_DEPRECATED_EXPORT
#  define JSON_STORE_QML_DEPRECATED_EXPORT JSON_STORE_QML_EXPORT JSON_STORE_QML_DEPRECATED
#endif

#ifndef JSON_STORE_QML_DEPRECATED_NO_EXPORT
#  define JSON_STORE_QML_DEPRECATED_NO_EXPORT JSON_STORE_QML_NO_EXPORT JSON_STORE_QML_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef JSON_STORE_QML_NO_DEPRECATED
#    define JSON_STORE_QML_NO_DEPRECATED
#  endif
#endif

#endif /* JSON_STORE_QML_EXPORT_H */
