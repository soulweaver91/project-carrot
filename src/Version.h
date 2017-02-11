#pragma once

#ifdef CARROT_DEBUG
    #define CP_VERSION QString("Carrot-dev")
#else
    #define CP_VERSION QString("Project Carrot")
#endif
#define CP_VERSION_NUM QString("0.2")
#define LAYERFORMATVERSION 7
#define EVENTSETVERSION 13
