
#include <gain_capital_api.h>
#include <json.hpp>
#include <cpr/cpr.h>
#include <boost/log/trivial.hpp>
#include <algorithm>
#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <chrono>
#include <map>

GCapiClient::GCapiClient(std::string username, std::string password, std::string apikey) {

    rest_url_v2 = "https://ciapi.cityindex.com/v2";
    rest_url = "https://ciapi.cityindex.com/TradingAPI";

    cpr::Header headers = cpr::Header{{"Content-Type", "application/json"}};
    nlohmann::json payload = {{"UserName", username}, {"Password", password},{"AppKey", apikey}};
    
    std::string status_code_error = "No Staus Code Error";
    std::string url = rest_url_v2 + "/Session";
    std::string resp_session;
    
    // ---------------------------
    for (int x; x < 4; x++) {
        try {
            cpr::Response r = cpr::Post(cpr::Url{url}, headers, cpr::Body{payload.dump()});

            if (r.status_code != 200) { throw r;}
            
            nlohmann::json resp = nlohmann::json::parse(r.text);

            if (resp["statusCode"] != 0) {

                std::string resp_status_code = resp["statusCode"];

                status_code_error = "Session Init Status Code Unusual: " + resp_status_code;

                BOOST_LOG_TRIVIAL(fatal) << status_code_error;
                std::cerr << status_code_error << std::endl;
                std::terminate();
            }
            resp_session = resp["session"];
            break;
        }
        catch (cpr::Response r) { 
            std::cerr << "Error - Status Code: " << r.status_code << "; Message: " << r.text << std::endl;
            BOOST_LOG_TRIVIAL(fatal) << "Error - Status Code: " << r.status_code << "; Message: " << r.text;
            if (x == 3) { std::terminate(); }
        }
        catch (...) { 
            sleep(1.5);
            if (x == 3) {
                BOOST_LOG_TRIVIAL(fatal) << "Session Request Didn't POST - Check Internet";
                std::cerr << "Session Request Didn't POST - Check Internet" << std::endl;
                std::terminate();
            }
        }
    }
    // ---------------------------
    session_header = {{"Content-Type","application/json"},{"UserName",username},{"Session",resp_session}};
}

nlohmann::json GCapiClient::get_account_info(std::string param = "") {
    // Gets trading account general information
    // :param: retrieve specific information (e.g. TradingAccountId)
    // :return: trading account information
    nlohmann::json resp;
    
    for (int x = 0; x < 3; x++) {
        try {
            cpr::Url url = cpr::Url{rest_url_v2 + "/userAccount/ClientAndTradingAccount"};
            cpr::Response r = cpr::Get(url, session_header);

            if (r.status_code != 200) { throw r;}

            resp = nlohmann::json::parse(r.text);

            trading_account_id = resp["tradingAccounts"][0]["tradingAccountId"].dump();
            client_account_id = resp["tradingAccounts"][0]["clientAccountId"].dump();

            if (!param.empty()) {
                return resp["tradingAccounts"][0][param];
            }
            break;
        }
        catch (cpr::Response r) { 
            std::cerr << "Account Info Error - Status Code: " << r.status_code << "; Message: " << r.text << std::endl;
            BOOST_LOG_TRIVIAL(fatal) << "Account Info Error - Status Code: " << r.status_code << "; Message: " << r.text;
            if (x == 2) { std::terminate(); }
        }
        catch (...) { 
            sleep(1);
            if (x == 2) {
                BOOST_LOG_TRIVIAL(fatal) << "Get Account Info Unknown Failure";
                std::cerr << "Get Account Info Unknown Failure" << std::endl;
                std::terminate();
            }
        }
    }
    // -------------------
    return resp;
}

