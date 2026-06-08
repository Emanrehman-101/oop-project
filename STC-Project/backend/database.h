#ifndef DATABASE_H
#define DATABASE_H

// =================================================================
// DUAL persistence selector:
// Set USE_MYSQL to 1 to enable actual MySQL database connections.
// Set USE_MYSQL to 0 to use clean, local text file storage (beginner friendly).
// =================================================================
#define USE_MYSQL 0

#include "classes.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>

// If MySQL is enabled, include the MySQL headers
#if USE_MYSQL
    #include <mysql.h>
#endif

using namespace std;

class DatabaseManager {
private:
#if USE_MYSQL
    MYSQL* conn;
    bool isConnected;
#endif

    // File names for File Handling fallback mode
    const string usersFile = "users.txt";
    const string menuFile = "menu.txt";
    const string ordersFile = "orders.txt";
    const string walletTransactionsFile = "wallet_transactions.txt";
    const string walletsFile = "wallets.txt";
    const string notificationsFile = "notifications.txt";
    const string reviewsFile = "reviews.txt";
    const string favoritesFile = "favorites.txt";
    const string announcementsFile = "announcements.txt";
    const string couponsFile = "coupons.txt";
    const string bannersFile = "banners.txt";

    // =================================================================
    // HELPER FUNCTIONS FOR FILE HANDLING MODE (USE_MYSQL = 0)
    // =================================================================

    // Split string helper
    vector<string> split(const string& str, char delimiter) {
        vector<string> tokens;
        string token;
        istringstream tokenStream(str);
        while (getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    // Get current system time as formatted string
    string getCurrentTimeStr() {
        time_t now = time(0);
        struct tm tstruct;
        char buf[80];
        // Safe time conversion on Windows/cross-platform
#ifdef _MSC_VER
        localtime_s(&tstruct, &now);
#else
        tstruct = *localtime(&now);
#endif
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);
        return string(buf);
    }

    // Hash function (Simple beginner-friendly hash, or standard MD5/SHA256 simulation)
    string hashPassword(const string& password) {
        unsigned long hash = 5381;
        for (char c : password) {
            hash = ((hash << 5) + hash) + c;
        }
        stringstream ss;
        ss << hex << hash << (hash * 31);
        return ss.str();
    }

    // Load initial menu and setup dummy files if not present
    void initializeDefaultFiles() {
        // 1. Users File
        ifstream uf(usersFile);
        if (!uf.is_open()) {
            ofstream out(usersFile);
            if (out.is_open()) {
                out << "1|admin|" << hashPassword("admin123") << "|admin@stc.com|admin|+966500000001|STC Head Office, Riyadh\n";
                out << "2|customer|" << hashPassword("customer123") << "|customer@stc.com|customer|+966500000002|Olaya District, Riyadh\n";
                out.close();
            }
        }
        uf.close();

        // 2. Wallets File
        ifstream wf(walletsFile);
        if (!wf.is_open()) {
            ofstream out(walletsFile);
            if (out.is_open()) {
                out << "2|100.00\n";
                out.close();
            }
        }
        wf.close();

        // 3. Wallet Transactions File
        ifstream wtf(walletTransactionsFile);
        if (!wtf.is_open()) {
            ofstream out(walletTransactionsFile);
            if (out.is_open()) {
                out << "1|2|DEPOSIT|100.00|Welcome Balance|" << getCurrentTimeStr() << "\n";
                out.close();
            }
        }
        wtf.close();

        // 4. Menu File
        ifstream mf(menuFile);
        if (!mf.is_open() || mf.peek() == std::ifstream::traits_type::eof()) {
            ofstream out(menuFile);
            if (out.is_open()) {
                out << "1|STC Signature Burger|Premium beef patty with cheddar cheese and STC orange sauce.|8.99|Burgers|0|4.8|124\n";
                out << "2|Crispy Chicken Burger|Crispy fried chicken breast, lettuce, and spicy mayo in a brioche bun.|7.49|Burgers|0|4.6|98\n";
                out << "3|STC Deluxe Pizza|Stone-baked pizza loaded with pepperoni, beef, and mozzarella.|12.99|Pizza|0|4.9|142\n";
                out << "4|Margarita Flatbread|Simplicity at its best: tomato sauce, mozzarella, and fresh basil.|9.50|Pizza|0|4.2|53\n";
                out << "5|Golden French Fries|Crispy golden fries seasoned with a light touch of sea salt.|3.00|Sides|0|4.5|210\n";
                out << "6|STC Dynamite Wings|Eight pieces of crispy chicken wings tossed in signature sweet glaze.|5.99|Sides|1|4.7|76\n";
                out << "7|Iced Latte|Chilled espresso poured over creamy whole milk and ice, sweetened.|4.50|Drinks|0|4.4|85\n";
                out << "8|Fresh Orange Juice|100% natural, freshly squeezed juice, served ice cold.|3.50|Drinks|0|4.9|115\n";
                out << "9|Chocolate Lava Cake|Warm chocolate cake with a molten fudge center and vanilla ice cream.|6.50|Desserts|0|4.8|189\n";
                out.close();
            }
        }
        mf.close();

        // 5. Coupons File
        ifstream cf(couponsFile);
        if (!cf.is_open()) {
            ofstream out(couponsFile);
            if (out.is_open()) {
                out << "STC20|20.00|1\n";
                out << "STC10|10.00|1\n";
                out << "WELCOME|15.00|1\n";
                out.close();
            }
        }
        cf.close();

        // 6. Banners File
        ifstream bf(bannersFile);
        if (!bf.is_open()) {
            ofstream out(bannersFile);
            if (out.is_open()) {
                // Customized default image banner to STC Headquarters image
                out << "1|Order STC Signature Feast - Save 20% today with Coupon: STC20!|/images/stc_building.jpg|1\n";
                out.close();
            }
        }
        bf.close();

        // 7. Announcements File
        ifstream af(announcementsFile);
        if (!af.is_open()) {
            ofstream out(announcementsFile);
            if (out.is_open()) {
                out << "1|Welcome to the new STC Ordering System! Use code STC20 for 20% off all orders!|" << getCurrentTimeStr() << "\n";
                out.close();
            }
        }
        af.close();

        // 8. Reviews File
        ifstream rf(reviewsFile);
        if (!rf.is_open()) {
            ofstream out(reviewsFile);
            if (out.is_open()) {
                out << "1|2|1|5|Best burger I have ever had! STC sauce is incredible.|customer|" << getCurrentTimeStr() << "\n";
                out << "2|2|3|5|A bit spicy but very tasty, plenty of pepperoni.|customer|" << getCurrentTimeStr() << "\n";
                out.close();
            }
        }
        rf.close();

        // 9. Favorites File
        ifstream fvf(favoritesFile);
        if (!fvf.is_open()) {
            ofstream out(favoritesFile);
            if (out.is_open()) {
                out << "2|1\n";
                out << "2|3\n";
                out.close();
            }
        }
        fvf.close();
    }

public:
    // Constructor
    DatabaseManager() {
        initializeDefaultFiles();

#if USE_MYSQL
        isConnected = false;
        conn = mysql_init(NULL);
        if (conn == NULL) {
            cerr << "MySQL Initialization failed" << endl;
            return;
        }
        if (mysql_real_connect(conn, "localhost", "root", "", "stc_ordering_db", 3306, NULL, 0)) {
            isConnected = true;
        } else {
            cerr << "MySQL Connection failed: " << mysql_error(conn) << endl;
        }
#endif
    }

    // Destructor
    ~DatabaseManager() {
#if USE_MYSQL
        if (isConnected) {
            mysql_close(conn);
        }
#endif
    }

    // =================================================================
    // 1. AUTHENTICATION (Login & Register)
    // =================================================================

    // Register a new customer
    bool registerCustomer(const string& username, const string& password, const string& email, const string& phone, const string& address) {
        string hashedPass = hashPassword(password);

#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "INSERT INTO users (username, password, email, role, phone, address) VALUES ("
                  << "'" << username << "', "
                  << "'" << hashedPass << "', "
                  << "'" << email << "', "
                  << "'customer', "
                  << "'" << phone << "', "
                  << "'" << address << "')";
            
            if (mysql_query(conn, query.str().c_str()) == 0) {
                int custId = mysql_insert_id(conn);
                
                stringstream walletQuery;
                walletQuery << "INSERT INTO wallets (customer_id, balance) VALUES (" << custId << ", 100.00)";
                mysql_query(conn, walletQuery.str().c_str());

                stringstream txQuery;
                txQuery << "INSERT INTO wallet_transactions (customer_id, type, amount, description) VALUES ("
                        << custId << ", 'DEPOSIT', 100.00, 'Welcome Balance')";
                mysql_query(conn, txQuery.str().c_str());

                return true;
            }
            return false;
        }
#endif
        // Fallback to File Handling
        ifstream in(usersFile);
        string line;
        while (getline(in, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() >= 2 && parts[1] == username) {
                in.close();
                return false; // Username exists
            }
        }
        in.close();

        int newId = 1;
        ifstream in2(usersFile);
        while (getline(in2, line)) {
            vector<string> parts = split(line, '|');
            if (!parts.empty()) {
                int tempId = atoi(parts[0].c_str());
                if (tempId >= newId) newId = tempId + 1;
            }
        }
        in2.close();

        ofstream out(usersFile, ios::app);
        if (out.is_open()) {
            out << newId << "|" << username << "|" << hashedPass << "|" << email << "|customer|" << phone << "|" << address << "\n";
            out.close();

            ofstream walletOut(walletsFile, ios::app);
            if (walletOut.is_open()) {
                walletOut << newId << "|100.00\n";
                walletOut.close();
            }

            ofstream txOut(walletTransactionsFile, ios::app);
            if (txOut.is_open()) {
                txOut << newId << "|" << newId << "|DEPOSIT|100.00|Welcome Balance|" << getCurrentTimeStr() << "\n";
                txOut.close();
            }
            return true;
        }
        return false;
    }

