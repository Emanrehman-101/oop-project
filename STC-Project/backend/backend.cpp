#include "cgi_helper.h"
#include "database.h"
#include "classes.h"

using namespace std;

int main() {
    // 1. Handle OPTIONS requests (pre-flight checks from browsers)
    if (CGI::handleOptions()) {
        return 0;
    }

    // 2. Instantiate DatabaseManager (handles MySQL or File DB automatically)
    DatabaseManager db;

    // 3. Get action from the URL Query String (GET parameters)
    string queryStr = CGI::getEnv("QUERY_STRING");
    map<string, string> queryParams = CGI::parseQueryString(queryStr);
    
    string action = queryParams["action"];
    string method = CGI::getEnv("REQUEST_METHOD");

    // Helper variables
    stringstream response;

    // =================================================================
    // ROUTE HANDLERS
    // =================================================================

    if (action == "get_menu") {
        // GET MENU
        vector<MenuItem> menu = db.getMenu();
        response << "[";
        for (size_t i = 0; i < menu.size(); ++i) {
            response << menu[i].toJSON();
            if (i < menu.size() - 1) response << ",";
        }
        response << "]";
        CGI::printResponse(response.str(), 200);
    }
    
    else if (action == "login" && method == "POST") {
        // LOGIN USER
        string body = CGI::readPostBody();
        map<string, string> data = CGI::parseJSON(body);

        string username = data["username"];
        string password = data["password"];

        if (username.empty() || password.empty()) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Username and password are required.\"}", 400);
            return 0;
        }

        User* user = db.loginUser(username, password);
        if (user != NULL) {
            response << "{\"status\":\"success\",\"user\":" << user->toJSON() << "}";
            CGI::printResponse(response.str(), 200);
            delete user; // Free memory (Destructor executes)
        } else {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Invalid username or password.\"}", 401);
        }
    }

    else if (action == "register" && method == "POST") {
        // REGISTER CUSTOMER
        string body = CGI::readPostBody();
        map<string, string> data = CGI::parseJSON(body);

        string username = data["username"];
        string password = data["password"];
        string email = data["email"];
        string phone = data["phone"];
        string address = data["address"];

        if (username.empty() || password.empty() || email.empty()) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Username, password, and email are required.\"}", 400);
            return 0;
        }

        if (db.registerCustomer(username, password, email, phone, address)) {
            CGI::printResponse("{\"status\":\"success\",\"message\":\"Registration successful! Enjoy your $100 starting balance.\"}", 200);
        } else {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Username already exists.\"}", 400);
        }
    }

    else if (action == "place_order" && method == "POST") {
        // PLACE ORDER
        string body = CGI::readPostBody();
        map<string, string> data = CGI::parseJSON(body);

        int customerId = atoi(data["customerId"].c_str());
        string items = data["items"]; // format "itemId:qty,itemId:qty"
        double totalAmount = atof(data["totalAmount"].c_str());
        string paymentMethod = data["paymentMethod"]; // "Wallet" or "QR"

        if (customerId <= 0 || items.empty() || totalAmount <= 0) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Invalid order data.\"}", 400);
            return 0;
        }

        int orderId = db.placeOrder(customerId, items, totalAmount, paymentMethod);
        if (orderId != -1) {
            CGI::printResponse("{\"status\":\"success\",\"orderId\":" + to_string(orderId) + "}", 200);
        } else {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Transaction failed. Check wallet balance.\"}", 400);
        }
    }

    else if (action == "get_order") {
        // GET ORDER STATUS (For Live Tracker)
        int orderId = atoi(queryParams["orderId"].c_str());
        if (orderId <= 0) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Missing orderId.\"}", 400);
            return 0;
        }

        Order order = db.getOrder(orderId);
        if (order.getOrderId() > 0) {
            CGI::printResponse(order.toJSON(), 200);
        } else {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Order not found.\"}", 404);
        }
    }

    else if (action == "get_customer_orders") {
        // GET ORDER HISTORY FOR CUSTOMER
        int customerId = atoi(queryParams["customerId"].c_str());
        if (customerId <= 0) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Missing customerId.\"}", 400);
            return 0;
        }

        vector<Order> orders = db.getCustomerOrders(customerId);
        response << "[";
        for (size_t i = 0; i < orders.size(); ++i) {
            response << orders[i].toJSON();
            if (i < orders.size() - 1) response << ",";
        }
        response << "]";
        CGI::printResponse(response.str(), 200);
    }

    else if (action == "add_wallet_balance" && method == "POST") {
        // ADD BALANCE
        string body = CGI::readPostBody();
        map<string, string> data = CGI::parseJSON(body);

        int customerId = atoi(data["customerId"].c_str());
        double amount = atof(data["amount"].c_str());
        string cardOrQR = data["method"]; // "CreditCard" or "QR"

        if (customerId <= 0 || amount <= 0) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Invalid balance load details.\"}", 400);
            return 0;
        }

        string desc = "Loaded money via " + (cardOrQR.empty() ? "Credit Card" : cardOrQR);
        if (db.addWalletBalance(customerId, amount, desc)) {
            CGI::printResponse("{\"status\":\"success\",\"message\":\"Money successfully added.\"}", 200);
        } else {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Failed to load money.\"}", 500);
        }
    }

    else if (action == "get_notifications") {
        // GET NOTIFICATIONS FOR CUSTOMER
        int customerId = atoi(queryParams["customerId"].c_str());
        if (customerId <= 0) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Missing customerId.\"}", 400);
            return 0;
        }

        vector<Notification> list = db.getNotifications(customerId);
        response << "[";
        for (size_t i = 0; i < list.size(); ++i) {
            response << list[i].toJSON();
            if (i < list.size() - 1) response << ",";
        }
        response << "]";
        CGI::printResponse(response.str(), 200);
    }

    else if (action == "mark_notifications_read" && method == "POST") {
        // MARK NOTIFICATIONS AS READ
        string body = CGI::readPostBody();
        map<string, string> data = CGI::parseJSON(body);
        int customerId = atoi(data["customerId"].c_str());

        if (customerId <= 0) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Invalid customer ID.\"}", 400);
            return 0;
        }

        if (db.markNotificationsRead(customerId)) {
            CGI::printResponse("{\"status\":\"success\"}", 200);
        } else {
            CGI::printResponse("{\"status\":\"error\"}", 500);
        }
    }

    // =================================================================
    // ADMIN ONLY ENDPOINTS
    // =================================================================

    else if (action == "add_menu_item" && method == "POST") {
        string body = CGI::readPostBody();
        map<string, string> data = CGI::parseJSON(body);

        string name = data["name"];
        string description = data["description"];
        double price = atof(data["price"].c_str());
        string category = data["category"];

        if (name.empty() || price <= 0 || category.empty()) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Invalid menu item data.\"}", 400);
            return 0;
        }

        if (db.addMenuItem(name, description, price, category)) {
            CGI::printResponse("{\"status\":\"success\",\"message\":\"Menu item added!\"}", 200);
        } else {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Could not add menu item.\"}", 500);
        }
    }

    else if (action == "update_menu_item" && method == "POST") {
        string body = CGI::readPostBody();
        map<string, string> data = CGI::parseJSON(body);

        int id = atoi(data["id"].c_str());
        string name = data["name"];
        string description = data["description"];
        double price = atof(data["price"].c_str());
        string category = data["category"];
        bool isSoldOut = data["is_sold_out"] == "1" || data["is_sold_out"] == "true";

        if (id <= 0 || name.empty() || price <= 0 || category.empty()) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Invalid update fields.\"}", 400);
            return 0;
        }

        if (db.updateMenuItem(id, name, description, price, category, isSoldOut)) {
            CGI::printResponse("{\"status\":\"success\",\"message\":\"Item updated successfully.\"}", 200);
        } else {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Item not found or could not update.\"}", 500);
        }
    }

    else if (action == "delete_menu_item" && method == "POST") {
        string body = CGI::readPostBody();
        map<string, string> data = CGI::parseJSON(body);
        int id = atoi(data["id"].c_str());

        if (id <= 0) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Invalid ID.\"}", 400);
            return 0;
        }

        if (db.deleteMenuItem(id)) {
            CGI::printResponse("{\"status\":\"success\",\"message\":\"Item deleted successfully.\"}", 200);
        } else {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Item not found.\"}", 404);
        }
    }

    else if (action == "set_sold_out" && method == "POST") {
        string body = CGI::readPostBody();
        map<string, string> data = CGI::parseJSON(body);

        int id = atoi(data["id"].c_str());
        bool sold = data["is_sold_out"] == "1" || data["is_sold_out"] == "true";

        if (id <= 0) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Invalid ID.\"}", 400);
            return 0;
        }

        if (db.setMenuItemSoldOut(id, sold)) {
            CGI::printResponse("{\"status\":\"success\",\"message\":\"Sold out status updated.\"}", 200);
        } else {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Failed to update status.\"}", 500);
        }
    }

    else if (action == "get_all_orders") {
        // GET ALL ORDERS FOR ADMIN VIEW
        vector<Order> orders = db.getAllOrders();
        response << "[";
        for (size_t i = 0; i < orders.size(); ++i) {
            response << orders[i].toJSON();
            if (i < orders.size() - 1) response << ",";
        }
        response << "]";
        CGI::printResponse(response.str(), 200);
    }

    else if (action == "update_order_status" && method == "POST") {
        string body = CGI::readPostBody();
        map<string, string> data = CGI::parseJSON(body);

        int orderId = atoi(data["orderId"].c_str());
        string status = data["status"]; // 'Order Received', 'Preparing', 'Ready', 'Completed'

        if (orderId <= 0 || status.empty()) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Missing orderId or status.\"}", 400);
            return 0;
        }

        if (db.updateOrderStatus(orderId, status)) {
            CGI::printResponse("{\"status\":\"success\",\"message\":\"Order status updated!\"}", 200);
        } else {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Failed to update order status.\"}", 500);
        }
    }

    else if (action == "update_prep_time" && method == "POST") {
        string body = CGI::readPostBody();
        map<string, string> data = CGI::parseJSON(body);

        int orderId = atoi(data["orderId"].c_str());
        int mins = atoi(data["minutes"].c_str());

        if (orderId <= 0 || mins < 0) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Invalid parameters.\"}", 400);
            return 0;
        }

        if (db.updateOrderPreparationTime(orderId, mins)) {
            CGI::printResponse("{\"status\":\"success\",\"message\":\"Estimated time updated.\"}", 200);
        } else {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Failed to update time.\"}", 500);
        }
    }

    else if (action == "get_customers") {
        // GET CUSTOMER DATABASE FOR ADMIN VIEW
        vector<Customer> custs = db.getAllCustomers();
        response << "[";
        for (size_t i = 0; i < custs.size(); ++i) {
            response << custs[i].toJSON();
            if (i < custs.size() - 1) response << ",";
        }
        response << "]";
        CGI::printResponse(response.str(), 200);
    }

    else if (action == "get_reviews") {
        // GET REVIEWS FOR A SPECIFIC MENU ITEM OR ALL REVIEWS
        string itemIdStr = queryParams["menuItemId"];
        vector<Review> reviews;
        if (!itemIdStr.empty()) {
            int itemId = atoi(itemIdStr.c_str());
            reviews = db.getReviews(itemId);
        } else {
            reviews = db.getAllReviews();
        }
        response << "[";
        for (size_t i = 0; i < reviews.size(); ++i) {
            response << reviews[i].toJSON();
            if (i < reviews.size() - 1) response << ",";
        }
        response << "]";
        CGI::printResponse(response.str(), 200);
    }

    else if (action == "add_review" && method == "POST") {
        // SUBMIT CUSTOMER REVIEW AND RATING
        string body = CGI::readPostBody();
        map<string, string> data = CGI::parseJSON(body);
        int customerId = atoi(data["customerId"].c_str());
        int menuItemId = atoi(data["menuItemId"].c_str());
        int rating = atoi(data["rating"].c_str());
        string comment = data["comment"];

        if (customerId <= 0 || menuItemId <= 0 || rating < 1 || rating > 5) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Invalid review parameters.\"}", 400);
            return 0;
        }

        if (db.addReview(customerId, menuItemId, rating, comment)) {
            CGI::printResponse("{\"status\":\"success\",\"message\":\"Your feedback has been recorded!\"}", 200);
        } else {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Failed to save feedback.\"}", 500);
        }
    }

    else if (action == "toggle_favorite" && method == "POST") {
        // TOGGLE FAVORITE MENU ITEM
        string body = CGI::readPostBody();
        map<string, string> data = CGI::parseJSON(body);
        int customerId = atoi(data["customerId"].c_str());
        int menuItemId = atoi(data["menuItemId"].c_str());

        if (customerId <= 0 || menuItemId <= 0) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Invalid user or item details.\"}", 400);
            return 0;
        }

        bool favorited = db.toggleFavorite(customerId, menuItemId);
        if (favorited) {
            CGI::printResponse("{\"status\":\"success\",\"state\":\"added\",\"message\":\"Added to favorites.\"}", 200);
        } else {
            CGI::printResponse("{\"status\":\"success\",\"state\":\"removed\",\"message\":\"Removed from favorites.\"}", 200);
        }
    }

    else if (action == "get_favorites") {
        // GET FAVORITED ITEM IDS
        int customerId = atoi(queryParams["customerId"].c_str());
        if (customerId <= 0) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Missing customer ID.\"}", 400);
            return 0;
        }
        vector<int> favs = db.getFavorites(customerId);
        response << "[";
        for (size_t i = 0; i < favs.size(); ++i) {
            response << favs[i];
            if (i < favs.size() - 1) response << ",";
        }
        response << "]";
        CGI::printResponse(response.str(), 200);
    }

    else if (action == "get_announcements") {
        // GET ANNOUNCEMENTS LIST
        vector<Announcement> list = db.getAnnouncements();
        response << "[";
        for (size_t i = 0; i < list.size(); ++i) {
            response << list[i].toJSON();
            if (i < list.size() - 1) response << ",";
        }
        response << "]";
        CGI::printResponse(response.str(), 200);
    }

    else if (action == "add_announcement" && method == "POST") {
        // ADMIN BROADCAST ANNOUNCEMENT
        string body = CGI::readPostBody();
        map<string, string> data = CGI::parseJSON(body);
        string message = data["message"];

        if (message.empty()) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Message content is empty.\"}", 400);
            return 0;
        }

        if (db.addAnnouncement(message)) {
            CGI::printResponse("{\"status\":\"success\",\"message\":\"Announcement broadcasted successfully!\"}", 200);
        } else {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Failed to save announcement.\"}", 500);
        }
    }

    else if (action == "get_coupons") {
        // GET ALL COUPONS
        vector<Coupon> list = db.getCoupons();
        response << "[";
        for (size_t i = 0; i < list.size(); ++i) {
            response << list[i].toJSON();
            if (i < list.size() - 1) response << ",";
        }
        response << "]";
        CGI::printResponse(response.str(), 200);
    }

    else if (action == "add_coupon" && method == "POST") {
        // ADMIN CREATE NEW COUPON CODE
        string body = CGI::readPostBody();
        map<string, string> data = CGI::parseJSON(body);
        string code = data["code"];
        double discount = atof(data["discount_percent"].c_str());

        if (code.empty() || discount <= 0.0 || discount > 100.0) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Invalid coupon properties.\"}", 400);
            return 0;
        }

        if (db.addCoupon(code, discount)) {
            CGI::printResponse("{\"status\":\"success\",\"message\":\"Coupon created successfully!\"}", 200);
        } else {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Coupon code already exists.\"}", 400);
        }
    }

    else if (action == "toggle_coupon" && method == "POST") {
        // TOGGLE COUPON ACTIVE STATE
        string body = CGI::readPostBody();
        map<string, string> data = CGI::parseJSON(body);
        string code = data["code"];
        bool active = data["is_active"] == "1" || data["is_active"] == "true";

        if (code.empty()) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Missing coupon code.\"}", 400);
            return 0;
        }

        if (db.toggleCoupon(code, active)) {
            CGI::printResponse("{\"status\":\"success\",\"message\":\"Coupon state updated successfully.\"}", 200);
        } else {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Coupon code not found.\"}", 404);
        }
    }

    else if (action == "get_banners") {
        // GET PUBLIC HERO BANNERS
        vector<Banner> list = db.getBanners();
        response << "[";
        for (size_t i = 0; i < list.size(); ++i) {
            response << list[i].toJSON();
            if (i < list.size() - 1) response << ",";
        }
        response << "]";
        CGI::printResponse(response.str(), 200);
    }

    else if (action == "update_banner" && method == "POST") {
        // UPDATE WEBSITE MAIN BANNER
        string body = CGI::readPostBody();
        map<string, string> data = CGI::parseJSON(body);
        string text = data["banner_text"];
        string imageUrl = data["banner_image_url"];

        if (text.empty() || imageUrl.empty()) {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Banner details cannot be empty.\"}", 400);
            return 0;
        }

        if (db.updateBanner(text, imageUrl)) {
            CGI::printResponse("{\"status\":\"success\",\"message\":\"Website banner updated!\"}", 200);
        } else {
            CGI::printResponse("{\"status\":\"error\",\"message\":\"Failed to update banner configuration.\"}", 500);
        }
    }

    else if (action == "get_sales_analytics") {
        // GET SALES ANALYTICS (REVENUE, DAILY CHARTS, CATEGORIES)
        string jsonStats = db.getSalesAnalytics();
        CGI::printResponse(jsonStats, 200);
    }

    else {
        // UNKNOWN ACTION OR DEFAULT ROOT
        CGI::printResponse("{\"status\":\"error\",\"message\":\"Invalid action endpoint or method type.\"}", 404);
    }

    return 0;
}
