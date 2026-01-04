#include <types.hpp>
#include <map>

using namespace std;
using namespace std::chrono;

class OrderBook {
    public: 
    
    Status placeMarket(qty quantity, side orderType);
    Status placeLimit(qty quantity, price px, side orderType);
    Status cancelOrder(id orderID);
    Status modifyOrder(id orderID, price newPx, qty newQty);

    price bestBid() const;
    price bestAsk() const;
    price spread() const;

    qty volume(price px) const;
    tuple<qty, qty> size() const;
    tuple<int64_t, int64_t> numOrders() const;

    void clear();

    vector<Trade> getTrades() const;
    vector<Order> OrderBook::getBook() const;

    private:

    id OrderBook::orderId = -1;
    vector<Pointer> orderIDs;
    vector<Trade> trades;
    array<map<price, level>, 2> orders;

    map<price, level>& OrderBook::sell = orders[0];
    map<price, level>& OrderBook::buy = orders[1];

    id getNewID();
    timestamp getTime();
    Status matchOrders(side incomingType);
};
