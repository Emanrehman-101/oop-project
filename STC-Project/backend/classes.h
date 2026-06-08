#ifndef CLASSES_H
#define CLASSES_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>

using namespace std;

// =================================================================
// 1. WALLET CLASS (Encapsulation)
// =================================================================
class Wallet {
private:
    double balance;                          // Encapsulated variable (private)
    vector<string> transactionHistory;       // List of transactions using STL vector

public:
    // Default Constructor
    Wallet() {
        balance = 100.0; // Gift starting balance of $100 for test users
        transactionHistory.push_back("Welcome Gift: Deposit $100.00");
    }

    // Parameterized Constructor
    Wallet(double initialBalance) {
        balance = initialBalance;
        stringstream ss;
        ss << "Wallet initialized with: $" << fixed << setprecision(2) << initialBalance;
        transactionHistory.push_back(ss.str());
    }

    // Destructor
    ~Wallet() {
        // Destructor to clean up resources (if any)
    }

    // Method to add money to wallet
    void addMoney(double amount, const string& description = "Money Loaded") {
        if (amount > 0) {
            balance += amount;
            stringstream ss;
            ss << description << ": +$" << fixed << setprecision(2) << amount;
            transactionHistory.push_back(ss.str());
        }
    }

    // Method to deduct money from wallet
    bool deductMoney(double amount, const string& description = "Payment Deducted") {
        if (amount > 0 && balance >= amount) {
            balance -= amount;
            stringstream ss;
            ss << description << ": -$" << fixed << setprecision(2) << amount;
            transactionHistory.push_back(ss.str());
            return true; // Deduction successful
        }
        return false; // Insufficient balance or invalid amount
    }

    // Getter for balance
    double getBalance() const {
        return balance;
    }

    // Getter for transaction history
    const vector<string>& getTransactionHistory() const {
        return transactionHistory;
    }

    // Method to set transaction history (helpful during database loading)
    void setTransactionHistory(const vector<string>& history) {
        transactionHistory = history;
    }

    // Helper to add manual raw transaction log (database loader)
    void addTransactionLog(const string& log) {
        transactionHistory.push_back(log);
    }

    // Helper to generate JSON string of the wallet
    string toJSON() const {
        stringstream ss;
        ss << "{"
           << "\"balance\":" << fixed << setprecision(2) << balance << ","
           << "\"transactions\":[";
        for (size_t i = 0; i < transactionHistory.size(); ++i) {
            ss << "\"" << transactionHistory[i] << "\"";
            if (i < transactionHistory.size() - 1) {
                ss << ",";
            }
        }
        ss << "]}";
        return ss.str();
    }
};

// =================================================================
// 2. USER CLASS (Abstraction & Inheritance Base Class)
// =================================================================
class User {
protected:
    int id;               // Encapsulated data (accessible by child classes)
    string username;
    string password;
    string email;
    string role;          // "customer" or "admin"

public:
    // Default Constructor
    User() {
        id = 0;
        username = "";
        password = "";
        email = "";
        role = "";
    }

    // Parameterized Constructor
    User(int uId, const string& uName, const string& uPass, const string& uEmail, const string& uRole) {
        id = uId;
        username = uName;
        password = uPass;
        email = uEmail;
        role = uRole;
    }

    // Virtual Destructor (essential for polymorphism)
    virtual ~User() {
        // Virtual destructor ensures proper cleanup of derived classes
    }

    // Getters and Setters
    int getId() const { return id; }
    void setId(int uId) { id = uId; }

    string getUsername() const { return username; }
    void setUsername(const string& name) { username = name; }

    string getPassword() const { return password; }
    void setPassword(const string& pass) { password = pass; }

    string getEmail() const { return email; }
    void setEmail(const string& mail) { email = mail; }

    // Pure Virtual Function (Abstraction)
    // Makes User an abstract base class. Derived classes MUST implement this.
    virtual string getRole() const = 0;

    // Virtual method for JSON representation
    virtual string toJSON() const {
        stringstream ss;
        ss << "{"
           << "\"id\":" << id << ","
           << "\"username\":\"" << username << "\","
           << "\"email\":\"" << email << "\","
           << "\"role\":\"" << role << "\""
           << "}";
        return ss.str();
    }
};

