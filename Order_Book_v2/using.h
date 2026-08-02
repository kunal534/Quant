#pragma once
#include<cstdint>
#include<vector>
using Price=std::uint32_t;
using Quantity=std::uint32_t;
using OrderId=std::uint64_t;
using orderIds=std::vector<OrderId>;// for multiple orders at a certain price