    // Login user
    User* loginUser(const string& username, const string& password) {
        string hashedPass = hashPassword(password);

#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "SELECT * FROM users WHERE username = '" << username << "' AND password = '" << hashedPass << "'";
            if (mysql_query(conn, query.str().c_str()) == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (res && mysql_num_rows(res) > 0) {
                    MYSQL_ROW row = mysql_fetch_row(res);
                    int id = atoi(row[0]);
                    string uName = row[1];
                    string email = row[3];
                    string role = row[4];
                    string phone = row[5] ? row[5] : "";
                    string address = row[6] ? row[6] : "";
                    
                    mysql_free_result(res);

                    if (role == "admin") {
                        return new Admin(id, uName, "", email);
                    } else {
                        double balance = 0.0;
                        stringstream wQuery;
                        wQuery << "SELECT balance FROM wallets WHERE customer_id = " << id;
                        if (mysql_query(conn, wQuery.str().c_str()) == 0) {
                            MYSQL_RES* wRes = mysql_store_result(conn);
                            if (wRes && mysql_num_rows(wRes) > 0) {
                                MYSQL_ROW wRow = mysql_fetch_row(wRes);
                                balance = atof(wRow[0]);
                            }
                            if (wRes) mysql_free_result(wRes);
                        }

                        Customer* cust = new Customer(id, uName, "", email, phone, address, balance);
                        
                        stringstream txQuery;
                        txQuery << "SELECT type, amount, description, timestamp FROM wallet_transactions WHERE customer_id = " << id << " ORDER BY timestamp DESC";
                        if (mysql_query(conn, txQuery.str().c_str()) == 0) {
                            MYSQL_RES* txRes = mysql_store_result(conn);
                            if (txRes) {
                                MYSQL_ROW txRow;
                                while ((txRow = mysql_fetch_row(txRes))) {
                                    stringstream log;
                                    log << txRow[2] << " (" << txRow[0] << "): " << (strcmp(txRow[0], "DEPOSIT") == 0 ? "+" : "-") << "$" << txRow[1] << " on " << txRow[3];
                                    cust->getWallet().addTransactionLog(log.str());
                                }
                                mysql_free_result(txRes);
                            }
                        }
                        return cust;
                    }
                }
                if (res) mysql_free_result(res);
            }
        }
#endif
        // Fallback to File Handling
        ifstream in(usersFile);
        string line;
        while (getline(in, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() >= 5 && parts[1] == username && parts[2] == hashedPass) {
                int id = atoi(parts[0].c_str());
                string uName = parts[1];
                string email = parts[3];
                string role = parts[4];

                in.close();

                if (role == "admin") {
                    return new Admin(id, uName, "", email);
                } else {
                    string phone = parts.size() >= 6 ? parts[5] : "";
                    string address = parts.size() >= 7 ? parts[6] : "";

                    double balance = 100.0;
                    ifstream wIn(walletsFile);
                    string wLine;
                    while (getline(wIn, wLine)) {
                        vector<string> wParts = split(wLine, '|');
                        if (wParts.size() >= 2 && atoi(wParts[0].c_str()) == id) {
                            balance = atof(wParts[1].c_str());
                            break;
                        }
                    }
                    wIn.close();

                    Customer* cust = new Customer(id, uName, "", email, phone, address, balance);

                    ifstream txIn(walletTransactionsFile);
                    string txLine;
                    while (getline(txIn, txLine)) {
                        vector<string> txParts = split(txLine, '|');
                        if (txParts.size() >= 6 && atoi(txParts[1].c_str()) == id) {
                            stringstream log;
                            log << txParts[4] << " (" << txParts[2] << "): " 
                                << (txParts[2] == "DEPOSIT" ? "+" : "-") << "$" << txParts[3] << " on " << txParts[5];
                            cust->getWallet().addTransactionLog(log.str());
                        }
                    }
                    txIn.close();
                    return cust;
                }
            }
        }
        in.close();
        return NULL;
    }

    // =================================================================
    // 2. MENU MANAGEMENT
    // =================================================================

    vector<MenuItem> getMenu() {
        vector<MenuItem> menuList;

#if USE_MYSQL
        if (isConnected) {
            if (mysql_query(conn, "SELECT * FROM menu") == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (res) {
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(res))) {
                        int id = atoi(row[0]);
                        string name = row[1];
                        string desc = row[2] ? row[2] : "";
                        double price = atof(row[3]);
                        string cat = row[4];
                        bool sold = atoi(row[5]) != 0;
                        double rating = atof(row[6]);
                        int revCount = atoi(row[7]);
                        menuList.push_back(MenuItem(id, name, desc, price, cat, sold, rating, revCount));
                    }
                    mysql_free_result(res);
                }
            }
            return menuList;
        }
