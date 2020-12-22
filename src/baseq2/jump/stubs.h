#pragma once


int Jump_GetPrevMaps(const char*** maps)
{
    static const char* old_maps[] = {
        "1_prevmap",
        "2_prevmap",
        "3_prevmap",
        "4_prevmap"
    };

    *maps = old_maps;

    return 4;
}
