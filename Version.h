#ifndef H_CP_VERSION
#include <QString>

#ifdef CARROT_DEBUG
    #define CP_VERSION QString("Carrot-dev")
#else
    #define CP_VERSION QString("Project Carrot")
#endif
#define CP_VERSION_NUM 0.11
#define LAYERFORMATVERSION 5
#define EVENTSETVERSION 1
#endif
