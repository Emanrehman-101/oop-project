import os
import sys
import http.server
import subprocess
import urllib.parse
import json
from datetime import datetime

PORT = 8000

# File names for Persistence fallback mode (matching database.h exactly)
USERS_FILE = "users.txt"
MENU_FILE = "menu.txt"
ORDERS_FILE = "orders.txt"
WALLET_TX_FILE = "wallet_transactions.txt"
WALLETS_FILE = "wallets.txt"
NOTIFICATIONS_FILE = "notifications.txt"
REVIEWS_FILE = "reviews.txt"
FAVORITES_FILE = "favorites.txt"
ANNOUNCEMENTS_FILE = "announcements.txt"
COUPONS_FILE = "coupons.txt"
BANNERS_FILE = "banners.txt"

# -----------------------------------------------------------------
# 1. DJB2 PASSWORD HASHING SIMULATOR (Matches database.h exactly)
# -----------------------------------------------------------------
def hash_password(password):
    h = 5381
    for c in password:
        h = ((h << 5) + h) + ord(c)
        h &= 0xFFFFFFFF
    part1 = f"{h:x}"
    part2 = f"{((h * 31) & 0xFFFFFFFF):x}"
    return f"{part1}{part2}"

# -----------------------------------------------------------------
# 2. SEED DEFAULT DB FILES (Matches C++ initializeDefaultFiles)
# -----------------------------------------------------------------
def initialize_default_files():
    now_str = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    # 1. Users File
    if not os.path.exists(USERS_FILE):
        with open(USERS_FILE, 'w') as f:
            f.write(f"1|admin|{hash_password('admin123')}|admin@stc.com|admin|+966500000001|STC Head Office, Riyadh\n")
            f.write(f"2|customer|{hash_password('customer123')}|customer@stc.com|customer|+966500000002|Olaya District, Riyadh\n")

    # 2. Wallets File - Set starting balance to 0 PKR
    if not os.path.exists(WALLETS_FILE):
        with open(WALLETS_FILE, 'w') as f:
            f.write("2|0.00\n")

    # 3. Wallet Transactions File
    if not os.path.exists(WALLET_TX_FILE):
        with open(WALLET_TX_FILE, 'w') as f:
            f.write(f"1|2|DEPOSIT|0.00|Account Created|{now_str}\n")

    # 4. Menu File - Set realistic PKR Prices
    if not os.path.exists(MENU_FILE) or os.path.getsize(MENU_FILE) == 0:
        with open(MENU_FILE, 'w') as f:
            f.write("1|STC Signature Burger|Premium beef patty with cheddar cheese and STC orange sauce.|1200.00|Burgers|0|4.8|124\n")
            f.write("2|Crispy Chicken Burger|Crispy fried chicken breast, lettuce, and spicy mayo in a brioche bun.|950.00|Burgers|0|4.6|98\n")
            f.write("3|STC Deluxe Pizza|Stone-baked pizza loaded with pepperoni, beef, and mozzarella.|2200.00|Pizza|0|4.9|142\n")
            f.write("4|Margarita Flatbread|Simplicity at its best: tomato sauce, mozzarella, and fresh basil.|1500.00|Pizza|0|4.2|53\n")
            f.write("5|Golden French Fries|Crispy golden fries seasoned with a light touch of sea salt.|450.00|Sides|0|4.5|210\n")
            f.write("6|STC Dynamite Wings|Eight pieces of crispy chicken wings tossed in signature sweet glaze.|750.00|Sides|1|4.7|76\n")
            f.write("7|Iced Latte|Chilled espresso poured over creamy whole milk and ice, sweetened.|650.00|Drinks|0|4.4|85\n")
            f.write("8|Fresh Orange Juice|100% natural, freshly squeezed juice, served ice cold.|500.00|Drinks|0|4.9|115\n")
            f.write("9|Chocolate Lava Cake|Warm chocolate cake with a molten fudge center and vanilla ice cream.|850.00|Desserts|0|4.8|189\n")

    # 5. Coupons File
    if not os.path.exists(COUPONS_FILE):
        with open(COUPONS_FILE, 'w') as f:
            f.write("STC20|20.00|1\n")
            f.write("STC10|10.00|1\n")
            f.write("WELCOME|15.00|1\n")

    # 6. Banners File
    if not os.path.exists(BANNERS_FILE):
        with open(BANNERS_FILE, 'w') as f:
            f.write("1|Order STC Signature Feast - Save 20% today with Coupon: STC20!|/images/stc_building.jpg|1\n")

    # 7. Announcements File
    if not os.path.exists(ANNOUNCEMENTS_FILE):
        with open(ANNOUNCEMENTS_FILE, 'w') as f:
            f.write(f"1|Welcome to the new STC Ordering System! Use code STC20 for 20% off all orders!|{now_str}\n")

    # 8. Reviews File
    if not os.path.exists(REVIEWS_FILE):
        with open(REVIEWS_FILE, 'w') as f:
            f.write(f"1|2|1|5|Best burger I have ever had! STC sauce is incredible.|customer|{now_str}\n")
            f.write(f"2|2|3|5|A bit spicy but very tasty, plenty of pepperoni.|customer|{now_str}\n")

    # 9. Favorites File
    if not os.path.exists(FAVORITES_FILE):
        with open(FAVORITES_FILE, 'w') as f:
            f.write("2|1\n")
            f.write("2|3\n")


