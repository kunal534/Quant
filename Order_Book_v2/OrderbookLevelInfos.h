#pragma once

#include "LevelInfo.h"

class OrderbookLevelInfos{
    public:
    OrderbookLevelInfos(const LevelInfo& ask,const LevelInfo& bid):asks_{ask},bids_{bid}
    {}

    const LevelInfos& GetBids() const { return bids_;}
    const LevelInfos& GetAsks() const { return asks_;}
    private:
    LevelInfos asks_;
    LevelInfos bids_;
};