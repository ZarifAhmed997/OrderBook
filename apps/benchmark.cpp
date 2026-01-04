#include <limits>
#include <chrono>
#include <iostream>
#include <fstream>
#include <random>

#include <orderbook.hpp>



using namespace std;

static inline int64_t clamp_i64(int64_t x, int64_t lo, int64_t hi) {
    return x < lo ? lo : (x > hi ? hi : x);
}

struct LoadConfig {
    int64_t ops = 1000000;          // how many actions
    double pLimit = 0.85;             // fraction limit orders
    double pBuy = 0.50;               // fraction buys
    int64_t startMid = 10000;        // price in ticks (e.g. 100.00 -> 10000)
    int64_t tick = 1;                 // tick size
    int64_t maxSpread = 50;           // ticks around mid for limit pricing
    int32_t minQty = 1;
    int32_t maxQty = 500;
    int64_t warmup = 10000;          // ignore first N ops to generate liquidity
    int64_t checkEvery = 50000;      // run sanity checks every N ops
    uint64_t seed = 123456789;        // reproducible
};

static inline void cheapInvariants(OrderBook& ob) {
    // minimal checks to catch obvious corruption
    price bid = ob.bestBid();
    price ask = ob.bestAsk();
    if (bid != -1 && ask != -1 && bid > ask) {
        std::cerr << "[INVARIANT FAIL] crossed book: bid=" << bid << " ask=" << ask << "\n";
        std::terminate();
    }
    // You can add more later: sizes non-negative, etc.


}

vector<Trade> massiveTestingAgent(const LoadConfig& cfg) {
    OrderBook ob;

    std::mt19937_64 rng(cfg.seed);
    std::uniform_real_distribution<double> uni01(0.0, 1.0);
    std::uniform_int_distribution<int32_t> qtyDist(cfg.minQty, cfg.maxQty);
    std::uniform_int_distribution<int64_t> spreadDist(-cfg.maxSpread, cfg.maxSpread);

    int64_t mid = cfg.startMid;

    // Timing
    using clock = chrono::steady_clock;
    auto t0 = clock::now();

    int64_t ok = 0, invalid = 0, partial = 0, empty = 0;

    for (int64_t i = 1; i <= cfg.ops; ++i) {
        bool isLimit = uni01(rng) < cfg.pLimit;
        bool isBuy = uni01(rng) < cfg.pBuy;

        int32_t q = qtyDist(rng);

        // crude “price process”: random walk on mid
        // (keeps book from drifting to infinity)
        if ((i % 1000) == 0) {
            mid += spreadDist(rng);
            mid = clamp_i64(mid, 1, std::numeric_limits<int64_t>::max() / 4);
        }

        Status st;
        if (isLimit) {
            int64_t offset = spreadDist(rng);
            int64_t px = mid + offset;

            // ensure positive price
            px = clamp_i64(px, 1, std::numeric_limits<int64_t>::max() / 4);

            st = ob.placeLimit(q, px, isBuy);
        } else {
            st = ob.placeMarket(q, isBuy);
        }



        switch (st) {
            case Status::OK: ok++; break;
            case Status::INVALID_QTY: invalid++; break;
            case Status::INVALID_PRICE: invalid++; break;
            case Status::PARTIAL_FILL: partial++; break;
            case Status::BOOK_EMPTY: empty++; break;
            default: break;
        }

        if (cfg.checkEvery > 0 && (i % cfg.checkEvery) == 0) {
            cheapInvariants(ob);
            std::cout << "[progress] " << i << "/" << cfg.ops
                      << " ok=" << ok << " partial=" << partial
                      << " empty=" << empty << " invalid=" << invalid << "\n";
        }
    }

    auto t1 = clock::now();
    double seconds = std::chrono::duration<double>(t1 - t0).count();
    double opsPerSec = cfg.ops / seconds;

    std::cout << "\nDONE\n"
              << "ops: " << cfg.ops << "\n"
              << "time(s): " << seconds << "\n"
              << "throughput(ops/s): " << opsPerSec << "\n"
              << "ok=" << ok << " partial=" << partial << " empty=" << empty << " invalid=" << invalid << "\n"
              << "trades stored: " << ob.getTrades().size() << "\n";

    return ob.getTrades();
}

int main() {
    LoadConfig cfg;
    cfg.ops = 3000000;
    cfg.pLimit = 0.8;
    cfg.pBuy = 0.5;
    cfg.startMid = 10000;
    cfg.tick = 1;
    cfg.maxSpread = 50;
    cfg.minQty = 1;
    cfg.maxQty = 10000;
    cfg.warmup = 10000;
    cfg.checkEvery = 50000;
    cfg.seed = 8768698;

    vector<Trade> trades = massiveTestingAgent(cfg);

    string csvLatency = "data/example_latency.csv";
    string csvData = "data/example_data.csv";
    ofstream f(csvData);
    ofstream l(csvLatency);

    if (!l.is_open()) {
        cerr << "Error opening file: " << csvLatency << "\n";
        return 0;  
    } 

    else l << "Time,NumOfOrders\n";

    if (!f.is_open()) {
        cerr << "Error opening file: " << csvData << "\n";
        return 0;
    }

    else f << "Price,Volume,Time\n";

    for (int i = 0; i < trades.size(); i++) {
        f << trades[i].price << "," << trades[i].quantity << "," << trades[i].ts << "\n";
        l << trades[i].ts << "," << i << "\n";
    }

    return 0;
}