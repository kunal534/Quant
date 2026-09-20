#pragma once
#include "Trade_Info.h"
#include<vector>
class Trade{
    public:
    // Value need to be called which is stored in Trade_info 
    Trade(const Trade_Info& bidTrade,const Trade_Info& askTrade):bidTrade_{bidTrade}, askTrade_{askTrade}
    {
    }
    // dummy api
    const Trade_Info& GetBidTrade()const {return bidTrade_;}
    const Trade_Info& GetAskTrade()const {return askTrade_;}
    private:
    Trade_Info bidTrade_;
    Trade_Info askTrade_;
};

using Trades=std::vector<Trade>;