// =================================================================
// 3. CUSTOMER CLASS (Inheritance from User)
// =================================================================
class Customer : public User {
private:
    string phone;
    string address;
    Wallet wallet; // Composition: Customer has a Wallet object

public:
    // Default Constructor
    Customer() : User() {
        phone = "";
        address = "";
        role = "customer";
    }

    // Parameterized Constructor
    Customer(int uId, const string& uName, const string& uPass, const string& uEmail, 
             const string& uPhone, const string& uAddress, double walletBal = 100.0) 
        : User(uId, uName, uPass, uEmail, "customer") {
        phone = uPhone;
        address = uAddress;
        wallet = Wallet(walletBal);
    }

    // Destructor
    ~Customer() override {
        // Automatically calls base class destructor
    }

    // Getters and Setters
    string getPhone() const { return phone; }
    void setPhone(const string& ph) { phone = ph; }

    string getAddress() const { return address; }
    void setAddress(const string& addr) { address = addr; }

    // Access to internal wallet (reference allows modification)
    Wallet& getWallet() { return wallet; }
    const Wallet& getWalletConst() const { return wallet; }

    // Implementation of Pure Virtual Function (Polymorphism)
    string getRole() const override {
        return "Customer";
    }

    // Override toJSON to include Customer-specific fields (Polymorphism)
    string toJSON() const override {
        stringstream ss;
        ss << "{"
           << "\"id\":" << id << ","
           << "\"username\":\"" << username << "\","
           << "\"email\":\"" << email << "\","
           << "\"role\":\"customer\","
           << "\"phone\":\"" << phone << "\","
           << "\"address\":\"" << address << "\","
           << "\"wallet\":" << wallet.toJSON()
           << "}";
        return ss.str();
    }
};

// =================================================================
// 4. ADMIN CLASS (Inheritance from User)
// =================================================================
class Admin : public User {
public:
    // Default Constructor
    Admin() : User() {
        role = "admin";
    }

    // Parameterized Constructor
    Admin(int uId, const string& uName, const string& uPass, const string& uEmail)
        : User(uId, uName, uPass, uEmail, "admin") {}

    // Destructor
    ~Admin() override {}

    // Implementation of Pure Virtual Function (Polymorphism)
    string getRole() const override {
        return "Admin";
    }

    // Override toJSON (Polymorphism)
    string toJSON() const override {
        stringstream ss;
        ss << "{"
           << "\"id\":" << id << ","
           << "\"username\":\"" << username << "\","
           << "\"email\":\"" << email << "\","
           << "\"role\":\"admin\""
           << "}";
        return ss.str();
    }
};

// =================================================================
// 5. MENUITEM CLASS
// =================================================================
class MenuItem {
private:
    int id;
    string name;
    string description;
    double price;
    string category; // "Burgers", "Pizza", "Drinks", "Desserts", "Sides"
    bool isSoldOut;
    double rating;
    int reviewsCount;

public:
    // Constructors
    MenuItem() {
        id = 0;
        name = "";
        description = "";
        price = 0.0;
        category = "";
        isSoldOut = false;
        rating = 5.0;
        reviewsCount = 0;
    }

    MenuItem(int mId, const string& mName, const string& mDesc, double mPrice, 
             const string& mCat, bool mSold = false, double mRate = 5.0, int mRev = 0) {
        id = mId;
        name = mName;
        description = mDesc;
        price = mPrice;
        category = mCat;
        isSoldOut = mSold;
        rating = mRate;
        reviewsCount = mRev;
    }

    // Getters and Setters
    int getId() const { return id; }
    void setId(int mId) { id = mId; }

    string getName() const { return name; }
    void setName(const string& mName) { name = mName; }

    string getDescription() const { return description; }
    void setDescription(const string& mDesc) { description = mDesc; }

    double getPrice() const { return price; }
    void setPrice(double mPrice) { price = mPrice; }

    string getCategory() const { return category; }
    void setCategory(const string& mCat) { category = mCat; }

    bool getIsSoldOut() const { return isSoldOut; }
    void setIsSoldOut(bool mSold) { isSoldOut = mSold; }

    double getRating() const { return rating; }
    void setRating(double mRate) { rating = mRate; }

    int getReviewsCount() const { return reviewsCount; }
    void setReviewsCount(int mRev) { reviewsCount = mRev; }