#endif
        // Fallback to File Handling
        ifstream in(menuFile);
        string line;
        while (getline(in, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() >= 8) {
                int id = atoi(parts[0].c_str());
                string name = parts[1];
                string desc = parts[2];
                double price = atof(parts[3].c_str());
                string cat = parts[4];
                bool sold = parts[5] == "1" || parts[5] == "true";
                double rating = atof(parts[6].c_str());
                int revCount = atoi(parts[7].c_str());
                menuList.push_back(MenuItem(id, name, desc, price, cat, sold, rating, revCount));
            }
        }
        in.close();
        return menuList;
    }

    bool addMenuItem(const string& name, const string& description, double price, const string& category) {
#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "INSERT INTO menu (name, description, price, category, is_sold_out, rating, reviews_count) VALUES ("
                  << "'" << name << "', "
                  << "'" << description << "', "
                  << price << ", "
                  << "'" << category << "', "
                  << "0, 5.0, 0)";
            return mysql_query(conn, query.str().c_str()) == 0;
        }
#endif
        vector<MenuItem> currentMenu = getMenu();
        int newId = 1;
        for (const auto& item : currentMenu) {
            if (item.getId() >= newId) newId = item.getId() + 1;
        }

        ofstream out(menuFile, ios::app);
        if (out.is_open()) {
            out << newId << "|" << name << "|" << description << "|" << price << "|" << category << "|0|5.0|0\n";
            out.close();
            return true;
        }
        return false;
    }

    bool updateMenuItem(int id, const string& name, const string& description, double price, const string& category, bool isSoldOut) {
#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "UPDATE menu SET "
                  << "name = '" << name << "', "
                  << "description = '" << description << "', "
                  << "price = " << price << ", "
                  << "category = '" << category << "', "
                  << "is_sold_out = " << (isSoldOut ? "1" : "0") << " "
                  << "WHERE id = " << id;
            return mysql_query(conn, query.str().c_str()) == 0;
        }
#endif
        vector<MenuItem> menu = getMenu();
        bool found = false;
        ofstream out(menuFile);
        for (auto& item : menu) {
            if (item.getId() == id) {
                item.setName(name);
                item.setDescription(description);
                item.setPrice(price);
                item.setCategory(category);
                item.setIsSoldOut(isSoldOut);
                found = true;
            }
            out << item.getId() << "|"
                << item.getName() << "|"
                << item.getDescription() << "|"
                << item.getPrice() << "|"
                << item.getCategory() << "|"
                << (item.getIsSoldOut() ? "1" : "0") << "|"
                << item.getRating() << "|"
                << item.getReviewsCount() << "\n";
        }
        out.close();
        return found;
    }

    bool deleteMenuItem(int id) {
#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "DELETE FROM menu WHERE id = " << id;
            return mysql_query(conn, query.str().c_str()) == 0;
        }
#endif
        vector<MenuItem> menu = getMenu();
        bool found = false;
        ofstream out(menuFile);
        for (const auto& item : menu) {
            if (item.getId() == id) {
                found = true;
                continue;
            }
            out << item.getId() << "|"
                << item.getName() << "|"
                << item.getDescription() << "|"
                << item.getPrice() << "|"
                << item.getCategory() << "|"
                << (item.getIsSoldOut() ? "1" : "0") << "|"
                << item.getRating() << "|"
                << item.getReviewsCount() << "\n";
        }
        out.close();
        return found;
    }

    bool setMenuItemSoldOut(int id, bool sold) {
#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "UPDATE menu SET is_sold_out = " << (sold ? "1" : "0") << " WHERE id = " << id;
            return mysql_query(conn, query.str().c_str()) == 0;
        }
#endif
        vector<MenuItem> menu = getMenu();
        bool found = false;
        ofstream out(menuFile);
        for (auto& item : menu) {
            if (item.getId() == id) {
                item.setIsSoldOut(sold);
                found = true;
            }
            out << item.getId() << "|"
                << item.getName() << "|"
                << item.getDescription() << "|"
                << item.getPrice() << "|"
                << item.getCategory() << "|"
                << (item.getIsSoldOut() ? "1" : "0") << "|"
                << item.getRating() << "|"
                << item.getReviewsCount() << "\n";
        }
        out.close();
        return found;
    }

    // =================================================================
    // 3. WALLET OPERATIONS
    // =================================================================

    bool addWalletBalance(int customerId, double amount, const string& desc = "Balance Added") {
#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "UPDATE wallets SET balance = balance + " << amount << " WHERE customer_id = " << customerId;
            if (mysql_query(conn, query.str().c_str()) == 0) {
                stringstream txQuery;
                txQuery << "INSERT INTO wallet_transactions (customer_id, type, amount, description) VALUES ("
                        << customerId << ", 'DEPOSIT', " << amount << ", '" << desc << "')";
                mysql_query(conn, txQuery.str().c_str());
                return true;
            }
            return false;
        }
