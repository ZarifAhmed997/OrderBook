#include <limitBook.hpp>
#include <types.hpp>
#include <iostream>

void main() {
    OrderBook ob;
    string command;

    cout << "Welcome to the Order Book Interface!" << endl;
    cout << "Available commands: placeLimit, placeMarket, cancelOrder, modifyOrder, bestBid, bestAsk, volume, spread, size, numOrders, printBook, printTrades, clear, exit" << endl;

    while (true) {
        cout << "\nEnter command: ";
        cin >> command;

        if (command == "placeLimit") {
            qty quantity;
            price price;
            side orderType;
            cout << "Enter quantity: ";
            cin >> quantity;
            cout << "Enter price: ";
            cin >> price;
            while (true) {
                cout << "Enter order type (0 for sell, 1 for buy): ";
                cin >> orderType;
                if (orderType == 0 || orderType == 1) break;
                else cout << "Invalid order type. Please enter 0 for sell or 1 for buy." << endl;
            }
            Status stat = ob.placeLimit(quantity, price, orderType);
            if (stat != Status::OK) {
                cout << "Error placing limit order: ";
                switch (stat) {
                    case Status::INVALID_QTY:
                        cout << "Invalid quantity." << endl;
                        break;
                    case Status::INVALID_PRICE:
                        cout << "Invalid price." << endl;
                        break;
                    default:
                        cout << "Unknown error." << endl;
                }
            } else {
                cout << "Limit order placed successfully." << endl;
            }
        } 

        else if (command == "placeMarket") {
            qty quantity;
            side orderType;
            cout << "Enter quantity: ";
            cin >> quantity;
            cout << "Enter order type (0 for sell, 1 for buy): ";
            cin >> orderType;

            Status stat = ob.placeMarket(quantity, orderType);
            if (stat != Status::OK) {
                cout << "Error placing market order: ";
                switch (stat) {
                    case Status::INVALID_QTY:
                        cout << "Invalid quantity." << endl;
                        break;
                    default:
                        cout << "Unknown error." << endl;
                } 
            } else {
                cout << "Market order placed successfully." << endl;
            }
        }
        else if (command == "cancelOrder") {
            id orderID;
            cout << "Enter order ID to cancel: ";
            cin >> orderID;

            Status stat = ob.cancelOrder(orderID);
            if (stat != Status::OK) {
                cout << "Error cancelling order: ";
                switch (stat) {
                    case Status::ORDER_NOT_FOUND:
                        cout << "Order not found." << endl;
                        break;
                    case Status::ORDER_INACTIVE:
                        cout << "Order is already inactive." << endl;
                        break;
                    default:
                        cout << "Unknown error." << endl;
                }
            } else {
                cout << "Order cancelled successfully." << endl;
            }
        } 

        else if (command == "modifyOrder") {
            id orderID;
            qty newQty;
            price newPrice;
            cout << "Enter order ID to modify: ";
            cin >> orderID;
            cout << "Enter new quantity: ";
            cin >> newQty;
            cout << "Enter new price: ";
            cin >> newPrice;

            Status stat = ob.modifyOrder(orderID, newQty, newPrice);
            if (stat != Status::OK) {
                cout << "Error modifying order: ";
                switch (stat) {
                    case Status::ORDER_NOT_FOUND:
                        cout << "Order not found." << endl;
                        break;
                    case Status::ORDER_INACTIVE:
                        cout << "Order is inactive." << endl;
                        break;
                    case Status::INVALID_QTY:
                        cout << "Invalid quantity." << endl;
                        break;
                    case Status::INVALID_PRICE:
                        cout << "Invalid price." << endl;
                        break;
                    default:
                        cout << "Unknown error." << endl;   
                } 
            } else {
                cout << "Order modified successfully." << endl;
            }
        }

        else if (command == "bestBid") {
            price p = ob.bestBid();
            if (p == -1) cout << "Best Bid: N/A" << endl;
            else cout << "Best Bid: " << p << endl;
        } 

        else if (command == "bestAsk") {
            price p = ob.bestAsk();
            if (p == -1) cout << "Best Ask: N/A" << endl;
            else cout << "Best Ask: " << p << endl;
        } 

        else if (command == "volume") {
            price pricePoint;
            cout << "Enter price point: ";
            cin >> pricePoint;
            cout << "Volume at price " << pricePoint << ": " << ob.volume(pricePoint) << endl;
        } 

        else if (command == "spread") {
            price s = ob.spread();
            if (s == -1) cout << "Spread: N/A" << endl;
            else cout << "Spread: " << s << endl;
        } 

        else if (command == "size") {
            qty buySize, sellSize;
            tie(buySize, sellSize) = ob.size();
            cout << "Buy Size: " << buySize << ", Sell Size: " << sellSize << endl;
        } 

        else if (command == "numOrders") {
            int64_t buyOrders, sellOrders;
            tie(buyOrders, sellOrders) = ob.numOrders();
            cout << "Number of Buy Orders: " << buyOrders << ", Number of Sell Orders: " << sellOrders << endl;
        } 

        else if (command == "getBook") {
            vector<Order> book = ob.getBook();
            for (Order order : book) {
                cout << order.orderID << " " << order.price << " " << order.quantity << " " << order.ts << endl;
            }
        } 

        else if (command == "getTrades") {
            vector<Trade> trades = ob.getTrades();
            for (Trade trade : trades) {
                cout << "Buyer ID: " << trade.buyerID << ", Seller ID: " << trade.sellerID << ", Price: " << trade.price << ", Quantity: " << trade.quantity << ", Timestamp: " << trade.ts << endl;
            }
        }
        
        else if (command == "exit") {
            cout << "Exiting Order Book Interface. Goodbye!" << endl;
            break;

        } else {
            cout << "Invalid command. Please try again." << endl;
        }
    }
}