# -----------------------------------------------------------------
# 3. PYTHON SIMULATED BACKEND ENGINE (Matches C++ CGI endpoints)
# -----------------------------------------------------------------
class PythonCgiSimulator:
    @staticmethod
    def get_menu():
        initialize_default_files()
        items = []
        with open(MENU_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if len(parts) >= 8:
                    items.append({
                        "id": int(parts[0]),
                        "name": parts[1],
                        "description": parts[2],
                        "price": float(parts[3]),
                        "category": parts[4],
                        "is_sold_out": parts[5] == "1" or parts[5].lower() == "true",
                        "rating": float(parts[6]),
                        "reviews_count": int(parts[7])
                    })
        return items

    @staticmethod
    def get_wallet(cust_id):
        initialize_default_files()
        balance = 0.00
        if os.path.exists(WALLETS_FILE):
            with open(WALLETS_FILE, 'r') as f:
                for line in f:
                    parts = line.strip().split('|')
                    if len(parts) >= 2 and int(parts[0]) == cust_id:
                        balance = float(parts[1])
                        break
        
        txs = []
        if os.path.exists(WALLET_TX_FILE):
            with open(WALLET_TX_FILE, 'r') as f:
                for line in f:
                    parts = line.strip().split('|')
                    if len(parts) >= 6 and int(parts[1]) == cust_id:
                        type_str = parts[2]
                        amt = parts[3]
                        desc = parts[4]
                        ts = parts[5]
                        sign = "+" if type_str == "DEPOSIT" else "-"
                        txs.append(f"{desc} ({type_str}): {sign}PKR {amt} on {ts}")
        return {"balance": balance, "transactions": txs[::-1]}

    @staticmethod
    def login(username, password):
        initialize_default_files()
        hashed = hash_password(password)
        with open(USERS_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if len(parts) >= 5 and parts[1] == username and parts[2] == hashed:
                    uid = int(parts[0])
                    role = parts[4]
                    if role == "admin":
                        return {
                            "status": "success",
                            "user": {
                                "id": uid,
                                "username": parts[1],
                                "email": parts[3],
                                "role": role
                            }
                        }
                    else:
                        phone = parts[5] if len(parts) >= 6 else ""
                        addr = parts[6] if len(parts) >= 7 else ""
                        wallet = PythonCgiSimulator.get_wallet(uid)
                        return {
                            "status": "success",
                            "user": {
                                "id": uid,
                                "username": parts[1],
                                "email": parts[3],
                                "role": role,
                                "phone": phone,
                                "address": addr,
                                "wallet": wallet
                            }
                        }
        return {"status": "error", "message": "Invalid username or password."}

    @staticmethod
    def register(username, password, email, phone, address):
        initialize_default_files()
        
        # Check username exists
        with open(USERS_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if len(parts) >= 2 and parts[1] == username:
                    return {"status": "error", "message": "Username already exists."}

        # Calculate new user ID
        new_id = 1
        with open(USERS_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if parts and parts[0]:
                    new_id = max(new_id, int(parts[0]) + 1)

        hashed = hash_password(password)
        with open(USERS_FILE, 'a') as f:
            f.write(f"{new_id}|{username}|{hashed}|{email}|customer|{phone}|{address}\n")

        with open(WALLETS_FILE, 'a') as f:
            f.write(f"{new_id}|0.00\n")

        now_str = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        with open(WALLET_TX_FILE, 'a') as f:
            f.write(f"{new_id}|{new_id}|DEPOSIT|0.00|Account Created|{now_str}\n")

        return {"status": "success", "message": "Registration successful! Your wallet balance is PKR 0.00."}

    @staticmethod
    def add_wallet_balance(customerId, amount, method):
        initialize_default_files()
        lines = []
        found = False
        with open(WALLETS_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if len(parts) >= 2:
                    uid = int(parts[0])
                    bal = float(parts[1])
                    if uid == customerId:
                        bal += amount
                        found = True
                    lines.append(f"{uid}|{bal:.2f}\n")
        
        if not found:
            lines.append(f"{customerId}|{amount:.2f}\n")

        with open(WALLETS_FILE, 'w') as f:
            f.writelines(lines)

        # Log tx
        new_tx_id = 1
        if os.path.exists(WALLET_TX_FILE):
            with open(WALLET_TX_FILE, 'r') as f:
                for line in f:
                    parts = line.strip().split('|')
                    if parts and parts[0]:
                        new_tx_id = max(new_tx_id, int(parts[0]) + 1)

        now_str = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        desc = f"Loaded money via {method or 'Credit Card'}"
        with open(WALLET_TX_FILE, 'a') as f:
            f.write(f"{new_tx_id}|{customerId}|DEPOSIT|{amount:.2f}|{desc}|{now_str}\n")

        # Log notification
        PythonCgiSimulator.create_notification(customerId, f"Your E-Wallet has been loaded with PKR {amount:.2f} via {method or 'Credit Card'}.")

        return {"status": "success", "message": "Money successfully added."}

    @staticmethod
    def deduct_wallet_balance(customerId, amount, desc):
        lines = []
        success = False
        with open(WALLETS_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if len(parts) >= 2:
                    uid = int(parts[0])
                    bal = float(parts[1])
                    if uid == customerId and bal >= amount:
                        bal -= amount
                        success = True
                    lines.append(f"{uid}|{bal:.2f}\n")
        
        if success:
            with open(WALLETS_FILE, 'w') as f:
                f.writelines(lines)
            
            # Log tx
            new_tx_id = 1
            if os.path.exists(WALLET_TX_FILE):
                with open(WALLET_TX_FILE, 'r') as f:
                    for line in f:
                        parts = line.strip().split('|')
                        if parts and parts[0]:
                            new_tx_id = max(new_tx_id, int(parts[0]) + 1)
            
            now_str = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            with open(WALLET_TX_FILE, 'a') as f:
                f.write(f"{new_tx_id}|{customerId}|PAYMENT|{amount:.2f}|{desc}|{now_str}\n")
            return True
        return False

    @staticmethod
    def create_notification(customerId, msg):
        new_id = 1
        if os.path.exists(NOTIFICATIONS_FILE):
            with open(NOTIFICATIONS_FILE, 'r') as f:
                for line in f:
                    parts = line.strip().split('|')
                    if parts and parts[0]:
                        try:
                            new_id = max(new_id, int(parts[0]) + 1)
                        except ValueError:
                            pass
        
        # Sanitize message: replace pipe chars to prevent file parsing issues
        safe_msg = msg.replace('|', ' -')
        now_str = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        with open(NOTIFICATIONS_FILE, 'a') as f:
            f.write(f"{new_id}|{customerId}|{safe_msg}|0|{now_str}\n")

    @staticmethod
    def place_order(customerId, items, totalAmount, paymentMethod):
        initialize_default_files()
        
        if paymentMethod == "Wallet":
            if not PythonCgiSimulator.deduct_wallet_balance(customerId, totalAmount, "Order checkout payment"):
                return {"status": "error", "message": "Transaction failed. Check wallet balance."}

        new_order_id = 1
        if os.path.exists(ORDERS_FILE):
            with open(ORDERS_FILE, 'r') as f:
                for line in f:
                    parts = line.strip().split('|')
                    if parts and parts[0]:
                        new_order_id = max(new_order_id, int(parts[0]) + 1)

        now_str = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        # Format: id|customerId|totalAmount|status|prepTime|timestamp|items|paymentMethod
        with open(ORDERS_FILE, 'a') as f:
            f.write(f"{new_order_id}|{customerId}|{totalAmount:.2f}|Order Received|15|{now_str}|{items}|{paymentMethod}\n")

        msg = f"Your order #{new_order_id} has been received! Payment Mode: {paymentMethod} - Total: PKR {totalAmount:.2f}"
        PythonCgiSimulator.create_notification(customerId, msg)

        # Also notify admin (customerId=1 is admin)
        # Get customer username for admin notification
        cust_name = "Customer"
        with open(USERS_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if parts and int(parts[0]) == customerId:
                    cust_name = parts[1]
                    break
        admin_msg = f"New Order #{new_order_id} from {cust_name}! Items: {items} - Total: PKR {totalAmount:.2f} - Payment: {paymentMethod}"
        PythonCgiSimulator.create_notification(1, admin_msg)

        return {"status": "success", "orderId": new_order_id}

    @staticmethod
    def get_order(orderId):
        initialize_default_files()
        if os.path.exists(ORDERS_FILE):
            with open(ORDERS_FILE, 'r') as f:
                for line in f:
                    parts = line.strip().split('|')
                    if parts and int(parts[0]) == orderId:
                        return {
                            "orderId": int(parts[0]),
                            "customerId": int(parts[1]),
                            "total_amount": float(parts[2]),
                            "status": parts[3],
                            "preparation_time": int(parts[4]),
                            "timestamp": parts[5],
                            "items": parts[6],
                            "payment_method": parts[7]
                        }
        return {"status": "error", "message": "Order not found."}

    @staticmethod
    def get_customer_orders(customerId):
        initialize_default_files()
        orders = []
        if os.path.exists(ORDERS_FILE):
            with open(ORDERS_FILE, 'r') as f:
                for line in f:
                    parts = line.strip().split('|')
                    if parts and len(parts) >= 8 and int(parts[1]) == customerId:
                        orders.append({
                            "orderId": int(parts[0]),
                            "customerId": int(parts[1]),
                            "total_amount": float(parts[2]),
                            "status": parts[3],
                            "preparation_time": int(parts[4]),
                            "timestamp": parts[5],
                            "items": parts[6],
                            "payment_method": parts[7]
                        })
        return orders[::-1]

    @staticmethod
    def get_notifications(customerId):
        initialize_default_files()
        list_notif = []
        if os.path.exists(NOTIFICATIONS_FILE):
            with open(NOTIFICATIONS_FILE, 'r') as f:
                for line in f:
                    parts = line.strip().split('|')
                    if len(parts) >= 5:
                        try:
                            cid = int(parts[1])
                        except ValueError:
                            continue
                        if cid == customerId:
                            list_notif.append({
                                "id": int(parts[0]),
                                "customerId": cid,
                                "message": parts[2],
                                "is_read": int(parts[3]),
                                "timestamp": parts[4]
                            })
        return list_notif[::-1]

    @staticmethod
    def mark_notifications_read(customerId):
        initialize_default_files()
        if not os.path.exists(NOTIFICATIONS_FILE):
            return {"status": "success"}

        lines = []
        with open(NOTIFICATIONS_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if len(parts) >= 5:
                    if int(parts[1]) == customerId:
                        parts[3] = "1"
                    lines.append("|".join(parts) + "\n")
        with open(NOTIFICATIONS_FILE, 'w') as f:
            f.writelines(lines)
        return {"status": "success"}

    @staticmethod
    def add_menu_item(name, description, price, category):
        initialize_default_files()
        new_id = 1
        with open(MENU_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if parts and parts[0]:
                    new_id = max(new_id, int(parts[0]) + 1)
        
        with open(MENU_FILE, 'a') as f:
            f.write(f"{new_id}|{name}|{description}|{price:.2f}|{category}|0|5.0|0\n")
        return {"status": "success", "message": "Menu item added!"}

    @staticmethod
    def update_menu_item(itemId, name, description, price, category, isSoldOut):
        initialize_default_files()
        lines = []
        found = False
        with open(MENU_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if len(parts) >= 8:
                    mid = int(parts[0])
                    if mid == itemId:
                        parts[1] = name
                        parts[2] = description
                        parts[3] = f"{price:.2f}"
                        parts[4] = category
                        parts[5] = "1" if isSoldOut else "0"
                        found = True
                    lines.append("|".join(parts) + "\n")
        
        if found:
            with open(MENU_FILE, 'w') as f:
                f.writelines(lines)
            return {"status": "success", "message": "Item updated successfully."}
        return {"status": "error", "message": "Item not found."}

    @staticmethod
    def delete_menu_item(itemId):
        initialize_default_files()
        lines = []
        found = False
        with open(MENU_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if parts and parts[0]:
                    if int(parts[0]) == itemId:
                        found = True
                        continue
                    lines.append(line)
        
        if found:
            with open(MENU_FILE, 'w') as f:
                f.writelines(lines)
            return {"status": "success", "message": "Item deleted successfully."}
        return {"status": "error", "message": "Item not found."}

    @staticmethod
    def set_sold_out(itemId, isSoldOut):
        initialize_default_files()
        lines = []
        found = False
        with open(MENU_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if len(parts) >= 8:
                    if int(parts[0]) == itemId:
                        parts[5] = "1" if isSoldOut else "0"
                        found = True
                    lines.append("|".join(parts) + "\n")
        
        if found:
            with open(MENU_FILE, 'w') as f:
                f.writelines(lines)
            return {"status": "success", "message": "Sold out status updated."}
        return {"status": "error", "message": "Failed to update status."}

    @staticmethod
    def get_all_orders():
        initialize_default_files()
        orders = []
        if os.path.exists(ORDERS_FILE):
            with open(ORDERS_FILE, 'r') as f:
                for line in f:
                    parts = line.strip().split('|')
                    if len(parts) >= 8:
                        orders.append({
                            "orderId": int(parts[0]),
                            "customerId": int(parts[1]),
                            "total_amount": float(parts[2]),
                            "status": parts[3],
                            "preparation_time": int(parts[4]),
                            "timestamp": parts[5],
                            "items": parts[6],
                            "payment_method": parts[7]
                        })
        return orders[::-1]

    @staticmethod
    def update_order_status(orderId, status):
        initialize_default_files()
        lines = []
        found = False
        cust_id = -1
        with open(ORDERS_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if len(parts) >= 8:
                    if int(parts[0]) == orderId:
                        parts[3] = status
                        cust_id = int(parts[1])
                        found = True
                    lines.append("|".join(parts) + "\n")
        
        if found:
            with open(ORDERS_FILE, 'w') as f:
                f.writelines(lines)
            
            # Notify customer
            msg = f"Your order #{orderId} status has been updated to: {status}"
            PythonCgiSimulator.create_notification(cust_id, msg)
            return {"status": "success", "message": "Order status updated!"}
        return {"status": "error", "message": "Failed to update order status."}

    @staticmethod
    def update_prep_time(orderId, minutes):
        initialize_default_files()
        lines = []
        found = False
        with open(ORDERS_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if len(parts) >= 8:
                    if int(parts[0]) == orderId:
                        parts[4] = str(minutes)
                        found = True
                    lines.append("|".join(parts) + "\n")
        
        if found:
            with open(ORDERS_FILE, 'w') as f:
                f.writelines(lines)
            return {"status": "success", "message": "Estimated time updated."}
        return {"status": "error", "message": "Failed to update time."}

    @staticmethod
    def get_customers():
        initialize_default_files()
        custs = []
        with open(USERS_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if len(parts) >= 5 and parts[4] == "customer":
                    uid = int(parts[0])
                    phone = parts[5] if len(parts) >= 6 else ""
                    addr = parts[6] if len(parts) >= 7 else ""
                    wallet = PythonCgiSimulator.get_wallet(uid)
                    custs.append({
                        "id": uid,
                        "username": parts[1],
                        "email": parts[3],
                        "role": "customer",
                        "phone": phone,
                        "address": addr,
                        "wallet": wallet
                    })
        return custs

    @staticmethod
    def get_reviews(menuItemId):
        initialize_default_files()
        revs = []
        if os.path.exists(REVIEWS_FILE):
            with open(REVIEWS_FILE, 'r') as f:
                for line in f:
                    parts = line.strip().split('|')
                    if len(parts) >= 7:
                        iid = int(parts[2])
                        if menuItemId is None or iid == menuItemId:
                            revs.append({
                                "id": int(parts[0]),
                                "customerId": int(parts[1]),
                                "menuItemId": iid,
                                "rating": int(parts[3]),
                                "comment": parts[4],
                                "username": parts[5],
                                "timestamp": parts[6]
                            })
        return revs[::-1]

    @staticmethod
    def add_review(customerId, menuItemId, rating, comment):
        initialize_default_files()
        
        # Get username
        username = "customer"
        with open(USERS_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if parts and int(parts[0]) == customerId:
                    username = parts[1]
                    break

        new_rev_id = 1
        if os.path.exists(REVIEWS_FILE):
            with open(REVIEWS_FILE, 'r') as f:
                for line in f:
                    parts = line.strip().split('|')
                    if parts and parts[0]:
                        new_rev_id = max(new_rev_id, int(parts[0]) + 1)
        
        now_str = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        with open(REVIEWS_FILE, 'a') as f:
            f.write(f"{new_rev_id}|{customerId}|{menuItemId}|{rating}|{comment}|{username}|{now_str}\n")

        # Recalculate menu item rating
        menu_lines = []
        with open(MENU_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if len(parts) >= 8:
                    mid = int(parts[0])
                    if mid == menuItemId:
                        # Fetch all reviews for this item
                        all_revs = PythonCgiSimulator.get_reviews(menuItemId)
                        all_revs.append({"rating": rating}) # Include this new one
                        avg_rating = sum(r["rating"] for r in all_revs) / len(all_revs)
                        parts[6] = f"{avg_rating:.1f}"
                        parts[7] = str(len(all_revs))
                    menu_lines.append("|".join(parts) + "\n")
        
        with open(MENU_FILE, 'w') as f:
            f.writelines(menu_lines)

        return {"status": "success", "message": "Your feedback has been recorded!"}

    @staticmethod
    def toggle_favorite(customerId, menuItemId):
        initialize_default_files()
        lines = []
        found = False
        if os.path.exists(FAVORITES_FILE):
            with open(FAVORITES_FILE, 'r') as f:
                for line in f:
                    parts = line.strip().split('|')
                    if len(parts) >= 2:
                        uid = int(parts[0])
                        iid = int(parts[1])
                        if uid == customerId and iid == menuItemId:
                            found = True
                            continue
                        lines.append(line)
        
        if found:
            with open(FAVORITES_FILE, 'w') as f:
                f.writelines(lines)
            return {"status": "success", "state": "removed", "message": "Removed from favorites."}
        else:
            with open(FAVORITES_FILE, 'a') as f:
                f.write(f"{customerId}|{menuItemId}\n")
            return {"status": "success", "state": "added", "message": "Added to favorites."}

    @staticmethod
    def get_favorites(customerId):
        initialize_default_files()
        favs = []
        if os.path.exists(FAVORITES_FILE):
            with open(FAVORITES_FILE, 'r') as f:
                for line in f:
                    parts = line.strip().split('|')
                    if len(parts) >= 2 and int(parts[0]) == customerId:
                        favs.append(int(parts[1]))
        return favs

    @staticmethod
    def get_announcements():
        initialize_default_files()
        list_a = []
        if os.path.exists(ANNOUNCEMENTS_FILE):
            with open(ANNOUNCEMENTS_FILE, 'r') as f:
                for line in f:
                    parts = line.strip().split('|')
                    if len(parts) >= 3:
                        list_a.append({
                            "id": int(parts[0]),
                            "message": parts[1],
                            "timestamp": parts[2]
                        })
        return list_a[::-1]

    @staticmethod
    def add_announcement(message):
        initialize_default_files()
        new_id = 1
        with open(ANNOUNCEMENTS_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if parts and parts[0]:
                    new_id = max(new_id, int(parts[0]) + 1)
        
        now_str = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        with open(ANNOUNCEMENTS_FILE, 'a') as f:
            f.write(f"{new_id}|{message}|{now_str}\n")
        return {"status": "success", "message": "Announcement broadcasted successfully!"}

    @staticmethod
    def get_coupons():
        initialize_default_files()
        list_c = []
        with open(COUPONS_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if len(parts) >= 3:
                    list_c.append({
                        "code": parts[0],
                        "discount_percent": float(parts[1]),
                        "is_active": int(parts[2])
                    })
        return list_c

    @staticmethod
    def add_coupon(code, discount_percent):
        initialize_default_files()
        # Check exists
        with open(COUPONS_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if parts and parts[0].upper() == code.upper():
                    return {"status": "error", "message": "Coupon code already exists."}

        with open(COUPONS_FILE, 'a') as f:
            f.write(f"{code.upper()}|{discount_percent:.2f}|1\n")
        return {"status": "success", "message": "Coupon created successfully!"}

    @staticmethod
    def toggle_coupon(code, is_active):
        initialize_default_files()
        lines = []
        found = False
        with open(COUPONS_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if parts and parts[0].upper() == code.upper():
                    parts[2] = "1" if is_active else "0"
                    found = True
                lines.append("|".join(parts) + "\n")
        
        if found:
            with open(COUPONS_FILE, 'w') as f:
                f.writelines(lines)
            return {"status": "success", "message": "Coupon state updated successfully."}
        return {"status": "error", "message": "Coupon code not found."}

    @staticmethod
    def get_banners():
        initialize_default_files()
        list_b = []
        with open(BANNERS_FILE, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                if len(parts) >= 4:
                    list_b.append({
                        "id": int(parts[0]),
                        "banner_text": parts[1],
                        "banner_image_url": parts[2],
                        "is_active": int(parts[3])
                    })
        return list_b

    @staticmethod
    def update_banner(banner_text, banner_image_url):
        initialize_default_files()
        with open(BANNERS_FILE, 'w') as f:
            f.write(f"1|{banner_text}|{banner_image_url}|1\n")
        return {"status": "success", "message": "Website banner updated!"}

    @staticmethod
    def get_sales_analytics():
        initialize_default_files()
        orders = PythonCgiSimulator.get_all_orders()
        customers = PythonCgiSimulator.get_customers()
        
        total_revenue = sum(o["total_amount"] for o in orders)
        total_orders = len(orders)
        total_customers = len(customers)

        # Sales by category
        cat_rev = {}
        for o in orders:
            # o.items is format "itemId:qty,itemId:qty"
            items_list = o["items"].split(',')
            for pair in items_list:
                if ':' in pair:
                    parts = pair.split(':')
                    iid = int(parts[0])
                    qty = int(parts[1])
                    menu_item = next((m for m in PythonCgiSimulator.get_menu() if m["id"] == iid), None)
                    if menu_item:
                        cat = menu_item["category"]
                        cat_rev[cat] = cat_rev.get(cat, 0.0) + (menu_item["price"] * qty)

        category_revenue = [{"category": cat, "revenue": rev} for cat, rev in cat_rev.items()]
        
        # Daily activity logs (unique dates)
        daily = {}
        for o in orders:
            # o.timestamp is format "YYYY-MM-DD HH:MM:S" -> strip time
            date = o["timestamp"].split(' ')[0]
            daily[date] = daily.get(date, 0) + 1
        
        daily_orders = [{"date": dt, "orders_count": count} for dt, count in sorted(daily.items(), reverse=True)]

        return {
            "total_revenue": total_revenue,
            "total_orders": total_orders,
            "total_customers": total_customers,
            "category_revenue": category_revenue,
            "daily_orders": daily_orders[:5]
        }


# -----------------------------------------------------------------
# 4. CUSTOM HTTP REQUEST HANDLER
# -----------------------------------------------------------------
class CustomHTTPRequestHandler(http.server.BaseHTTPRequestHandler):
    def end_headers(self):
        # Add CORS headers to all responses
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        super().end_headers()

    def do_OPTIONS(self):
        self.send_response(200)
        self.end_headers()

    def do_GET(self):
        if self.path.startswith('/cgi-bin/'):
            self.handle_cgi()
        else:
            self.handle_static()

    def do_POST(self):
        if self.path.startswith('/cgi-bin/'):
            self.handle_cgi()
        else:
            self.send_error(404, "POST not supported for static files")

    def handle_static(self):
        # Clean up path
        url_path = urllib.parse.urlparse(self.path).path
        if url_path == '/' or url_path == '':
            file_path = 'index.html' # Changed to check root first for Netlify optimization
        elif url_path.endswith('.html'):
            file_path = url_path.lstrip('/')
        else:
            file_path = url_path.lstrip('/')

        # Prevent directory traversal
        file_path = os.path.normpath(file_path)
        if not os.path.exists(file_path) or os.path.isdir(file_path):
            # Try frontend/
            file_path = os.path.join('frontend', file_path)
            if not os.path.exists(file_path) or os.path.isdir(file_path):
                # Try from root
                file_path = urllib.parse.urlparse(self.path).path.lstrip('/')
                if not os.path.exists(file_path) or os.path.isdir(file_path):
                    self.send_error(404, f"File not found: {self.path}")
                    return

        # Determine Content-Type
        content_type = 'text/plain'
        if file_path.endswith('.html'):
            content_type = 'text/html'
        elif file_path.endswith('.css'):
            content_type = 'text/css'
        elif file_path.endswith('.js'):
            content_type = 'application/javascript'
        elif file_path.endswith('.jpg') or file_path.endswith('.jpeg'):
            content_type = 'image/jpeg'
        elif file_path.endswith('.png'):
            content_type = 'image/png'
        elif file_path.endswith('.gif'):
            content_type = 'image/gif'
        elif file_path.endswith('.svg'):
            content_type = 'image/svg+xml'
        elif file_path.endswith('.ico'):
            content_type = 'image/x-icon'

        try:
            with open(file_path, 'rb') as f:
                content = f.read()
            self.send_response(200)
            self.send_header('Content-Type', content_type)
            self.send_header('Content-Length', str(len(content)))
            self.end_headers()
            self.wfile.write(content)
        except Exception as e:
            self.send_error(500, f"Error reading file: {e}")

    def handle_cgi(self):
        parsed_url = urllib.parse.urlparse(self.path)
        query_string = parsed_url.query
        query_params = urllib.parse.parse_qs(query_string)
        
        action = query_params.get('action', [None])[0]
        
        # Binary location
        exe_path = os.path.join(os.getcwd(), 'cgi-bin', 'backend.exe')
        
        # -------------------------------------------------------------
        # BRANCH A: IF compiled C++ backend.exe exists, route to C++!
        # -------------------------------------------------------------
        if os.path.exists(exe_path):
            # Read POST body
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = b''
            if content_length > 0:
                post_data = self.rfile.read(content_length)

            # Setup environment variables for CGI
            env = os.environ.copy()
            env['REQUEST_METHOD'] = self.command
            env['QUERY_STRING'] = query_string
            env['CONTENT_LENGTH'] = str(content_length)
            env['CONTENT_TYPE'] = self.headers.get('Content-Type', '')

            try:
                proc = subprocess.Popen(
                    [exe_path],
                    stdin=subprocess.PIPE,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    env=env
                )
                stdout_data, stderr_data = proc.communicate(input=post_data)

                if proc.returncode != 0:
                    self.send_response(500)
                    self.send_header('Content-Type', 'application/json')
                    self.end_headers()
                    err_msg = stderr_data.decode('utf-8', errors='ignore')
                    self.wfile.write(f'{{"status":"error","message":"C++ backend failed with code {proc.returncode}. {err_msg}"}}'.encode('utf-8'))
                    return

                # Parse C++ CGI Output
                parts = stdout_data.split(b'\r\n\r\n', 1)
                if len(parts) < 2:
                    parts = stdout_data.split(b'\n\n', 1)

                headers_part = parts[0]
                body_part = parts[1] if len(parts) > 1 else b''

                status_code = 200
                headers = {}
                for line in headers_part.decode('utf-8', errors='ignore').split('\n'):
                    line = line.strip()
                    if not line:
                        continue
                    if ':' in line:
                        k, v = line.split(':', 1)
                        k = k.strip().lower()
                        v = v.strip()
                        if k == 'status':
                            status_code = int(v.split()[0])
                        else:
                            headers[k] = v

                self.send_response(status_code)
                for k, v in headers.items():
                    if k not in ['content-length', 'connection']:
                        self.send_header(k, v)
                self.send_header('Content-Length', str(len(body_part)))
                self.end_headers()
                self.wfile.write(body_part)
                return

            except Exception as e:
                # Fallback silently to Python simulation if C++ execution fails
                pass

        # -------------------------------------------------------------
        # BRANCH B: PYTHON SIMULATION FALLBACK (Bulletproof, instant dev)
        # -------------------------------------------------------------
        try:
            # Read POST body
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = '{}'
            if content_length > 0:
                post_data = self.rfile.read(content_length).decode('utf-8', errors='ignore')

            # Parse JSON body
            body_data = {}
            if post_data:
                try:
                    body_data = json.loads(post_data)
                except:
                    # Fallback to query-like parsing if JSON malformed
                    body_data = urllib.parse.parse_qs(post_data)
                    body_data = {k: v[0] for k, v in body_data.items()}

            # Wipe out old seeded file values if they have old USD defaults, for instant PKR upgrade
            if action == "get_menu" and os.path.exists(MENU_FILE):
                with open(MENU_FILE, 'r') as check_f:
                    first_l = check_f.readline()
                    if "8.99" in first_l:
                        # Old USD configuration, delete files to recreate with premium PKR
                        check_f.close()
                        try:
                            os.remove(MENU_FILE)
                            os.remove(WALLETS_FILE)
                            os.remove(USERS_FILE)
                            os.remove(WALLET_TX_FILE)
                        except:
                            pass
                        initialize_default_files()

            response_data = None
            status_code = 200

            # --- Route Handler Simulation (matching backend.cpp exactly) ---
            if action == "get_menu":
                response_data = PythonCgiSimulator.get_menu()
            
            elif action == "login" and self.command == "POST":
                username = body_data.get("username")
                password = body_data.get("password")
                if not username or not password:
                    status_code = 400
                    response_data = {"status": "error", "message": "Username and password required."}
                else:
                    response_data = PythonCgiSimulator.login(username, password)
                    if response_data.get("status") == "error":
                        status_code = 401
            
            elif action == "register" and self.command == "POST":
                username = body_data.get("username")
                password = body_data.get("password")
                email = body_data.get("email")
                phone = body_data.get("phone", "")
                address = body_data.get("address", "")
                if not username or not password or not email:
                    status_code = 400
                    response_data = {"status": "error", "message": "Required fields missing."}
                else:
                    response_data = PythonCgiSimulator.register(username, password, email, phone, address)
            
            elif action == "place_order" and self.command == "POST":
                customerId = int(body_data.get("customerId", 0))
                items = body_data.get("items")
                totalAmount = float(body_data.get("totalAmount", 0.0))
                paymentMethod = body_data.get("paymentMethod", "Wallet")
                if customerId <= 0 or not items or totalAmount <= 0.0:
                    status_code = 400
                    response_data = {"status": "error", "message": "Invalid order data."}
                else:
                    response_data = PythonCgiSimulator.place_order(customerId, items, totalAmount, paymentMethod)
                    if response_data.get("status") == "error":
                        status_code = 400
            
            elif action == "get_order":
                orderId = int(query_params.get('orderId', [0])[0])
                response_data = PythonCgiSimulator.get_order(orderId)
                if response_data.get("status") == "error":
                    status_code = 404
            
            elif action == "get_customer_orders":
                customerId = int(query_params.get('customerId', [0])[0])
                response_data = PythonCgiSimulator.get_customer_orders(customerId)
            
            elif action == "add_wallet_balance" and self.command == "POST":
                customerId = int(body_data.get("customerId", 0))
                amount = float(body_data.get("amount", 0.0))
                method = body_data.get("method", "CreditCard")
                if customerId <= 0 or amount <= 0.0:
                    status_code = 400
                    response_data = {"status": "error", "message": "Invalid deposit details."}
                else:
                    response_data = PythonCgiSimulator.add_wallet_balance(customerId, amount, method)
            
            elif action == "get_notifications":
                customerId = int(query_params.get('customerId', [0])[0])
                response_data = PythonCgiSimulator.get_notifications(customerId)
            
            elif action == "mark_notifications_read" and self.command == "POST":
                customerId = int(body_data.get("customerId", 0))
                response_data = PythonCgiSimulator.mark_notifications_read(customerId)
            
            # --- Admin endpoints ---
            elif action == "add_menu_item" and self.command == "POST":
                name = body_data.get("name")
                description = body_data.get("description", "")
                price = float(body_data.get("price", 0.0))
                category = body_data.get("category")
                if not name or price <= 0.0 or not category:
                    status_code = 400
                    response_data = {"status": "error", "message": "Invalid item parameters."}
                else:
                    response_data = PythonCgiSimulator.add_menu_item(name, description, price, category)
            
            elif action == "update_menu_item" and self.command == "POST":
                itemId = int(body_data.get("id", 0))
                name = body_data.get("name")
                description = body_data.get("description", "")
                price = float(body_data.get("price", 0.0))
                category = body_data.get("category")
                isSoldOut = body_data.get("is_sold_out") == "1" or body_data.get("is_sold_out") is True
                if itemId <= 0 or not name or price <= 0.0 or not category:
                    status_code = 400
                    response_data = {"status": "error", "message": "Invalid update parameters."}
                else:
                    response_data = PythonCgiSimulator.update_menu_item(itemId, name, description, price, category, isSoldOut)
            
            elif action == "delete_menu_item" and self.command == "POST":
                itemId = int(body_data.get("id", 0))
                response_data = PythonCgiSimulator.delete_menu_item(itemId)
                if response_data.get("status") == "error":
                    status_code = 404
            
            elif action == "set_sold_out" and self.command == "POST":
                itemId = int(body_data.get("id", 0))
                isSoldOut = body_data.get("is_sold_out") == "1" or body_data.get("is_sold_out") is True
                response_data = PythonCgiSimulator.set_sold_out(itemId, isSoldOut)
            
            elif action == "get_all_orders":
                response_data = PythonCgiSimulator.get_all_orders()
            
            elif action == "update_order_status" and self.command == "POST":
                orderId = int(body_data.get("orderId", 0))
                status = body_data.get("status")
                response_data = PythonCgiSimulator.update_order_status(orderId, status)
            
            elif action == "update_prep_time" and self.command == "POST":
                orderId = int(body_data.get("orderId", 0))
                minutes = int(body_data.get("minutes", 15))
                response_data = PythonCgiSimulator.update_prep_time(orderId, minutes)
            
            elif action == "get_customers":
                response_data = PythonCgiSimulator.get_customers()
            
            elif action == "get_reviews":
                itemId_str = query_params.get('menuItemId', [None])[0]
                itemId = int(itemId_str) if itemId_str else None
                response_data = PythonCgiSimulator.get_reviews(itemId)
            
            elif action == "add_review" and self.command == "POST":
                customerId = int(body_data.get("customerId", 0))
                menuItemId = int(body_data.get("menuItemId", 0))
                rating = int(body_data.get("rating", 5))
                comment = body_data.get("comment", "")
                response_data = PythonCgiSimulator.add_review(customerId, menuItemId, rating, comment)
            
            elif action == "toggle_favorite" and self.command == "POST":
                customerId = int(body_data.get("customerId", 0))
                menuItemId = int(body_data.get("menuItemId", 0))
                response_data = PythonCgiSimulator.toggle_favorite(customerId, menuItemId)
            
            elif action == "get_favorites":
                customerId = int(query_params.get('customerId', [0])[0])
                response_data = PythonCgiSimulator.get_favorites(customerId)
            
            elif action == "get_announcements":
                response_data = PythonCgiSimulator.get_announcements()
            
            elif action == "add_announcement" and self.command == "POST":
                message = body_data.get("message")
                response_data = PythonCgiSimulator.add_announcement(message)
            
            elif action == "get_coupons":
                response_data = PythonCgiSimulator.get_coupons()
            
            elif action == "add_coupon" and self.command == "POST":
                code = body_data.get("code")
                discount = float(body_data.get("discount_percent", 0.0))
                response_data = PythonCgiSimulator.add_coupon(code, discount)
                if response_data.get("status") == "error":
                    status_code = 400
            
            elif action == "toggle_coupon" and self.command == "POST":
                code = body_data.get("code")
                is_active = body_data.get("is_active") == "1" or body_data.get("is_active") is True
                response_data = PythonCgiSimulator.toggle_coupon(code, is_active)
            
            elif action == "get_banners":
                response_data = PythonCgiSimulator.get_banners()
            
            elif action == "update_banner" and self.command == "POST":
                text = body_data.get("banner_text")
                img = body_data.get("banner_image_url")
                response_data = PythonCgiSimulator.update_banner(text, img)
            
            elif action == "get_sales_analytics":
                response_data = PythonCgiSimulator.get_sales_analytics()
            
            else:
                status_code = 404
                response_data = {"status": "error", "message": f"Action '{action}' not recognized."}

            self.send_response(status_code)
            self.send_header('Content-Type', 'application/json')
            
            body_bytes = json.dumps(response_data).encode('utf-8')
            self.send_header('Content-Length', str(len(body_bytes)))
            self.end_headers()
            self.wfile.write(body_bytes)

        except Exception as e:
            self.send_response(500)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            err_json = {"status": "error", "message": f"Simulation Error: {e}"}
            self.wfile.write(json.dumps(err_json).encode('utf-8'))

if __name__ == '__main__':
    # Initialize files immediately on startup so text files exist for study
    initialize_default_files()
    
    # Ensure cgi-bin directory exists
    os.makedirs('cgi-bin', exist_ok=True)
    
    server_address = ('', PORT)
    httpd = http.server.HTTPServer(server_address, CustomHTTPRequestHandler)
    print(f"STC Ordering System Server running on http://localhost:{PORT}")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping server.")
        sys.exit(0)
