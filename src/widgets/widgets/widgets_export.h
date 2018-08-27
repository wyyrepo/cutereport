#ifndef WIDGETS_GLOBALS_H
#define WIDGETS_GLOBALS_H

#ifdef WIN32

    #ifndef STATIC_WIDGETS
    #ifdef CUTEREPORT_WIDGET_EXPORTS
    #define WIDGET_EXPORTS  __declspec(dllexport)
    #else
    #define WIDGET_EXPORTS  __declspec(dllimport)
    #endif
    #else
    #define WIDGET_EXPORTS
    #endif

#else
    #define WIDGET_EXPORTS
#endif

#endif // WIDGETS_GLOBALS_H


