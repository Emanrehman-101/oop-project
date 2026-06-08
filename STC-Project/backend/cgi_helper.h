#ifndef CGI_HELPER_H
#define CGI_HELPER_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <cstdlib>

using namespace std;

// Namespace to group CGI utilities
namespace CGI {

    // Helper to get environment variables safely
    inline string getEnv(const string& name) {
        char* value = getenv(name.c_str());
        return value ? string(value) : "";
    }

    // Decode URL-encoded strings (converts %20 to space, %40 to @, etc.)
    // Essential for reading form-urlencoded query strings
    inline string urlDecode(const string& src) {
        string dst;
        char ch;
        int i, ii;
        for (i = 0; i < (int)src.length(); i++) {
            if (src[i] == '%') {
                if (sscanf(src.substr(i + 1, 2).c_str(), "%x", &ii) == 1) {
                    ch = (char)ii;
                    dst += ch;
                    i += 2;
                }
            } else if (src[i] == '+') {
                dst += ' '; // URL query string encodes space as '+'
            } else {
                dst += src[i];
            }
        }
        return dst;
    }

    // Parse URL query string (e.g., "action=get_menu&category=Burgers") into key-value map
    inline map<string, string> parseQueryString(const string& query) {
        map<string, string> params;
        stringstream ss(query);
        string pair;
        while (getline(ss, pair, '&')) {
            size_t pos = pair.find('=');
            if (pos != string::npos) {
                string key = urlDecode(pair.substr(0, pos));
                string val = urlDecode(pair.substr(pos + 1));
                params[key] = val;
            } else if (!pair.empty()) {
                params[urlDecode(pair)] = "";
            }
        }
        return params;
    }

    // Beginner-friendly custom JSON parser for flat, single-level JSON payloads.
    // Handles payloads like: {"username": "john", "password": "123", "amount": 25.50}
    // Avoids complex external JSON libraries to keep the compile commands simple.
    inline map<string, string> parseJSON(const string& jsonStr) {
        map<string, string> result;
        if (jsonStr.empty()) return result;

        string key, val;
        bool inKey = false, inVal = false;
        bool insideQuotes = false;
        string currentToken = "";

        for (size_t i = 0; i < jsonStr.length(); ++i) {
            char c = jsonStr[i];

            // Toggle inside-quotes flag when finding unescaped quotation marks
            if (c == '"' && (i == 0 || jsonStr[i - 1] != '\\')) {
                insideQuotes = !insideQuotes;
                
                // When we exit quotes, we assign the accumulated token
                if (!insideQuotes) {
                    if (inKey) {
                        key = currentToken;
                        inKey = false;
                    } else if (inVal) {
                        val = currentToken;
                        inVal = false;
                        result[key] = val; // Store key-value pair
                        key = ""; // Reset key for the next pair
                    }
                } else {
                    // When entering quotes, determine if it is a key or a value
                    // If we just saw a colon or are on a new line, it's a value; otherwise, it's a key
                    // Let's decide based on whether we have a key already
                    if (key.empty()) {
                        inKey = true;
                    } else {
                        inVal = true;
                    }
                    currentToken = "";
                }
                continue;
            }

            if (insideQuotes) {
                currentToken += c;
            } else {
                // If we are not inside quotes, handle colons, commas, and numeric/boolean values
                if (c == ':') {
                    // Colon marks start of a value. If value is numeric/boolean, it won't have quotes.
                    // We check if the next non-whitespace char is not '"'
                    size_t nextIdx = i + 1;
                    while (nextIdx < jsonStr.length() && (jsonStr[nextIdx] == ' ' || jsonStr[nextIdx] == '\t' || jsonStr[nextIdx] == '\r' || jsonStr[nextIdx] == '\n')) {
                        nextIdx++;
                    }
                    if (nextIdx < jsonStr.length() && jsonStr[nextIdx] != '"') {
                        // Numeric or boolean value (no quotes)
                        inVal = true;
                        currentToken = "";
                    }
                } else if (c == ',' || c == '}') {
                    // Comma or closing brace marks end of a numeric/boolean value
                    if (inVal && !currentToken.empty()) {
                        // Trim trailing spaces in token
                        while (!currentToken.empty() && (currentToken.back() == ' ' || currentToken.back() == '\t' || currentToken.back() == '\r' || currentToken.back() == '\n')) {
                            currentToken.pop_back();
                        }
                        result[key] = currentToken;
                        inVal = false;
                        key = "";
                    }
                    currentToken = "";
                } else if (inVal && c != ' ' && c != '\t' && c != '\r' && c != '\n') {
                    currentToken += c;
                }
            }
        }
        return result;
    }

    // Helper to read the POST request body from stdin based on CONTENT_LENGTH
    inline string readPostBody() {
        string method = getEnv("REQUEST_METHOD");
        if (method != "POST") return "";

        string lenStr = getEnv("CONTENT_LENGTH");
        if (lenStr.empty()) return "";

        int contentLength = atoi(lenStr.c_str());
        if (contentLength <= 0) return "";

        string body = "";
        body.reserve(contentLength);

        char c;
        for (int i = 0; i < contentLength; ++i) {
            if (cin.get(c)) {
                body += c;
            } else {
                break; // Stream ended unexpectedly
            }
        }
        return body;
    }

    // Helper to print standard HTTP Headers and JSON body response
    // Includes CORS headers to prevent browser security blocks during development
    inline void printResponse(const string& jsonBody, int statusCode = 200) {
        string statusStr = "200 OK";
        if (statusCode == 400) statusStr = "400 Bad Request";
        else if (statusCode == 401) statusStr = "401 Unauthorized";
        else if (statusCode == 403) statusStr = "403 Forbidden";
        else if (statusCode == 404) statusStr = "404 Not Found";
        else if (statusCode == 500) statusStr = "500 Internal Server Error";

        // 1. Output HTTP Status Header (required by CGI)
        cout << "Status: " << statusStr << "\r\n";
        
        // 2. Output Content-Type (JSON)
        cout << "Content-Type: application/json\r\n";
        
        // 3. Output CORS headers for browser testing
        cout << "Access-Control-Allow-Origin: *\r\n";
        cout << "Access-Control-Allow-Methods: GET, POST, OPTIONS, PUT, DELETE\r\n";
        cout << "Access-Control-Allow-Headers: Content-Type\r\n";
        
        // 4. Blank line to mark end of HTTP response headers
        cout << "\r\n";
        
        // 5. Output JSON payload
        cout << jsonBody;
    }

    // Handle OPTIONS requests automatically (pre-flight requests from browser fetch)
    inline bool handleOptions() {
        if (getEnv("REQUEST_METHOD") == "OPTIONS") {
            cout << "Status: 200 OK\r\n";
            cout << "Access-Control-Allow-Origin: *\r\n";
            cout << "Access-Control-Allow-Methods: GET, POST, OPTIONS, PUT, DELETE\r\n";
            cout << "Access-Control-Allow-Headers: Content-Type\r\n";
            cout << "Content-Length: 0\r\n";
            cout << "\r\n";
            return true; // Request handled
        }
        return false;
    }

} // namespace CGI

#endif // CGI_HELPER_H