    // JSON serialization helper
    string toJSON() const {
        stringstream ss;
        ss << "{"
           << "\"id\":" << id << ","
           << "\"name\":\"" << name << "\","
           << "\"description\":\"" << description << "\","
           << "\"price\":" << fixed << setprecision(2) << price << ","
           << "\"category\":\"" << category << "\","
           << "\"is_sold_out\":" << (isSoldOut ? "true" : "false") << ","
           << "\"rating\":" << fixed << setprecision(1) << rating << ","
           << "\"reviews_count\":" << reviewsCount
           << "}";
        return ss.str();
    }
};

// =================================================================
// 6. CART CLASS
// =================================================================
class Cart {
private:
    int customerId;
    map<int, int> items; // Map: key = MenuItemID, value = Quantity

public:
    // Constructors
    Cart() {
        customerId = 0;
    }

    Cart(int custId) {
        customerId = custId;
    }

    // Add item to cart
    void addItem(int itemId, int quantity = 1) {
        if (quantity > 0) {
            items[itemId] += quantity; // Adds to existing, or initializes to quantity
        }
    }

    // Remove or reduce item from cart
    void removeItem(int itemId, int quantity = 1) {
        if (items.find(itemId) != items.end()) {
            items[itemId] -= quantity;
            if (items[itemId] <= 0) {
                items.erase(itemId); // Remove item completely if count is 0 or less
            }
        }
    }

    // Clear entire cart
    void clear() {
        items.clear();
    }

    // Getters
    int getCustomerId() const { return customerId; }
    const map<int, int>& getItems() const { return items; }

    // Check if cart is empty
    bool isEmpty() const { return items.empty(); }

    // JSON serialization
    string toJSON() const {
        stringstream ss;
        ss << "{"
           << "\"customer_id\":" << customerId << ","
           << "\"items\":[";
        
        map<int, int>::const_iterator it;
        size_t count = 0;
        for (it = items.begin(); it != items.end(); ++it) {
            ss << "{\"item_id\":" << it->first << ",\"quantity\":" << it->second << "}";
            if (++count < items.size()) {
                ss << ",";
            }
        }
        ss << "]}";
        return ss.str();
    }
};

// =================================================================
// 7. ORDER CLASS
// =================================================================
class Order {
private:
    int orderId;
    int customerId;
    vector<pair<int, int>> items; // Pair: <MenuItemID, Quantity>
    double totalAmount;
    string status;                // "Order Received", "Preparing", "Ready", "Dispatched", "Completed"
    int preparationTime;          // Estimation in minutes (e.g. 15)
    string orderTime;             // Timestamp string
    string paymentMethod;         // "Wallet", "QR", or "COD"

public:
    // Constructors
    Order() {
        orderId = 0;
        customerId = 0;
        totalAmount = 0.0;
        status = "Order Received";
        preparationTime = 15;
        orderTime = "";
        paymentMethod = "Wallet";
    }

    Order(int oId, int cId, double amt, const string& oStatus = "Order Received", int prepTime = 15, const string& timeStr = "", const string& payMethod = "Wallet") {
        orderId = oId;
        customerId = cId;
        totalAmount = amt;
        status = oStatus;
        preparationTime = prepTime;
        orderTime = timeStr;
        paymentMethod = payMethod;
    }

    // Getters and Setters
    int getOrderId() const { return orderId; }
    void setOrderId(int oId) { orderId = oId; }

    int getCustomerId() const { return customerId; }
    void setCustomerId(int cId) { customerId = cId; }

    const vector<pair<int, int>>& getItems() const { return items; }
    void setItems(const vector<pair<int, int>>& orderItems) { items = orderItems; }
    void addItem(int itemId, int quantity) {
        items.push_back(make_pair(itemId, quantity));
    }

    double getTotalAmount() const { return totalAmount; }
    void setTotalAmount(double amt) { totalAmount = amt; }

    string getStatus() const { return status; }
    void setStatus(const string& oStatus) { status = oStatus; }

    int getPreparationTime() const { return preparationTime; }
    void setPreparationTime(int mins) { preparationTime = mins; }

    string getOrderTime() const { return orderTime; }
    void setOrderTime(const string& timeStr) { orderTime = timeStr; }

    string getPaymentMethod() const { return paymentMethod; }
    void setPaymentMethod(const string& payMethod) { paymentMethod = payMethod; }