#endif
        ifstream win(walletsFile);
        string line;
        vector<pair<int, double>> wallets;
        bool found = false;
        while (getline(win, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() >= 2) {
                int id = atoi(parts[0].c_str());
                double bal = atof(parts[1].c_str());
                if (id == customerId) {
                    bal += amount;
                    found = true;
                }
                wallets.push_back(make_pair(id, bal));
            }
        }
        win.close();

        if (!found) {
            wallets.push_back(make_pair(customerId, amount));
        }

        ofstream wout(walletsFile);
        for (const auto& w : wallets) {
            wout << w.first << "|" << w.second << "\n";
        }
        wout.close();

        int newTxId = 1;
        ifstream txin(walletTransactionsFile);
        while (getline(txin, line)) {
            vector<string> parts = split(line, '|');
            if (!parts.empty()) {
                int id = atoi(parts[0].c_str());
                if (id >= newTxId) newTxId = id + 1;
            }
        }
        txin.close();

        ofstream txout(walletTransactionsFile, ios::app);
        if (txout.is_open()) {
            txout << newTxId << "|" << customerId << "|DEPOSIT|" << amount << "|" << desc << "|" << getCurrentTimeStr() << "\n";
            txout.close();
            return true;
        }
        return false;
    }

    bool deductWalletBalance(int customerId, double amount, const string& desc) {
#if USE_MYSQL
        if (isConnected) {
            stringstream checkQuery;
            checkQuery << "SELECT balance FROM wallets WHERE customer_id = " << customerId;
            if (mysql_query(conn, checkQuery.str().c_str()) == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (res && mysql_num_rows(res) > 0) {
                    MYSQL_ROW row = mysql_fetch_row(res);
                    double currentBal = atof(row[0]);
                    mysql_free_result(res);

                    if (currentBal >= amount) {
                        stringstream updateQuery;
                        updateQuery << "UPDATE wallets SET balance = balance - " << amount << " WHERE customer_id = " << customerId;
                        if (mysql_query(conn, updateQuery.str().c_str()) == 0) {
                            stringstream txQuery;
                            txQuery << "INSERT INTO wallet_transactions (customer_id, type, amount, description) VALUES ("
                                    << customerId << ", 'PAYMENT', " << amount << ", '" << desc << "')";
                            mysql_query(conn, txQuery.str().c_str());
                            return true;
                        }
                    }
                } else if (res) {
                    mysql_free_result(res);
                }
            }
            return false;
        }
#endif
        ifstream win(walletsFile);
        string line;
        vector<pair<int, double>> wallets;
        bool success = false;
        while (getline(win, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() >= 2) {
                int id = atoi(parts[0].c_str());
                double bal = atof(parts[1].c_str());
                if (id == customerId && bal >= amount) {
                    bal -= amount;
                    success = true;
                }
                wallets.push_back(make_pair(id, bal));
            }
        }
        win.close();

        if (success) {
            ofstream wout(walletsFile);
            for (const auto& w : wallets) {
                wout << w.first << "|" << w.second << "\n";
            }
            wout.close();

            int newTxId = 1;
            ifstream txin(walletTransactionsFile);
            while (getline(txin, line)) {
                vector<string> parts = split(line, '|');
                if (!parts.empty()) {
                    int id = atoi(parts[0].c_str());
                    if (id >= newTxId) newTxId = id + 1;
                }
            }
            txin.close();

            ofstream txout(walletTransactionsFile, ios::app);
            txout << newTxId << "|" << customerId << "|PAYMENT|" << amount << "|" << desc << "|" << getCurrentTimeStr() << "\n";
            txout.close();
            return true;
        }
        return false;
    }

    // =================================================================
    // 4. ORDERS & TRACKING
    // =================================================================

    int placeOrder(int customerId, const string& itemsString, double totalAmount, const string& paymentMethod) {
        if (paymentMethod == "Wallet") {
            if (!deductWalletBalance(customerId, totalAmount, "Order checkout payment")) {
                return -1; // Insufficient balance
            }
        }

        string curTime = getCurrentTimeStr();

#if USE_MYSQL
        if (isConnected) {
            stringstream orderQuery;
            orderQuery << "INSERT INTO orders (customer_id, total_amount, status, preparation_time, payment_method) VALUES ("
                       << customerId << ", " << totalAmount << ", 'Order Received', 15, '" << paymentMethod << "')";
            
            if (mysql_query(conn, orderQuery.str().c_str()) == 0) {
                int orderId = mysql_insert_id(conn);
                
                vector<string> itemsList = split(itemsString, ',');
                for (const auto& itemPair : itemsList) {
                    vector<string> parts = split(itemPair, ':');
                    if (parts.size() == 2) {
                        int itemId = atoi(parts[0].c_str());
                        int qty = atoi(parts[1].c_str());
                        stringstream itemQuery;
                        itemQuery << "INSERT INTO order_items (order_id, menu_item_id, quantity) VALUES ("
                                  << orderId << ", " << itemId << ", " << qty << ")";
                        mysql_query(conn, itemQuery.str().c_str());
                    }
                }

                createNotification(customerId, "Your order #" + to_string(orderId) + " has been received! Payment: " + paymentMethod);
                return orderId;
            }
            return -1;
        }
#endif
        // Fallback to File Handling
        int newOrderId = 1;
        ifstream oin(ordersFile);
        string line;
        while (getline(oin, line)) {
            vector<string> parts = split(line, '|');
            if (!parts.empty()) {
                int id = atoi(parts[0].c_str());
                if (id >= newOrderId) newOrderId = id + 1;
            }
        }
        oin.close();

        ofstream oout(ordersFile, ios::app);
        if (oout.is_open()) {
            oout << newOrderId << "|" << customerId << "|" << totalAmount << "|Order Received|15|" << curTime << "|" << itemsString << "|" << paymentMethod << "\n";
            oout.close();

            stringstream msg;
            msg << "Your order #" << newOrderId << " has been received! Payment Mode: " << paymentMethod << " | Total: $" << fixed << setprecision(2) << totalAmount;
            createNotification(customerId, msg.str());

            return newOrderId;
        }
        return -1;
    }

    Order getOrder(int orderId) {
        Order order;

#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "SELECT * FROM orders WHERE id = " << orderId;
            if (mysql_query(conn, query.str().c_str()) == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (res && mysql_num_rows(res) > 0) {
                    MYSQL_ROW row = mysql_fetch_row(res);
                    order.setOrderId(atoi(row[0]));
                    order.setCustomerId(atoi(row[1]));
                    order.setTotalAmount(atof(row[2]));
                    order.setStatus(row[3]);
                    order.setPreparationTime(atoi(row[4]));
                    order.setOrderTime(row[5]);
                    order.setPaymentMethod(row[6] ? row[6] : "Wallet");
                    
                    mysql_free_result(res);

                    stringstream itemsQuery;
                    itemsQuery << "SELECT menu_item_id, quantity FROM order_items WHERE order_id = " << orderId;
                    if (mysql_query(conn, itemsQuery.str().c_str()) == 0) {
                        MYSQL_RES* itemRes = mysql_store_result(conn);
                        if (itemRes) {
                            MYSQL_ROW iRow;
                            while ((iRow = mysql_fetch_row(itemRes))) {
                                order.addItem(atoi(iRow[0]), atoi(iRow[1]));
                            }
                            mysql_free_result(itemRes);
                        }
                    }
                } else if (res) {
                    mysql_free_result(res);
                }
            }
            return order;
        }