nlohmann::json GCapiClient::get_margin_info(std::string param = "") {
    // Gets trading account margin information
    // :param: retrieve specific information (e.g. Cash)
	// :return: trading account margin information
    float equity_total = 0.0;
    float margin_total = 0.0;
    nlohmann::json resp;
    cpr::Url url = cpr::Url{rest_url_v2 + "/margin/clientAccountMargin?clientAccountId=" + client_account_id};

    for (int x; x < 3; x++) {
        try {
            cpr::Response r = cpr::Get(url, session_header);

            if (r.status_code != 200) { throw r;}

            nlohmann::json resp = nlohmann::json::parse(r.text);

            margin_total = resp["margin"];
            equity_total = resp["netEquity"];

            if (!param.empty()) {
                return resp[param];
            }
            break;
        }
        catch (cpr::Response r) { 
            std::cerr << "Margin Info Error - Status Code: " << r.status_code << "; Message: " << r.text << std::endl;
            BOOST_LOG_TRIVIAL(fatal) << "Margin Info Error - Status Code: " << r.status_code << "; Message: " << r.text;
            if (x == 2) { std::terminate(); }
        }
        catch (...) { 
            sleep(1);
            if (x == 2) {
                BOOST_LOG_TRIVIAL(fatal) << "Get Margin Info Uknown Failure";
                std::cerr << "Get Margin Info Unkown Failure" << std::endl;
                std::terminate();
            }
        }
    }
    // -------------------
    return resp;

}
    
std::map<std::string, int> GCapiClient::get_market_ids(std::vector<std::string> market_name_list) {
    // Gets market information 
    // :market_name_list: market name (e.g. USD/CAD) 
    // :return: market information
    market_id_map = {};
    int market_id;

    for (int x = 0; x < market_name_list.size(); x++) {
        std::string market_name = market_name_list[x];
        try {
            cpr::Url url = cpr::Url{rest_url + "/cfd/markets?MarketName=" + market_name};
            cpr::Response r = cpr::Get(url, session_header);

            if (r.status_code != 200) { throw r;}

            nlohmann::json resp = nlohmann::json::parse(r.text);

            market_id = resp["Markets"][0]["MarketId"];
            market_id_map[market_name] = market_id;
        }
        catch (cpr::Response r) { 
            std::cout << "Error Fetching Market ID for " << market_name << "; Status Code: " << r.status_code << "; Message: " << r.text << std::endl;
            BOOST_LOG_TRIVIAL(error) << "Error Fetching Market ID for " << market_name << "; Status Code: " << r.status_code << "; Message: " << r.text;
        }
        catch (...) { 
            BOOST_LOG_TRIVIAL(error) << "Uknown Error in Fetching Market Id for " << market_name;
            std::cout << "Uknown Error in Fetching Market Id for " << market_name << std::endl;
        }
    }
    // -------------------
    return market_id_map;
}

std::map<std::string, nlohmann::json> GCapiClient::get_market_info(std::vector<std::string> market_name_list, std::string param="") {
    // Gets market information 
    // :market_name_list: market name (e.g. USD/CAD) 
    // :param: retrieve specific information (e.g. MarketId)
    // :return: market information
    std::map<std::string, nlohmann::json> response_map;

    for (int x = 0; x < market_name_list.size(); x++) {
        std::string market_name = market_name_list[x];
        try {
            cpr::Url url = cpr::Url{rest_url + "/cfd/markets?MarketName=" + market_name};
            cpr::Response r = cpr::Get(url, session_header);

            if (r.status_code != 200) { throw r;}

            nlohmann::json resp = nlohmann::json::parse(r.text);

            if (!param.empty()) {
                response_map[market_name] = resp["Markets"][0]["MarketId"];
            } else {
                response_map[market_name] = resp["Markets"][0][param];
            }   
        }
        catch (cpr::Response r) { 
            std::cout << "Error Fetching Market ID for " << market_name << "; Status Code: " << r.status_code << "; Message: " << r.text << std::endl;
            BOOST_LOG_TRIVIAL(error) << "Error Fetching Market ID for " << market_name << "; Status Code: " << r.status_code << "; Message: " << r.text;
        }
        catch (...) { 
            BOOST_LOG_TRIVIAL(error) << "Uknown Error in Fetching Market Id for " << market_name;
            std::cout << "Uknown Error in Fetching Market Id for " << market_name << std::endl;
        }
    }
    // -------------------
    return response_map;
}

