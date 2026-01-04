
#include "orderbook.hpp" 
#include <cassert>
#include <iostream>
#include <random>
#include <tuple>

// Helper: basic invariants you can check via public API
static void check_invariants(const OrderBook& ob) {
    price bid = ob.bestBid();
    price ask = ob.bestAsk();

    // If both sides exist, book must not be crossed.
    if (bid != -1 && ask != -1) {
        assert(bid <= ask);
        price spr = ob.spread();
        assert(spr >= 0);
        assert(spr == (ask - bid));
    }

    // If one side missing, spread should be -1 
    if (bid == -1 || ask == -1) {
        assert(ob.spread() == -1);
    }

    // Sizes should never be negative
    auto [buySize, sellSize] = ob.size();
    assert(buySize >= 0);
    assert(sellSize >= 0);
}

static void test_empty_book() {
    OrderBook ob;
    assert(ob.bestBid() == -1);
    assert(ob.bestAsk() == -1);
    assert(ob.spread() == -1);
    check_invariants(ob);
}

static void test_simple_cross_trade() {
    OrderBook ob;

    // First order is ID 0 (buy), second is ID 1 (sell)
    assert(ob.placeLimit(10, 100, true) == Status::OK);   // buy 10 @100
    assert(ob.placeLimit(10, 99,  false) == Status::OK);  // sell 10 @99 crosses

    auto trades = ob.getTrades();
    assert(!trades.empty());

    // Here incomingType is sell (false) since we placed a sell second,
    // The code sets price = incomingType ? bestSell : bestBuy => bestBuy (=100)
    const Trade& t = trades.back();
    assert(t.quantity == 10);
    assert(t.buyerID == 0);
    assert(t.sellerID == 1);
    assert(t.price == 100);

    check_invariants(ob);
}

static void test_fifo_at_same_price() {
    OrderBook ob;

    // IDs: 0,1 are buys at same price; ID 2 is the sell that crosses
    assert(ob.placeLimit(5, 100, true) == Status::OK);  // buy id 0
    assert(ob.placeLimit(5, 100, true) == Status::OK);  // buy id 1
    assert(ob.placeLimit(7, 100, false) == Status::OK); // sell id 2 crosses

    auto trades = ob.getTrades();
    // Should fill id0 fully (5), then id1 partially (2)
    assert(trades.size() >= 2);

    // First trade should be buyer 0
    assert(trades[0].buyerID == 0);
    assert(trades[0].sellerID == 2);
    assert(trades[0].quantity == 5);

    // Second trade should be buyer 1
    assert(trades[1].buyerID == 1);
    assert(trades[1].sellerID == 2);
    assert(trades[1].quantity == 2);

    check_invariants(ob);
}

static void test_cancel_and_inactive() {
    OrderBook ob;

    // Place a limit -> id 0
    assert(ob.placeLimit(10, 101, true) == Status::OK);

    // Cancel works
    assert(ob.cancelOrder(0) == Status::OK);

    // Cancel again should be inactive
    assert(ob.cancelOrder(0) == Status::ORDER_INACTIVE);

    // Cancel nonsense id
    assert(ob.cancelOrder(999999) == Status::ORDER_NOT_FOUND);

    check_invariants(ob);
}

static void test_modify_order_basic() {
    OrderBook ob;

    // Place buy id 0
    assert(ob.placeLimit(10, 100, true) == Status::OK);

    // Modify will cancel then re-place a NEW order (new ID)
    assert(ob.modifyOrder(0, 10, 105) == Status::OK);

    // Old order is inactive, so cancel should say inactive now
    assert(ob.cancelOrder(0) == Status::ORDER_INACTIVE);

    // New best bid should be 105 (assuming book only has that buy)
    assert(ob.bestBid() == 105);

    check_invariants(ob);
}

// Randomized “fuzz” test: throws lots of ops at your book and checks invariants.
// This catches crashes, crossed book states, negative sizes, etc.
static void test_fuzz_invariants() {
    OrderBook ob;
    std::mt19937_64 rng(12345);

    std::uniform_int_distribution<int> opDist(0, 3);     // 0=limit,1=market,2=cancel,3=modify
    std::uniform_int_distribution<int> sideDist(0, 1);   // 0 sell, 1 buy
    std::uniform_int_distribution<int> qtyDist(1, 500);
    std::uniform_int_distribution<int> pxDist(90, 110);  // tight band around 100
    std::uniform_int_distribution<long long> idDist(0, 5000);

    for (int i = 0; i < 20000; ++i) {
        int op = opDist(rng);
        bool s = (sideDist(rng) == 1);
        qty q = (qty)qtyDist(rng);
        price p = (price)pxDist(rng);

        if (op == 0) {
            (void)ob.placeLimit(q, p, s);
        } else if (op == 1) {
            (void)ob.placeMarket(q, s);
        } else if (op == 2) {
            id oid = (id)idDist(rng);
            (void)ob.cancelOrder(oid);
        } else {
            id oid = (id)idDist(rng);
            price newP = (price)pxDist(rng);
            qty newQ = (qty)qtyDist(rng);
            (void)ob.modifyOrder(oid, newQ, newP);
        }

        // Check invariants frequently
        check_invariants(ob);
    }
}

int main() {
    test_empty_book();
    test_simple_cross_trade();
    test_fifo_at_same_price();
    test_cancel_and_inactive();
    test_modify_order_basic();
    test_fuzz_invariants();

    std::cout << "All OrderBook tests passed.\n";
    return 0;
}