#endif
        // Fallback to File Handling
        ifstream oin(ordersFile);
        string line;
        while (getline(oin, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() >= 7 && atoi(parts[0].c_str()) == orderId) {
                order.setOrderId(orderId);
                order.setCustomerId(atoi(parts[1].c_str()));
                order.setTotalAmount(atof(parts[2].c_str()));
                order.setStatus(parts[3]);
                order.setPreparationTime(atoi(parts[4].c_str()));
                order.setOrderTime(parts[5]);
                if (parts.size() >= 8) {
                    order.setPaymentMethod(parts[7]);
                } else {
                    order.setPaymentMethod("Wallet");
                }

                vector<string> itemsList = split(parts[6], ',');
                for (const auto& itemPair : itemsList) {
                    vector<string> itemParts = split(itemPair, ':');
                    if (itemParts.size() == 2) {
                        order.addItem(atoi(itemParts[0].c_str()), atoi(itemParts[1].c_str()));
                    }
                }
                break;
            }
        }
        oin.close();
        return order;
    }

    vector<Order> getCustomerOrders(int customerId) {
        vector<Order> customerOrders;

#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "SELECT id FROM orders WHERE customer_id = " << customerId << " ORDER BY id DESC";
            if (mysql_query(conn, query.str().c_str()) == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (res) {
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(res))) {
                        int orderId = atoi(row[0]);
                        customerOrders.push_back(getOrder(orderId));
                    }
                    mysql_free_result(res);
                }
            }
            return customerOrders;
        }
#endif
        ifstream oin(ordersFile);
        string line;
        while (getline(oin, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() >= 7 && atoi(parts[1].c_str()) == customerId) {
                int orderId = atoi(parts[0].c_str());
                customerOrders.push_back(getOrder(orderId));
            }
        }
        oin.close();
        reverse(customerOrders.begin(), customerOrders.end());
        return customerOrders;
    }

    vector<Order> getAllOrders() {
        vector<Order> allOrders;

#if USE_MYSQL
        if (isConnected) {
            if (mysql_query(conn, "SELECT id FROM orders ORDER BY id DESC") == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (res) {
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(res))) {
                        int orderId = atoi(row[0]);
                        allOrders.push_back(getOrder(orderId));
                    }
                    mysql_free_result(res);
                }
            }
            return allOrders;
        }
#endif
        ifstream oin(ordersFile);
        string line;
        while (getline(oin, line)) {
            vector<string> parts = split(line, '|');
            if (!parts.empty()) {
                int orderId = atoi(parts[0].c_str());
                allOrders.push_back(getOrder(orderId));
            }
        }
        oin.close();
        reverse(allOrders.begin(), allOrders.end());
        return allOrders;
    }

    bool updateOrderStatus(int orderId, const string& newStatus) {
#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "UPDATE orders SET status = '" << newStatus << "' WHERE id = " << orderId;
            if (mysql_query(conn, query.str().c_str()) == 0) {
                Order order = getOrder(orderId);
                
                string alertMsg = "Your order #" + to_string(orderId) + " status updated: " + newStatus;
                if (newStatus == "Preparing") {
                    alertMsg = "Your order #" + to_string(orderId) + " is being prepared by our chefs.";
                } else if (newStatus == "Ready") {
                    alertMsg = "Your order #" + to_string(orderId) + " is ready! Please collect it.";
                } else if (newStatus == "Dispatched") {
                    alertMsg = "Your order #" + to_string(orderId) + " has been dispatched and is on the way!";
                } else if (newStatus == "Completed") {
                    alertMsg = "Your order #" + to_string(orderId) + " has been completed. Enjoy your meal!";
                }
                createNotification(order.getCustomerId(), alertMsg);
                return true;
            }
            return false;
        }
#endif
        ifstream oin(ordersFile);
        string line;
        vector<string> fileLines;
        bool found = false;
        int custId = 0;
        while (getline(oin, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() >= 7 && atoi(parts[0].c_str()) == orderId) {
                parts[3] = newStatus;
                custId = atoi(parts[1].c_str());
                found = true;
                
                stringstream rebuilt;
                rebuilt << parts[0] << "|" << parts[1] << "|" << parts[2] << "|" 
                        << parts[3] << "|" << parts[4] << "|" << parts[5] << "|" << parts[6];
                if (parts.size() >= 8) {
                    rebuilt << "|" << parts[7];
                } else {
                    rebuilt << "|Wallet";
                }
                fileLines.push_back(rebuilt.str());
            } else {
                fileLines.push_back(line);
            }
        }
        oin.close();

        if (found) {
            ofstream oout(ordersFile);
            for (const auto& fl : fileLines) {
                oout << fl << "\n";
            }
            oout.close();

            string alertMsg = "Your order #" + to_string(orderId) + " status updated: " + newStatus;
            if (newStatus == "Preparing") {
                alertMsg = "Your order #" + to_string(orderId) + " is being prepared. Estimated time: " + to_string(getOrder(orderId).getPreparationTime()) + " mins.";
            } else if (newStatus == "Ready") {
                alertMsg = "Your order #" + to_string(orderId) + " is ready! Pop up and collect it.";
            } else if (newStatus == "Dispatched") {
                alertMsg = "Your order #" + to_string(orderId) + " is dispatched! Delivery rider is on the way.";
            } else if (newStatus == "Completed") {
                alertMsg = "Your order #" + to_string(orderId) + " is completed. Thank you for dining with STC!";
            }
            createNotification(custId, alertMsg);
            return true;
        }
        return false;
    }

    bool updateOrderPreparationTime(int orderId, int mins) {
#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "UPDATE orders SET preparation_time = " << mins << " WHERE id = " << orderId;
            if (mysql_query(conn, query.str().c_str()) == 0) {
                Order order = getOrder(orderId);
                createNotification(order.getCustomerId(), "Order #" + to_string(orderId) + " remaining preparation time: " + to_string(mins) + " minutes.");
                return true;
            }
            return false;
        }
#endif
        ifstream oin(ordersFile);
        string line;
        vector<string> fileLines;
        bool found = false;
        int custId = 0;
        while (getline(oin, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() >= 7 && atoi(parts[0].c_str()) == orderId) {
                parts[4] = to_string(mins);
                custId = atoi(parts[1].c_str());
                found = true;
                
                stringstream rebuilt;
                rebuilt << parts[0] << "|" << parts[1] << "|" << parts[2] << "|" 
                        << parts[3] << "|" << parts[4] << "|" << parts[5] << "|" << parts[6];
                if (parts.size() >= 8) {
                    rebuilt << "|" << parts[7];
                } else {
                    rebuilt << "|Wallet";
                }
                fileLines.push_back(rebuilt.str());
            } else {
                fileLines.push_back(line);
            }
        }
        oin.close();

        if (found) {
            ofstream oout(ordersFile);
            for (const auto& fl : fileLines) {
                oout << fl << "\n";
            }
            oout.close();

            createNotification(custId, "Order #" + to_string(orderId) + " remaining preparation time: " + to_string(mins) + " minutes.");
            return true;
        }
        return false;
    }

    // =================================================================
    // 5. NOTIFICATION SYSTEM
    // =================================================================

    bool createNotification(int customerId, const string& message) {
        string curTime = getCurrentTimeStr();

#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "INSERT INTO notifications (customer_id, message, is_read) VALUES ("
                  << customerId << ", '" << message << "', 0)";
            return mysql_query(conn, query.str().c_str()) == 0;
        }