std::map<std::string, nlohmann::json> GCapiClient::get_prices(std::vector<std::string> market_name_list, int num_ticks=1, long unsigned int from_ts=0, long unsigned int to_ts=0, std::string price_type="MID") {
	// Get prices
    // :param market_name_list: market name (e.g. USD/CAD)
	// :param num_ticks: number of price ticks/data to retrieve
	// :param from_ts: from timestamp UTC
    // :param to_ts: to timestamp UTC
    // :return: price data    
    std::map<std::string, nlohmann::json> response_map = {};

    if (market_id_map.empty()) {
        get_market_ids(market_name_list);
    }

    std::transform(price_type.begin(), price_type.end(), price_type.begin(), ::toupper);
	
    for (int x = 0; x < market_name_list.size(); x++) {
        std::string market_name = market_name_list[x];
        std::string market_id;
        try {
            market_id = std::to_string(market_id_map[market_name]);
        }
        catch ( ... ) { 
            get_market_ids({market_name}); 
            market_id = std::to_string(market_id_map[market_name]);
        }

        // ---------------------------
        cpr::Url url;

        if (from_ts != 0 && to_ts != 0) {
            url = cpr::Url{rest_url + "/market/" + market_id + "/tickhistorybetween?fromTimeStampUTC=" + std::to_string(from_ts) + "&toTimestampUTC=" + std::to_string(to_ts) + "&priceType=" + price_type};
        }
        else if (to_ts != 0) {
            url = cpr::Url{rest_url + "/market/" + market_id + "/tickhistorybefore?maxResults=" + std::to_string(num_ticks) + "&toTimestampUTC=" + std::to_string(to_ts) + "&priceType=" + price_type};
        }
        else if (from_ts != 0) {
            url = cpr::Url{rest_url + "/market/" + market_id + "/tickhistoryafter?maxResults=" + std::to_string(num_ticks) + "&fromTimestampUTC=" + std::to_string(from_ts) + "&priceType=" + price_type};
        }
        else {
            url = cpr::Url{rest_url + "/market/" + market_id + "/tickhistory?PriceTicks=" + std::to_string(num_ticks) + "&priceType=" + price_type};
        }
        
        for (int j = 0; j < 3; j++) {
            try {
                cpr::Response r = cpr::Get(url, session_header);

                if (r.status_code != 200) { throw r;}

                nlohmann::json resp = nlohmann::json::parse(r.text);

                response_map[market_name] = resp["PriceTicks"]; 
                break;
            }
            catch (cpr::Response r) { 
                response_map[market_name] = {"Error"};
                std::cout << "Error Fetching Price for " << market_name << "; Status Code: " << r.status_code << "; Message: " << r.text << std::endl;
                BOOST_LOG_TRIVIAL(error) << "Error Fetching Price for " << market_name << "; Status Code: " << r.status_code << "; Message: " << r.text;
            }
            catch (...) {
                if (j == 2) { 
                    response_map[market_name] = {"Error"};
                }
                else {
                    sleep(1+j);
                }
            }
        }
    }
    // -------------------
    return response_map;
}

