
#include <list>

using namespace std;

using price = int64_t;
using qty = int32_t;
using side = bool;
using id = int64_t;
using timestamp = int64_t;

struct Order {
    id orderID;
    qty quantity;
    price price;
    timestamp ts;
};

using level = list<Order>;

enum class Status {
    OK,
    INVALID_QTY,
    INVALID_PRICE,
    BOOK_EMPTY,
    PARTIAL_FILL,
    ORDER_NOT_FOUND,
    ORDER_INACTIVE
};

struct Trade {
    id sellerID;
    id buyerID;
    price price;
    qty quantity;
    timestamp ts;
};

struct Pointer {
    side orderType;
    price price;
    level::iterator iterator;
    bool active;
};