    // Check progress as a percentage for progress bar
    int getProgressPercentage() const {
        if (status == "Order Received") return 10;
        if (status == "Preparing") return 40;
        if (status == "Ready") return 70;
        if (status == "Dispatched") return 90;
        if (status == "Completed") return 100;
        return 0;
    }

    // JSON serialization (outputs full summary of the order details)
    string toJSON() const {
        stringstream ss;
        ss << "{"
           << "\"order_id\":" << orderId << ","
           << "\"customer_id\":" << customerId << ","
           << "\"total_amount\":" << fixed << setprecision(2) << totalAmount << ","
           << "\"status\":\"" << status << "\","
           << "\"progress\":" << getProgressPercentage() << ","
           << "\"preparation_time\":" << preparationTime << ","
           << "\"order_time\":\"" << orderTime << "\","
           << "\"payment_method\":\"" << paymentMethod << "\","
           << "\"items\":[";
        
        for (size_t i = 0; i < items.size(); ++i) {
            ss << "{\"item_id\":" << items[i].first << ",\"quantity\":" << items[i].second << "}";
            if (i < items.size() - 1) {
                ss << ",";
            }
        }
        ss << "]}";
        return ss.str();
    }
};

// =================================================================
// 8. NOTIFICATION CLASS
// =================================================================
class Notification {
private:
    int id;
    int customerId;
    string message;
    string timestamp;
    bool isRead;

public:
    // Constructors
    Notification() {
        id = 0;
        customerId = 0;
        message = "";
        timestamp = "";
        isRead = false;
    }

    Notification(int nId, int cId, const string& msg, const string& timeStr = "", bool read = false) {
        id = nId;
        customerId = cId;
        message = msg;
        timestamp = timeStr;
        isRead = read;
    }

    // Getters and Setters
    int getId() const { return id; }
    void setId(int nId) { id = nId; }

    int getCustomerId() const { return customerId; }
    void setCustomerId(int cId) { customerId = cId; }

    string getMessage() const { return message; }
    void setMessage(const string& msg) { message = msg; }

    string getTimestamp() const { return timestamp; }
    void setTimestamp(const string& timeStr) { timestamp = timeStr; }

    bool getIsRead() const { return isRead; }
    void setIsRead(bool read) { isRead = read; }

    // JSON serialization
    string toJSON() const {
        stringstream ss;
        ss << "{"
           << "\"id\":" << id << ","
           << "\"customer_id\":" << customerId << ","
           << "\"message\":\"" << message << "\","
           << "\"timestamp\":\"" << timestamp << "\","
           << "\"is_read\":" << (isRead ? "true" : "false")
           << "}";
        return ss.str();
    }
};

// =================================================================
// 9. PAYMENT CLASS
// =================================================================
class Payment {
private:
    int paymentId;
    int orderId;
    double amount;
    string method; // "Wallet" or "QR"
    string status; // "Pending", "Paid", "Failed"

public:
    // Constructors
    Payment() {
        paymentId = 0;
        orderId = 0;
        amount = 0.0;
        method = "Wallet";
        status = "Pending";
    }

    Payment(int pId, int oId, double amt, const string& payMethod, const string& payStatus = "Pending") {
        paymentId = pId;
        orderId = oId;
        amount = amt;
        method = payMethod;
        status = payStatus;
    }

    // Process customer payment
    bool processPayment(Wallet& customerWallet) {
        if (method == "Wallet") {
            stringstream ss;
            ss << "Order #" << orderId << " payment";
            if (customerWallet.deductMoney(amount, ss.str())) {
                status = "Paid";
                return true;
            } else {
                status = "Failed";
                return false;
            }
        } else if (method == "QR") {
            // Mocking QR payment scan as auto-approval for simplicity
            status = "Paid";
            return true;
        }
        status = "Failed";
        return false;
    }

    // Getters and Setters
    int getPaymentId() const { return paymentId; }
    int getOrderId() const { return orderId; }
    double getAmount() const { return amount; }
    string getMethod() const { return method; }
    string getStatus() const { return status; }
    void setStatus(const string& payStatus) { status = payStatus; }

