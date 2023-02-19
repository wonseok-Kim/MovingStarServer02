#pragma once

#define Crash()                                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        *(int*)0 = 0;                                                                                                  \
    } while (0)