std::map<std::string, nlohmann::json> GCapiClient::get_ohlc(std::vector<std::string> market_name_list, int num_ticks=1, std::string interval, int span=1, long unsigned int from_ts=0, long unsigned int to_ts=0) {
    // Get the open, high, low, close of a specific market_id
	// :param market_name_list: market name (e.g. USD/CAD)
	// :param num_ticks: number of price ticks/data to retrieve
	// :param interval: MINUTE, HOUR or DAY tick interval
	// :param span: it can be a combination of span with interval, 1Hour, 15 MINUTE
	// :param from_ts: from timestamp UTC
	// :param to_ts: to timestamp UTC
	// :return: ohlc dataframe
    std::map<std::string, nlohmann::json> response_map = {};

    if (market_id_map.empty()) {
        get_market_ids(market_name_list);
    }

    std::vector<int> SPAN_M = {1, 2, 3, 5, 10, 15, 30}; // Span intervals for minutes
    std::vector<int> SPAN_H = {1, 2, 4, 8}; // Span intervals for hours
    std::vector<std::string> INTERVAL = {"HOUR", "MINUTE", "DAY", "WEEK", "MONTH"};

    if (std::find(INTERVAL.begin(), INTERVAL.end(), interval) == INTERVAL.end()) {
        std::cerr << "Interval Error - Provide one of the following intervals: 'HOUR', 'MINUTE', 'DAY', 'WEEK', 'MONTH'" << std::endl;
        std::terminate();
    }

    if (interval == "HOUR") {
        if (std::find(SPAN_H.begin(), SPAN_H.end(), span) == SPAN_H.end()) {
            std::cerr << "Span Hour Error - Provide one of the following spans: 1, 2, 4, 8" << std::endl;
            std::terminate();
        }
    }
    else if (interval == "MINUTE") {
        if (std::find(SPAN_M.begin(), SPAN_M.end(), span) == SPAN_M.end()) {
            std::cerr << "Span Minute Error - Provide one of the following spans: 1, 2, 3, 5, 10, 15, 30" << std::endl;
            std::terminate();
        }
    } else { span = 1; }

    for (int x = 0; x < market_name_list.size(); x++) {
        std::string market_name = market_name_list[x];
        std::string market_id;
        try {
            market_id = std::to_string(market_id_map[market_name]);
        }
        catch ( ... ) { 
            get_market_ids({market_name}); 
            market_id = std::to_string(market_id_map[market_name]);
        }

        // ---------------------------
        cpr::Url url; 

        if (from_ts != 0 && to_ts != 0) {
            url = cpr::Url{rest_url + "/market/" + market_id + "/barhistorybetween?interval=" + interval + "&span=" + std::to_string(span) + "&fromTimeStampUTC=" + std::to_string(from_ts) + "&toTimestampUTC=" + std::to_string(to_ts)};
        }
        else if (to_ts != 0) {
            url = cpr::Url{rest_url + "/market/" + market_id + "/barhistorybefore?interval=" + interval + "&span=" + std::to_string(span) + "&maxResults=" + std::to_string(num_ticks) + "&toTimestampUTC=" + std::to_string(to_ts)};
        }
        else if (from_ts != 0) {
            url = cpr::Url{rest_url + "/market/" + market_id + "/barhistoryafter?interval=" + interval + "&span=" + std::to_string(span) + "&maxResults=" + std::to_string(num_ticks) + "&fromTimestampUTC=" + std::to_string(from_ts)};
        }
        else {
            url = cpr::Url{rest_url + "/market/" + market_id + "/barhistory?interval=" + interval + "&span=" + std::to_string(span) + "&PriceBars=" + std::to_string(num_ticks)};
        }
        
        for (int j = 0; j < 3; j++) {
            try {
                cpr::Response r = cpr::Get(url, session_header);

                if (r.status_code != 200) { throw r;}

                nlohmann::json resp = nlohmann::json::parse(r.text);

                response_map[market_name] = resp["PriceBars"]; 
                break;
            }
            catch (cpr::Response r) { 
                response_map[market_name] = {"Error"};
                std::cout << "Error Fetching OHLC for " << market_name << "; Status Code: " << r.status_code << "; Message: " << r.text << std::endl;
                BOOST_LOG_TRIVIAL(error) << "Error Fetching OHLC for " << market_name << "; Status Code: " << r.status_code << "; Message: " << r.text;
            }
            catch (...) {
                if (j == 2) { 
                    response_map[market_name] = {"Error"};
                }
                else {
                    sleep(1+j);
                }
            }
        }
    }
    // -------------------
    return response_map;
}