    // JSON representation
    string toJSON() const {
        stringstream ss;
        ss << "{"
           << "\"payment_id\":" << paymentId << ","
           << "\"order_id\":" << orderId << ","
           << "\"amount\":" << fixed << setprecision(2) << amount << ","
           << "\"method\":\"" << method << "\","
           << "\"status\":\"" << status << "\""
           << "}";
        return ss.str();
    }
};

// =================================================================
// 10. REVIEW CLASS
// =================================================================
class Review {
private:
    int id;
    int customerId;
    int menuItemId;
    int rating;
    string comment;
    string customerName;
    string timestamp;

public:
    Review() {
        id = 0;
        customerId = 0;
        menuItemId = 0;
        rating = 5;
        comment = "";
        customerName = "";
        timestamp = "";
    }

    Review(int rId, int cId, int mId, int rate, const string& comm, const string& cName, const string& timeStr) {
        id = rId;
        customerId = cId;
        menuItemId = mId;
        rating = rate;
        comment = comm;
        customerName = cName;
        timestamp = timeStr;
    }

    int getId() const { return id; }
    int getCustomerId() const { return customerId; }
    int getMenuItemId() const { return menuItemId; }
    int getRating() const { return rating; }
    string getComment() const { return comment; }
    string getCustomerName() const { return customerName; }
    string getTimestamp() const { return timestamp; }

    string toJSON() const {
        stringstream ss;
        ss << "{"
           << "\"id\":" << id << ","
           << "\"customer_id\":" << customerId << ","
           << "\"menu_item_id\":" << menuItemId << ","
           << "\"rating\":" << rating << ","
           << "\"comment\":\"" << comment << "\","
           << "\"customer_name\":\"" << customerName << "\","
           << "\"timestamp\":\"" << timestamp << "\""
           << "}";
        return ss.str();
    }
};

// =================================================================
// 11. ANNOUNCEMENT CLASS
// =================================================================
class Announcement {
private:
    int id;
    string message;
    string timestamp;

public:
    Announcement() {
        id = 0;
        message = "";
        timestamp = "";
    }

    Announcement(int aId, const string& msg, const string& timeStr) {
        id = aId;
        message = msg;
        timestamp = timeStr;
    }

    int getId() const { return id; }
    string getMessage() const { return message; }
    string getTimestamp() const { return timestamp; }

    string toJSON() const {
        stringstream ss;
        ss << "{"
           << "\"id\":" << id << ","
           << "\"message\":\"" << message << "\","
           << "\"timestamp\":\"" << timestamp << "\""
           << "}";
        return ss.str();
    }
};

// =================================================================
// 12. COUPON CLASS
// =================================================================
class Coupon {
private:
    string code;
    double discountPercent;
    bool isActive;

public:
    Coupon() {
        code = "";
        discountPercent = 0.0;
        isActive = false;
    }

    Coupon(const string& cCode, double discount, bool active = true) {
        code = cCode;
        discountPercent = discount;
        isActive = active;
    }

    string getCode() const { return code; }
    double getDiscountPercent() const { return discountPercent; }
    bool getIsActive() const { return isActive; }
    void setIsActive(bool active) { isActive = active; }

    string toJSON() const {
        stringstream ss;
        ss << "{"
           << "\"code\":\"" << code << "\","
           << "\"discount_percent\":" << fixed << setprecision(2) << discountPercent << ","
           << "\"is_active\":" << (isActive ? "true" : "false")
           << "}";
        return ss.str();
    }
};

// =================================================================
// 13. BANNER CLASS
// =================================================================
class Banner {
private:
    int id;
    string text;
    string imageUrl;
    bool isActive;

public:
    Banner() {
        id = 0;
        text = "";
        imageUrl = "";
        isActive = false;
    }

    Banner(int bId, const string& bText, const string& bImageUrl, bool active = true) {
        id = bId;
        text = bText;
        imageUrl = bImageUrl;
        isActive = active;
    }

    int getId() const { return id; }
    string getText() const { return text; }
    string getImageUrl() const { return imageUrl; }
    bool getIsActive() const { return isActive; }

    string toJSON() const {
        stringstream ss;
        ss << "{"
           << "\"id\":" << id << ","
           << "\"banner_text\":\"" << text << "\","
           << "\"banner_image_url\":\"" << imageUrl << "\","
           << "\"is_active\":" << (isActive ? "true" : "false")
           << "}";
        return ss.str();
    }
};

#endif // CLASSES_H
