#include <orderbook.hpp>

Status OrderBook::placeMarket(qty quantity, side orderType) {
    if (quantity <= 0) {return Status::INVALID_QTY;}

    id oid = getNewID();
    if ((id)orderIDs.size() <= oid) orderIDs.resize(oid + 1);
    orderIDs[oid] = Pointer {orderType, 0, level::iterator(), false};

    auto& opp = orderType ? sell : buy; 
    if (opp.empty()) return Status::BOOK_EMPTY;

    timestamp t = getTime();

    while (quantity > 0 && !opp.empty()) { //O(n)

        auto lvlIt = orderType? opp.begin() : prev(opp.end());
        level& top = lvlIt->second;

        Order& restingOrder = top.front();
        qty traded = min(quantity, restingOrder.quantity);
        
        quantity -= traded;
        restingOrder.quantity -= traded;

        trades.push_back( Trade {
            .buyerID = orderType? oid : restingOrder.orderID,
            .sellerID = orderType? restingOrder.orderID : oid,
            .price = restingOrder.price,
            .quantity = traded,
            .ts = t
        });

        if (restingOrder.quantity == 0) {
            orderIDs[restingOrder.orderID].active = false;
            top.pop_front();
            if (top.empty()) opp.erase(lvlIt);
        }
    }

    if (quantity > 0) return Status::PARTIAL_FILL;
    
    return Status::OK;
}
Status OrderBook::placeLimit(qty quantity, price px, side orderType) {

    if (px <= 0) {return Status::INVALID_PRICE;}
    if (quantity <= 0) {return Status::INVALID_QTY;} //O(1)

    id oid = getNewID();
    if ((id)orderIDs.size() <= oid) orderIDs.resize(oid + 1); 

    auto& pLevel = orders[orderType][px];
    
    pLevel.push_back( Order {
        .orderID = oid,
        .price = px,
        .quantity = quantity,
        .ts = getTime()
    }); //O(log n) for map insertion, O(1) for list insertion

    orderIDs[oid] = Pointer {orderType, px, prev(pLevel.end()), true}; //O(1) (contiguious array indexing instead of hashing)

    return matchOrders(orderType); //Use of function at the end might be inefficient.
}
Status OrderBook::cancelOrder(id orderID) {
    if (orderID < 0 || orderID >= (id)orderIDs.size()) {return Status::ORDER_NOT_FOUND;}

    Pointer& it = orderIDs[orderID]; 

    if (!it.active) {return Status::ORDER_INACTIVE;}

    auto mapIt = orders[it.orderType].find(it.price);
    if (mapIt == orders[it.orderType].end()) return Status::ORDER_NOT_FOUND;
    auto& pLevel = mapIt->second;

    pLevel.erase(it.iterator); //O(1)
    if (pLevel.empty()) orders[it.orderType].erase(it.price); //O(1)

    it.active = false;
    return Status::OK;
}
Status OrderBook::modifyOrder(id orderID, price newPx, qty newQty) { //Maybe implement price changing without changing quantity priority selection.
    side s = orderIDs[orderID].orderType;
    Status stat = cancelOrder(orderID);
    if (stat != Status::OK) {return stat;}
    return placeLimit(newQty, newPx, s); //O(1)
}
price OrderBook::bestBid() const { //O(1)
    if (buy.empty()) return -1; 
    else return prev(buy.end()) -> first;
}
price OrderBook::bestAsk() const { //O(1)
    if (sell.empty()) return -1; 
    return sell.begin() -> first;
} 
price OrderBook::spread() const { //O(1)
    price topBuy = bestBid();
    price topSell = bestAsk();

    if (topSell == -1 || topBuy == -1) return -1;

    return topSell - topBuy;
} 
qty OrderBook::volume(price px) const { //O(n)
    qty total = 0;

    auto b = buy.find(px);
    auto s = sell.find(px);

    if (b != buy.end()) for (const Order& ord : b -> second) total += ord.quantity;
    if (s != sell.end()) for (const Order& ord : s -> second) total += ord.quantity;

    return total;
}
void OrderBook::clear() { //O(1)
    orderId = -1;
    orderIDs.clear();
    buy.clear();
    sell.clear();
    trades.clear();
}
tuple<qty, qty> OrderBook::size() const {
    qty s, t;
    s = t = 0;

    for (auto& [key, value] : buy ) { //O(n)
        for (const Order& ord : value) s += ord.quantity;
    }

    for (auto& [key, value] : sell ) { //O(n)
        for (const Order& ord : value) t += ord.quantity;
    }

    return tuple<qty, qty> {s, t};
}
tuple<int64_t, int64_t> OrderBook::numOrders() const {
    int s = 0;

    for (auto& [key, value] : buy ) s += value.size(); //O(n)

    return tuple<int64_t, int64_t> {s, orderId - s};
}
vector<Order> OrderBook::getBook() const {
    vector<Order> book;
    
    for (auto& [key, value] : buy) {
        for (const Order& order : value) book.push_back(order);
    }

    for (auto& [key, value] : sell) {
        for (const Order& order : value) book.push_back(order);
    }

    return book;
}
vector<Trade> OrderBook::getTrades() const {return trades;}

timestamp OrderBook::getTime() {
    return duration_cast<microseconds> (steady_clock::now().time_since_epoch()).count();
}
id OrderBook::getNewID() {
    orderId++; 
    return orderId;
}
Status OrderBook::matchOrders(side incomingType) {
    
    while (!buy.empty() && !sell.empty()) {
        price bestBuy = bestBid();
        price bestSell = bestAsk();
        if (bestBuy < bestSell) return Status::OK;

        Order& topBuy = buy[bestBuy].front();
        Order& topSell = sell[bestSell].front();

        qty quantity = min(topBuy.quantity, topSell.quantity);

        Trade newTrade;
        newTrade.buyerID = topBuy.orderID;
        newTrade.sellerID = topSell.orderID;
        newTrade.price = (incomingType)? bestSell : bestBuy;
        newTrade.quantity = quantity;
        newTrade.ts = getTime();

        trades.push_back(newTrade); //O(1)
        
        topBuy.quantity -= quantity;
        topSell.quantity -= quantity;

        if (topBuy.quantity == 0) {

            orderIDs[topBuy.orderID].active = false;
            buy[bestBuy].pop_front();
        }

        if (topSell.quantity == 0) {

            orderIDs[topSell.orderID].active = false;
            sell[bestSell].pop_front();
        }
                
        if (buy[bestBuy].empty()) buy.erase(bestBuy);
        if (sell[bestSell].empty()) sell.erase(bestSell);
    }

    return Status::BOOK_EMPTY;
}