std::vector<std::string> GCapiClient::trade_market_order(nlohmann::json trade_map, std::vector<std::string> market_name_list, std::string tr_account_id="") {
    // Makes a new trade market order
    // :param trade_map: JSON object formated as in the example below
	// :param market_name_list: market name (e.g. USD/CAD)
	// :param trading_acc_id: trading account ID
	// :return: error_list: list of symbol name that failed ot place trade
    // TRADE_MAP = {{"MARKET_NAME",{{"Direction","buy/sell"},{"Quantity","1000"}}}}   
    std::vector<std::string> error_list = {};

    if (tr_account_id.empty()) { tr_account_id = trading_account_id; }

    int offset_seconds = 10;
    unsigned int stop_time = (std::chrono::system_clock::now().time_since_epoch()).count() * std::chrono::system_clock::period::num / std::chrono::system_clock::period::den + offset_seconds;
    unsigned int current_time = (std::chrono::system_clock::now().time_since_epoch()).count() * std::chrono::system_clock::period::num / std::chrono::system_clock::period::den;

    cpr::Url url = cpr::Url{rest_url + "/order/newtradeorder"};

    if (market_id_map.empty()) {
        get_market_ids(market_name_list);
    }

    while (!market_name_list.empty() && current_time <= stop_time) {
        // -----------------------
        error_list = {};
        sleep(0.5);

        for (int x = 0; x < market_name_list.size(); x++) {
            std::string market_name = market_name_list[x];
            std::string market_id;
            try {
                market_id = std::to_string(market_id_map[market_name]);
            }
            catch ( ... ) { 
                get_market_ids({market_name}); 
                market_id = std::to_string(market_id_map[market_name]);
            }

            float bid_price = get_prices({market_name},1,0,0,"BID")[0];
            float offer_price = get_prices({market_name},1,0,0,"ASK")[0];
            // ---------------------------

            try {
                nlohmann::json trade_payload = {
                    {"Direction", trade_map[market_name]["Direction"]},
                    // {"AuditId", audit_id},
                    {"MarketId", market_id},
                    {"Quantity", trade_map[market_name]["Quantity"]},
                    {"MarketName", market_name},
                    {"TradingAccountId", tr_account_id},
                    {"OfferPrice", std::to_string(offer_price)},
                    {"BidPrice", std::to_string(bid_price)},
                    {"PriceTolerance", "0"}
                };                
            
                cpr::Response r = cpr::Post(url, session_header, cpr::Body(trade_payload.dump()));

                if (r.status_code == 200) {

                    nlohmann::json resp = nlohmann::json::parse(r.text);

                    BOOST_LOG_TRIVIAL(debug) << "Order Response: " << market_name << " " << resp;
                    // ---------------------------
                    if (resp["OrderId"] == "0") { 
                        error_list.push_back(market_name);
                    }
                } else {
                     error_list.push_back(market_name);
                     std::cout << "Order Response: Status Code:" << r.status_code << "; Message:  " << r.text << std::endl;
                     BOOST_LOG_TRIVIAL(debug) << "Order Response: Status Code:" << r.status_code << "; Message:  " << r.text;
                }
            }
            catch (...) { 
                std::cout << "Error Placing Order for " << market_name << std::endl;
                BOOST_LOG_TRIVIAL(error) << "Error Placing Order for " << market_name;
            }
        }
        // -----------------------
        market_name_list = error_list;
        current_time = (std::chrono::system_clock::now().time_since_epoch()).count() * std::chrono::system_clock::period::num / std::chrono::system_clock::period::den;
    }
    // -------------------
    return error_list;
}