#endif
        int newNotifId = 1;
        ifstream nin(notificationsFile);
        string line;
        while (getline(nin, line)) {
            vector<string> parts = split(line, '|');
            if (!parts.empty()) {
                int id = atoi(parts[0].c_str());
                if (id >= newNotifId) newNotifId = id + 1;
            }
        }
        nin.close();

        ofstream nout(notificationsFile, ios::app);
        if (nout.is_open()) {
            nout << newNotifId << "|" << customerId << "|" << message << "|" << curTime << "|0\n";
            nout.close();
            return true;
        }
        return false;
    }

    vector<Notification> getNotifications(int customerId) {
        vector<Notification> list;

#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "SELECT * FROM notifications WHERE customer_id = " << customerId << " ORDER BY id DESC";
            if (mysql_query(conn, query.str().c_str()) == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (res) {
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(res))) {
                        int id = atoi(row[0]);
                        string msg = row[2];
                        string timeStr = row[3];
                        bool isRead = atoi(row[4]) != 0;
                        list.push_back(Notification(id, customerId, msg, timeStr, isRead));
                    }
                    mysql_free_result(res);
                }
            }
            return list;
        }
#endif
        ifstream nin(notificationsFile);
        string line;
        while (getline(nin, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() >= 5 && atoi(parts[1].c_str()) == customerId) {
                int id = atoi(parts[0].c_str());
                string msg = parts[2];
                string timeStr = parts[3];
                bool isRead = parts[4] == "1" || parts[4] == "true";
                list.push_back(Notification(id, customerId, msg, timeStr, isRead));
            }
        }
        nin.close();
        reverse(list.begin(), list.end());
        return list;
    }

    bool markNotificationsRead(int customerId) {
#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "UPDATE notifications SET is_read = 1 WHERE customer_id = " << customerId;
            return mysql_query(conn, query.str().c_str()) == 0;
        }
#endif
        ifstream nin(notificationsFile);
        string line;
        vector<string> fileLines;
        bool found = false;
        while (getline(nin, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() >= 5 && atoi(parts[1].c_str()) == customerId) {
                parts[4] = "1";
                found = true;
                stringstream rebuilt;
                rebuilt << parts[0] << "|" << parts[1] << "|" << parts[2] << "|" << parts[3] << "|" << parts[4];
                fileLines.push_back(rebuilt.str());
            } else {
                fileLines.push_back(line);
            }
        }
        nin.close();

        if (found) {
            ofstream nout(notificationsFile);
            for (const auto& fl : fileLines) {
                nout << fl << "\n";
            }
            nout.close(); // Fixed typo oout.close() -> nout.close()
            return true;
        }
        return false;
    }

    // =================================================================
    // 6. CUSTOMER REGISTRY
    // =================================================================

    vector<Customer> getAllCustomers() {
        vector<Customer> customersList;

#if USE_MYSQL
        if (isConnected) {
            if (mysql_query(conn, "SELECT id, username, email, phone, address FROM users WHERE role = 'customer'") == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (res) {
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(res))) {
                        int id = atoi(row[0]);
                        string name = row[1];
                        string email = row[2];
                        string phone = row[3] ? row[3] : "";
                        string address = row[4] ? row[4] : "";
                        
                        double balance = 0.0;
                        stringstream wQuery;
                        wQuery << "SELECT balance FROM wallets WHERE customer_id = " << id;
                        mysql_query(conn, wQuery.str().c_str());
                        MYSQL_RES* wRes = mysql_store_result(conn);
                        if (wRes && mysql_num_rows(wRes) > 0) {
                            MYSQL_ROW wRow = mysql_fetch_row(wRes);
                            balance = atof(wRow[0]);
                        }
                        if (wRes) mysql_free_result(wRes);

                        customersList.push_back(Customer(id, name, "", email, phone, address, balance));
                    }
                    mysql_free_result(res);
                }
            }
            return customersList;
        }
#endif
        ifstream in(usersFile);
        string line;
        while (getline(in, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() >= 5 && parts[4] == "customer") {
                int id = atoi(parts[0].c_str());
                string name = parts[1];
                string email = parts[3];
                string phone = parts.size() >= 6 ? parts[5] : "";
                string address = parts.size() >= 7 ? parts[6] : "";

                double balance = 0.0;
                ifstream win(walletsFile);
                string wline;
                while (getline(win, wline)) {
                    vector<string> wparts = split(wline, '|');
                    if (wparts.size() >= 2 && atoi(wparts[0].c_str()) == id) {
                        balance = atof(wparts[1].c_str());
                        break;
                    }
                }
                win.close();

                customersList.push_back(Customer(id, name, "", email, phone, address, balance));
            }
        }
        in.close();
        return customersList;
    }

    // =================================================================
    // 7. PRODUCT REVIEWS & RATINGS SYSTEM
    // =================================================================

    vector<Review> getReviews(int menuItemId) {
        vector<Review> list;
#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "SELECT * FROM reviews WHERE menu_item_id = " << menuItemId << " ORDER BY id DESC";
            if (mysql_query(conn, query.str().c_str()) == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (res) {
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(res))) {
                        int id = atoi(row[0]);
                        int custId = atoi(row[1]);
                        int mId = atoi(row[2]);
                        int rating = atoi(row[3]);
                        string comment = row[4] ? row[4] : "";
                        string custName = row[5];
                        string timestamp = row[6];
                        list.push_back(Review(id, custId, mId, rating, comment, custName, timestamp));
                    }
                    mysql_free_result(res);
                }
            }
            return list;
        }
#endif
        ifstream rin(reviewsFile);
        string line;
        while (getline(rin, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() >= 7 && atoi(parts[2].c_str()) == menuItemId) {
                list.push_back(Review(
                    atoi(parts[0].c_str()),
                    atoi(parts[1].c_str()),
                    menuItemId,
                    atoi(parts[3].c_str()),
                    parts[4],
                    parts[5],
                    parts[6]
                ));
            }
        }
        rin.close();
        reverse(list.begin(), list.end());
        return list;
    }

    vector<Review> getAllReviews() {
        vector<Review> list;
#if USE_MYSQL
        if (isConnected) {
            if (mysql_query(conn, "SELECT * FROM reviews ORDER BY id DESC") == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (res) {
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(res))) {
                        list.push_back(Review(atoi(row[0]), atoi(row[1]), atoi(row[2]), atoi(row[3]), row[4] ? row[4] : "", row[5], row[6]));
                    }
                    mysql_free_result(res);
                }
            }
            return list;
        }
