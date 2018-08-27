#ifndef WIDGETS_THIRDPARTY_GLOBALS_H
#define WIDGETS_THIRDPARTY_GLOBALS_H

#ifdef WIN32

    #ifndef STATIC_WIDGETS_THIRDPARTY
    #ifdef CUTEREPORT_WIDGET_THIRDPARTY_EXPORTS
    #define WIDGET_THIRDPARTY_EXPORTS  __declspec(dllexport)
    #else
    #define WIDGET_THIRDPARTY_EXPORTS  __declspec(dllimport)
    #endif
    #else
    #define WIDGET_THIRDPARTY_EXPORTS
    #endif

#else
    #define WIDGET_THIRDPARTY_EXPORTS
#endif

#endif // WIDGETS_THIRDPARTY_GLOBALS_H


