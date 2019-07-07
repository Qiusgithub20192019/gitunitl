#ifndef DEBUG_H
#define DEBUG_H

#define LEVEL_INFO  1
#define LEVEL_WARN  2
#define LEVEL_DEBUG 3
#define LEVEL_ERROR 4

#define PRT_TITLE "DEBUG"

#define PRT(level,fmt,arg...) do{printf("[%s]",PRT_TITLE);printf(fmt,##arg);}while(0);

#endif