#endif
        ifstream rin(reviewsFile);
        string line;
        while (getline(rin, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() >= 7) {
                list.push_back(Review(
                    atoi(parts[0].c_str()),
                    atoi(parts[1].c_str()),
                    atoi(parts[2].c_str()),
                    atoi(parts[3].c_str()),
                    parts[4],
                    parts[5],
                    parts[6]
                ));
            }
        }
        rin.close();
        reverse(list.begin(), list.end());
        return list;
    }

    bool addReview(int customerId, int menuItemId, int rating, const string& comment) {
        string custName = "Customer";
        
        // Find username
        ifstream uin(usersFile);
        string uline;
        while (getline(uin, uline)) {
            vector<string> parts = split(uline, '|');
            if (parts.size() >= 2 && atoi(parts[0].c_str()) == customerId) {
                custName = parts[1];
                break;
            }
        }
        uin.close();

        string curTime = getCurrentTimeStr();

#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "INSERT INTO reviews (customer_id, menu_item_id, rating, comment, customer_name) VALUES ("
                  << customerId << ", " << menuItemId << ", " << rating << ", '" << comment << "', '" << custName << "')";
            
            if (mysql_query(conn, query.str().c_str()) == 0) {
                // Trigger dynamic recalculation of MenuItem rating & count
                stringstream updateQuery;
                updateQuery << "UPDATE menu SET "
                            << "rating = (SELECT AVG(rating) FROM reviews WHERE menu_item_id = " << menuItemId << "), "
                            << "reviews_count = (SELECT COUNT(*) FROM reviews WHERE menu_item_id = " << menuItemId << ") "
                            << "WHERE id = " << menuItemId;
                mysql_query(conn, updateQuery.str().c_str());
                return true;
            }
            return false;
        }
#endif
        // Fallback to File Handling
        int newId = 1;
        ifstream rin(reviewsFile);
        string line;
        while (getline(rin, line)) {
            vector<string> parts = split(line, '|');
            if (!parts.empty()) {
                int id = atoi(parts[0].c_str());
                if (id >= newId) newId = id + 1;
            }
        }
        rin.close();

        ofstream rout(reviewsFile, ios::app);
        if (rout.is_open()) {
            rout << newId << "|" << customerId << "|" << menuItemId << "|" << rating << "|" << comment << "|" << custName << "|" << curTime << "\n";
            rout.close();

            // Dynamic recalculation of reviews for menu item
            vector<Review> reviews = getReviews(menuItemId);
            double sum = 0;
            for (const auto& r : reviews) {
                sum += r.getRating();
            }
            double avgRating = reviews.empty() ? 5.0 : (sum / reviews.size());
            int totalReviews = reviews.size();

            vector<MenuItem> menu = getMenu();
            ofstream out(menuFile);
            for (auto& item : menu) {
                if (item.getId() == menuItemId) {
                    item.setRating(avgRating);
                    item.setReviewsCount(totalReviews);
                }
                out << item.getId() << "|"
                    << item.getName() << "|"
                    << item.getDescription() << "|"
                    << item.getPrice() << "|"
                    << item.getCategory() << "|"
                    << (item.getIsSoldOut() ? "1" : "0") << "|"
                    << fixed << setprecision(1) << item.getRating() << "|"
                    << item.getReviewsCount() << "\n";
            }
            out.close();
            return true;
        }
        return false;
    }

    // =================================================================
    // 8. FAVORITES SYSTEM
    // =================================================================

    bool toggleFavorite(int customerId, int menuItemId) {
#if USE_MYSQL
        if (isConnected) {
            stringstream checkQuery;
            checkQuery << "SELECT * FROM favorites WHERE customer_id = " << customerId << " AND menu_item_id = " << menuItemId;
            if (mysql_query(conn, checkQuery.str().c_str()) == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (res && mysql_num_rows(res) > 0) {
                    mysql_free_result(res);
                    stringstream deleteQuery;
                    deleteQuery << "DELETE FROM favorites WHERE customer_id = " << customerId << " AND menu_item_id = " << menuItemId;
                    mysql_query(conn, deleteQuery.str().c_str());
                    return false; // Removed
                }
                if (res) mysql_free_result(res);
            }
            stringstream insertQuery;
            insertQuery << "INSERT INTO favorites (customer_id, menu_item_id) VALUES (" << customerId << ", " << menuItemId << ")";
            mysql_query(conn, insertQuery.str().c_str());
            return true; // Added
        }
#endif
        ifstream fin(favoritesFile);
        string line;
        vector<pair<int, int>> favorites;
        bool found = false;
        while (getline(fin, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() >= 2) {
                int cId = atoi(parts[0].c_str());
                int mId = atoi(parts[1].c_str());
                if (cId == customerId && mId == menuItemId) {
                    found = true; // Exists, so we filter it out (toggle remove)
                    continue;
                }
                favorites.push_back(make_pair(cId, mId));
            }
        }
        fin.close();

        if (!found) {
            favorites.push_back(make_pair(customerId, menuItemId));
        }

        ofstream fout(favoritesFile);
        for (const auto& fav : favorites) {
            fout << fav.first << "|" << fav.second << "\n";
        }
        fout.close();
        return !found; // Returns true if added, false if removed
    }

    vector<int> getFavorites(int customerId) {
        vector<int> list;
#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "SELECT menu_item_id FROM favorites WHERE customer_id = " << customerId;
            if (mysql_query(conn, query.str().c_str()) == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (res) {
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(res))) {
                        list.push_back(atoi(row[0]));
                    }
                    mysql_free_result(res);
                }
            }
            return list;
        }
#endif
        ifstream fin(favoritesFile);
        string line;
        while (getline(fin, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() >= 2 && atoi(parts[0].c_str()) == customerId) {
                list.push_back(atoi(parts[1].c_str()));
            }
        }
        fin.close();
        return list;
    }

    // =================================================================
    // 9. ANNOUNCEMENT SYSTEM
    // =================================================================

    vector<Announcement> getAnnouncements() {
        vector<Announcement> list;
#if USE_MYSQL
        if (isConnected) {
            if (mysql_query(conn, "SELECT * FROM announcements ORDER BY id DESC") == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (res) {
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(res))) {
                        list.push_back(Announcement(atoi(row[0]), row[1], row[2]));
                    }
                    mysql_free_result(res);
                }
            }
            return list;
        }
#endif
        ifstream ain(announcementsFile);
        string line;
        while (getline(ain, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() >= 3) {
                list.push_back(Announcement(
                    atoi(parts[0].c_str()),
                    parts[1],
                    parts[2]
                ));
            }
        }
        ain.close();
        reverse(list.begin(), list.end());
        return list;
    }

    bool addAnnouncement(const string& message) {
        string curTime = getCurrentTimeStr();
#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "INSERT INTO announcements (message) VALUES ('" << message << "')";
            return mysql_query(conn, query.str().c_str()) == 0;
        }