std::vector<std::string> GCapiClient::trade_limit_order(nlohmann::json trade_map, std::vector<std::string> market_name_list, std::string tr_account_id="") {
    // Makes a new trade limit order
    // :param trade_map: JSON object formated as in the example below
	// :param market_name_list: market name (e.g. USD/CAD)
	// :param trading_acc_id: trading account ID
	// :return: error_list: list of symbol name that failed ot place trade
    // TRADE_MAP = {{"MARKET_NAME",{{"Direction","buy/sell"},{"Quantity","1000"},{}}}}
    std::vector<std::string> error_list = {};

    if (tr_account_id.empty()) { tr_account_id = trading_account_id; }

    int offset_seconds = 10;
    unsigned int stop_time = (std::chrono::system_clock::now().time_since_epoch()).count() * std::chrono::system_clock::period::num / std::chrono::system_clock::period::den + offset_seconds;
    unsigned int current_time = (std::chrono::system_clock::now().time_since_epoch()).count() * std::chrono::system_clock::period::num / std::chrono::system_clock::period::den;

    cpr::Url url = cpr::Url{rest_url + "/order/newstoplimitorder"};

    if (market_id_map.empty()) {
        get_market_ids(market_name_list);
    }

    while (!market_name_list.empty() && current_time <= stop_time) {
        // -----------------------
        error_list = {};
        sleep(0.5);

        for (int x = 0; x < market_name_list.size(); x++) {
            std::string market_name = market_name_list[x];
            std::string market_id;
            try {
                market_id = std::to_string(market_id_map[market_name]);
            }
            catch ( ... ) { 
                get_market_ids({market_name}); 
                market_id = std::to_string(market_id_map[market_name]);
            }

            float bid_price = get_prices({market_name},1,0,0,"BID")[0];
            float offer_price = get_prices({market_name},1,0,0,"ASK")[0];
            // ---------------------------

            try {
                nlohmann::json trade_payload = {
                    {"Direction", trade_map[market_name]["Direction"]},
                    // {"AuditId", audit_id},
                    {"MarketId", market_id},
                    {"Quantity", trade_map[market_name]["Quantity"]},
                    {"MarketName", market_name},
                    {"TradingAccountId", tr_account_id},
                    {"OfferPrice", std::to_string(offer_price)},
                    {"BidPrice", std::to_string(bid_price)},
                    {"PriceTolerance", "0"}
                };                
            
                cpr::Response r = cpr::Post(url, session_header, cpr::Body(trade_payload.dump()));

                if (r.status_code == 200) {

                    nlohmann::json resp = nlohmann::json::parse(r.text);

                    BOOST_LOG_TRIVIAL(debug) << "Order Response: " << market_name << " " << resp;
                    // ---------------------------
                    if (resp["OrderId"] == "0") { 
                        error_list.push_back(market_name);
                    }
                } else {
                     error_list.push_back(market_name);
                     std::cout << "Order Response: Status Code:" << r.status_code << "; Message:  " << r.text << std::endl;
                     BOOST_LOG_TRIVIAL(debug) << "Order Response: Status Code:" << r.status_code << "; Message:  " << r.text;
                }
            }
            catch (...) { 
                std::cout << "Error Placing Order for " << market_name << std::endl;
                BOOST_LOG_TRIVIAL(error) << "Error Placing Order for " << market_name;
            }
        }
        // -----------------------
        market_name_list = error_list;
        current_time = (std::chrono::system_clock::now().time_since_epoch()).count() * std::chrono::system_clock::period::num / std::chrono::system_clock::period::den;
    }
    // -------------------
    return error_list;
}

