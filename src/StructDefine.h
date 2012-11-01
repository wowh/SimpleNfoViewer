#ifndef STRUCT_DEFINE_H
#define STRUCT_DEFINE_H

#include <vector>

struct HyperlinkOffset
{
    int start;
    int end;
};
typedef std::vector<HyperlinkOffset> HyperlinkOffsetVec;

#endif