#endif
        int newId = 1;
        ifstream ain(announcementsFile);
        string line;
        while (getline(ain, line)) {
            vector<string> parts = split(line, '|');
            if (!parts.empty()) {
                int id = atoi(parts[0].c_str());
                if (id >= newId) newId = id + 1;
            }
        }
        ain.close();

        ofstream aout(announcementsFile, ios::app);
        if (aout.is_open()) {
            aout << newId << "|" << message << "|" << curTime << "\n";
            aout.close();

            // Broad-cast notification to all users
            vector<Customer> custs = getAllCustomers();
            for (const auto& c : custs) {
                createNotification(c.getId(), "📢 ANNOUNCEMENT: " + message);
            }
            return true;
        }
        return false;
    }

    // =================================================================
    // 10. COUPONS & DISCOUNTS SYSTEM
    // =================================================================

    vector<Coupon> getCoupons() {
        vector<Coupon> list;
#if USE_MYSQL
        if (isConnected) {
            if (mysql_query(conn, "SELECT * FROM coupons") == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (res) {
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(res))) {
                        list.push_back(Coupon(row[0], atof(row[1]), atoi(row[2]) != 0));
                    }
                    mysql_free_result(res);
                }
            }
            return list;
        }
#endif
        ifstream cin(couponsFile);
        string line;
        while (getline(cin, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() >= 3) {
                list.push_back(Coupon(
                    parts[0],
                    atof(parts[1].c_str()),
                    parts[2] == "1" || parts[2] == "true"
                ));
            }
        }
        cin.close();
        return list;
    }

    bool addCoupon(const string& code, double discountPercent) {
#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "INSERT INTO coupons (code, discount_percent, is_active) VALUES ('" << code << "', " << discountPercent << ", 1)";
            return mysql_query(conn, query.str().c_str()) == 0;
        }
#endif
        vector<Coupon> coupons = getCoupons();
        for (const auto& cp : coupons) {
            if (cp.getCode() == code) return false; // Code exists
        }

        ofstream cout(couponsFile, ios::app);
        if (cout.is_open()) {
            cout << code << "|" << discountPercent << "|1\n";
            cout.close();
            return true;
        }
        return false;
    }

    bool toggleCoupon(const string& code, bool active) {
#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "UPDATE coupons SET is_active = " << (active ? "1" : "0") << " WHERE code = '" << code << "'";
            return mysql_query(conn, query.str().c_str()) == 0;
        }
#endif
        vector<Coupon> coupons = getCoupons();
        bool found = false;
        ofstream cout(couponsFile);
        for (auto& cp : coupons) {
            if (cp.getCode() == code) {
                cp.setIsActive(active);
                found = true;
            }
            cout << cp.getCode() << "|" << cp.getDiscountPercent() << "|" << (cp.getIsActive() ? "1" : "0") << "\n";
        }
        cout.close();
        return found;
    }

    // =================================================================
    // 11. WEBPAGE HERO BANNERS MANAGEMENT
    // =================================================================

    vector<Banner> getBanners() {
        vector<Banner> list;
#if USE_MYSQL
        if (isConnected) {
            if (mysql_query(conn, "SELECT * FROM banners WHERE is_active = 1") == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (res) {
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(res))) {
                        list.push_back(Banner(atoi(row[0]), row[1], row[2], atoi(row[3]) != 0));
                    }
                    mysql_free_result(res);
                }
            }
            return list;
        }
#endif
        ifstream bin(bannersFile);
        string line;
        while (getline(bin, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() >= 4) {
                list.push_back(Banner(
                    atoi(parts[0].c_str()),
                    parts[1],
                    parts[2],
                    parts[3] == "1" || parts[3] == "true"
                ));
            }
        }
        bin.close();
        return list;
    }

    bool updateBanner(const string& text, const string& imageUrl) {
#if USE_MYSQL
        if (isConnected) {
            stringstream query;
            query << "UPDATE banners SET banner_text = '" << text << "', banner_image_url = '" << imageUrl << "' WHERE id = 1";
            return mysql_query(conn, query.str().c_str()) == 0;
        }
#endif
        ofstream bout(bannersFile); // Overwrite first banner settings
        if (bout.is_open()) {
            bout << "1|" << text << "|" << imageUrl << "|1\n";
            bout.close();
            return true;
        }
        return false;
    }

    // =================================================================
    // 12. SALES ANALYTICS & DAILY REPORTS
    // =================================================================

    string getSalesAnalytics() {
        vector<Order> orders = getAllOrders();
        vector<Customer> custs = getAllCustomers();

        double totalRevenue = 0.0;
        int totalOrdersCount = orders.size();
        int activeCustomersCount = custs.size();

        // Accumulate sales by category
        map<string, double> categorySales;
        categorySales["Burgers"] = 0.0;
        categorySales["Pizza"] = 0.0;
        categorySales["Sides"] = 0.0;
        categorySales["Drinks"] = 0.0;
        categorySales["Desserts"] = 0.0;

        // Daily sales graph map
        map<string, double> dailySales;

        // Load complete menu details to cross-reference categories
        vector<MenuItem> menu = getMenu();
        map<int, MenuItem> menuMap;
        for (const auto& item : menu) {
            menuMap[item.getId()] = item;
        }

        for (const auto& o : orders) {
            totalRevenue += o.getTotalAmount();

            // Group by daily dates: timestamp format "YYYY-MM-DD HH:MM:SS"
            string dateStr = "2026-05-29"; // Default fallback
            if (o.getOrderTime().length() >= 10) {
                dateStr = o.getOrderTime().substr(0, 10);
            }
            dailySales[dateStr] += o.getTotalAmount();

            // Map item categories
            for (const auto& pair : o.getItems()) {
                int itemId = pair.first;
                int qty = pair.second;
                if (menuMap.find(itemId) != menuMap.end()) {
                    MenuItem item = menuMap[itemId];
                    categorySales[item.getCategory()] += (item.getPrice() * qty);
                }
            }
        }

        // Convert mappings to JSON structures
        stringstream ss;
        ss << "{"
           << "\"total_revenue\":" << fixed << setprecision(2) << totalRevenue << ","
           << "\"total_orders\":" << totalOrdersCount << ","
           << "\"total_customers\":" << activeCustomersCount << ","
           << "\"category_sales\":{";
        
        size_t count = 0;
        for (auto const& x : categorySales) {
            ss << "\"" << x.first << "\":" << fixed << setprecision(2) << x.second;
            if (++count < categorySales.size()) ss << ",";
        }
        ss << "},\"daily_sales\":[";

        size_t dCount = 0;
        for (auto const& d : dailySales) {
            ss << "{\"date\":\"" << d.first << "\",\"revenue\":" << fixed << setprecision(2) << d.second << "}";
            if (++dCount < dailySales.size()) ss << ",";
        }
        ss << "]}";

        return ss.str();
    }
};

#endif // DATABASE_H