nlohmann::json GCapiClient::list_open_positions(std::string tr_account_id = "") {
    // List of Open Positons in Trading Account
    // :param trading_acc_id: trading account ID
    // :return JSON response
    nlohmann::json resp;
    if (tr_account_id.empty()) { tr_account_id = trading_account_id; }
    cpr::Url url = cpr::Url{rest_url + "/order/openpositions?TradingAccountId=" + tr_account_id};

    for (int x = 0; x < 3; x++) {
        try {
            cpr::Response r = cpr::Get(url, session_header);

            if (r.status_code != 200) { throw r;}

            resp = nlohmann::json::parse(r.text);
            resp = resp["OpenPositions"];
            break;
        }
        catch (cpr::Response r) { 
            std::cerr << "Open Positions Error - Status Code: " << r.status_code << "; Message: " << r.text << std::endl;
            BOOST_LOG_TRIVIAL(fatal) << "Open Positions Error - Status Code: " << r.status_code << "; Message: " << r.text;
            if (x == 2) { std::terminate(); }
        }
        catch (...) { 
            sleep(1);
            if (x == 2) {
                BOOST_LOG_TRIVIAL(fatal) << "List Open Position Uknown Failure";
                std::cerr << "List Open Position Uknown Failure" << std::endl;
                std::terminate();
            }
        }
    }
    // -------------------
    return resp;
}

nlohmann::json GCapiClient::list_active_orders(std::string tr_account_id = "") {
    // List of Active Order in Trading Account
    // :param trading_acc_id: trading account ID
    // :return JSON response
    nlohmann::json resp;
    if (tr_account_id.empty()) { tr_account_id = trading_account_id; }
    cpr::Url url = cpr::Url{rest_url + "/order/activeorders"};
    nlohmann::json active_order_payload = {{"TradingAccountId",tr_account_id}, {"MaxResults", "100"}};

    for (int x = 0; x < 3; x++) {
        try {
            cpr::Response r = cpr::Post(url, session_header, cpr::Body{active_order_payload.dump()});

            if (r.status_code != 200) { throw r;}

            resp = nlohmann::json::parse(r.text);
            resp = resp["ActiveOrders"];
            break;
        }
        catch (cpr::Response r) { 
            std::cerr << "Active Orders Error - Status Code: " << r.status_code << "; Message: " << r.text << std::endl;
            BOOST_LOG_TRIVIAL(fatal) << "Active Orders Error - Status Code: " << r.status_code << "; Message: " << r.text;
            if (x == 2) { std::terminate(); }
        }
        catch (...) { 
            sleep(1);
            if (x == 2) {
                BOOST_LOG_TRIVIAL(fatal) << "List Active Order Failure " << resp;
                std::cerr << "List Active Order Failure " << resp << std::endl;
                std::terminate();
            }
        }
    }
    // -------------------
    return resp;
}

nlohmann::json GCapiClient::cancel_order(std::string order_id, std::string tr_account_id="") {
    // Cancels an Active Order
    // :order_id: Order ID of the Order to Cancel
    // :param trading_acc_id: trading account ID
    // :return JSON response
    nlohmann::json resp;
    if (tr_account_id.empty()) { tr_account_id = trading_account_id; }
    cpr::Url url = cpr::Url{rest_url + "/order/cancel"};
    nlohmann::json cancel_order_payload = {{"TradingAccountId", tr_account_id},{"OrderId", order_id}};

    for (int x = 0; x < 3; x++) {
        try {
            cpr::Response r = cpr::Post(url, session_header, cpr::Body{cancel_order_payload.dump()});

            if (r.status_code != 200) { throw r;}

            resp = nlohmann::json::parse(r.text);
            break;
        }
        catch (cpr::Response r) { 
            std::cerr << "Cancel Order Error - Status Code: " << r.status_code << "; Message: " << r.text << std::endl;
            BOOST_LOG_TRIVIAL(fatal) << "Cancel Order Error - Status Code: " << r.status_code << "; Message: " << r.text;
            if (x == 2) { std::terminate(); }
        }
        catch (...) { 
            sleep(1);
            if (x == 2) {
                BOOST_LOG_TRIVIAL(fatal) << "Cancel Order Uknown Failure";
                std::cerr << "Cancel Order Unknown Failure" << std::endl;
                std::terminate();
            }
        }
    }
    // -------------------
    return resp;
}