// Copyright 2024, Andrew Drogalis
// GNU License

#include "gain_capital_api.h"

#include <algorithm>       // for transform
#include <bits/chrono.h>   // for system_clock
#include <ctype.h>         // for toupper
#include <exception>       // for exception
#include <initializer_list>// for initialize...
#include <iostream>        // for operator<<
#include <string>          // for basic_string
#include <unistd.h>        // for sleep
#include <unordered_map>   // for unordered_map
#include <utility>         // for pair
#include <vector>          // for vector

#include "boost/log/trivial.hpp"                        // for BOOST_LOG_...
#include "boost/log/utility/setup/common_attributes.hpp"// for add_common...
#include "boost/log/utility/setup/console.hpp"          // for add_consol...
#include "boost/log/utility/setup/file.hpp"             // for add_file_log
#include "cpr/api.h"                                    // for Get, Post
#include "cpr/body.h"                                   // for Body
#include "cpr/response.h"                               // for Response
#include "json/json.hpp"                                // for json_ref

namespace gaincapital
{

GCapiClient::GCapiClient(std::string const& username, std::string const& password, std::string const& apikey)
{
    auth_payload = {{"UserName", username}, {"Password", password}, {"AppKey", apikey}};
}

// =================================================================================================================
// AUTHENTICATION
// =================================================================================================================

bool GCapiClient::authenticate_session()
{
    /* * This is the first authentication of the user.
     * This method MUST run before any other API request. */
    if (! validate_auth_payload()) { return false; }

    cpr::Header const headers {{"Content-Type", "application/json"}};
    cpr::Url const url {rest_url_v2 + "/Session"};
    // ---------------------------
    nlohmann::json resp = make_network_call(headers, url, auth_payload.dump(), "POST", "Authenticate Session");
    if (! resp.contains("statusCode") || ! resp.contains("session")) { return false; }

    if (! resp["statusCode"].is_number_integer() || resp["statusCode"] != 0)
    {
        BOOST_LOG_TRIVIAL(fatal) << "API Response is Valid, but Gain Capital Status Code Error: " << resp["statusCode"];
        return false;
    }
    // ---------------------------
    session_header = {{"Content-Type", "application/json"}, {"UserName", auth_payload["UserName"]}, {"Session", resp["session"].dump()}};
    if (! set_trading_account_id()) { return false; }
    // [No Errors] Response Good
    return true;
}

bool GCapiClient::set_trading_account_id()
{
    /* * Sets the member variables CLASS_trading_account_id & CLASS_client_account_id. */
    cpr::Url const url {rest_url_v2 + "/userAccount/ClientAndTradingAccount"};
    // ---------------------------
    nlohmann::json resp = make_network_call(session_header, url, "", "GET", "Set Trading Account ID");

    CLASS_trading_account_id = resp["tradingAccounts"][0]["tradingAccountId"].dump();
    CLASS_client_account_id = resp["tradingAccounts"][0]["clientAccountId"].dump();

    if (CLASS_trading_account_id == "null" || CLASS_client_account_id == "null")
    {
        BOOST_LOG_TRIVIAL(fatal) << "Set Trading Account ID - JSON Key Error - Response: " << resp;
        return false;
    }
    // [No Errors] Response Good
    return true;
}

bool GCapiClient::validate_session()
{
    /* * Validates current session and updates if token expired. */
    if (! validate_session_header()) { return false; }

    nlohmann::json payload = {{"ClientAccountId", CLASS_client_account_id},
                              {"UserName", session_header["Username"]},
                              {"Session", session_header["Session"]},
                              {"TradingAccountId", CLASS_trading_account_id}};
    cpr::Url const url {rest_url_v2 + "/Session/validate"};

    nlohmann::json resp = make_network_call(session_header, url, payload.dump(), "POST", "Validate Session");

    if (resp["isAuthenticated"].dump() != "true" && ! authenticate_session()) { return false; }
    // [No Errors] Response Good
    return true;
}

// =================================================================================================================
// API CALLS
// =================================================================================================================
nlohmann::json GCapiClient::get_account_info(std::string const& param)
{
    /* Gets the trading account's general information.
       :param: retrieve specific information (e.g. TradingAccountId)
       :return: trading account information
    */
    nlohmann::json resp;
    // -------------------
    if (! validate_session()) { return resp; }

    cpr::Url const url {rest_url_v2 + "/userAccount/ClientAndTradingAccount"};

    resp = make_network_call(session_header, url, "", "GET", "Get Account Info");

    if (! param.empty() && resp["tradingAccounts"][0].contains(param)) { return resp["tradingAccounts"][0][param]; }
    // -------------------
    return resp;
}

nlohmann::json GCapiClient::get_margin_info(std::string const& param)
{
    /*  Gets the trading account's margin information.
        :param: retrieve specific information (e.g. Cash)
        :return: trading account margin information
        -- Example of param options
        float margin_total = resp["margin"];
        float equity_total = resp["netEquity"];
    */
    nlohmann::json resp;
    // -------------------
    if (! validate_session()) { return resp; }

    cpr::Url const url {rest_url_v2 + "/margin/clientAccountMargin?clientAccountId=" + CLASS_client_account_id};

    resp = make_network_call(session_header, url, "", "GET", "Get Margin Info");

    if (! param.empty() && resp.contains(param)) { return resp[param]; }
    // -------------------
    return resp;
}

std::unordered_map<std::string, std::string> GCapiClient::get_market_ids(std::vector<std::string> const& market_name_list)
{
    /* Gets the market information.
       :market_name_list: market name (e.g. USD/CAD)
       :return: market information
    */
    if (! validate_session()) { return market_id_map; }

    for (auto const& market_name : market_name_list)
    {
        cpr::Url const url {rest_url + "/cfd/markets?MarketName=" + market_name};

        nlohmann::json resp = make_network_call(session_header, url, "", "GET", "Get Market IDs - " + market_name);

        std::string market_id = resp["Markets"][0]["MarketId"].dump();

        if (market_id != "null") { market_id_map[market_name] = market_id; }
    }
    // -------------------
    return market_id_map;
}

std::unordered_map<std::string, std::string> GCapiClient::get_market_info(std::vector<std::string> const& market_name_list, std::string const& param)
{
    /* Gets the market information.
       :market_name_list: market name (e.g. USD/CAD)
       :param: retrieve specific information (e.g. MarketId)
       :return: market information
    */
    std::unordered_map<std::string, std::string> response_map;
    // -------------------
    if (! validate_session()) { return response_map; }

    for (auto const& market_name : market_name_list)
    {
        cpr::Url const url {rest_url + "/cfd/markets?MarketName=" + market_name};

        nlohmann::json resp = make_network_call(session_header, url, "", "GET", "Get Market Info - " + market_name);

        if (resp.empty()) { continue; }

        if (param.empty()) { response_map[market_name] = resp["Markets"][0]["MarketId"].dump(); }
        else if (! param.empty() && resp["Markets"][0].contains(param)) { response_map[market_name] = resp["Markets"][0][param].dump(); }
        else { response_map[market_name] = resp["Markets"][0].dump(); }
    }
    // -------------------
    return response_map;
}

std::unordered_map<std::string, nlohmann::json> GCapiClient::get_prices(std::vector<std::string> const& market_name_list, std::size_t const num_ticks,
                                                                        std::size_t const from_ts, std::size_t const to_ts, std::string price_type)
{
    /*  Get prices
        :param market_name_list: market name (e.g. USD/CAD)
        :param num_ticks: number of price ticks/data to retrieve
        :param from_ts: from timestamp UTC
        :param to_ts: to timestamp UTC
        :return: price data
  */
    std::unordered_map<std::string, nlohmann::json> response_map;
    // -------------------
    if (! validate_session()) { return response_map; }

    std::transform(price_type.begin(), price_type.end(), price_type.begin(), ::toupper);

    if (price_type != "BID" && price_type != "ASK" && price_type != "MID")
    {
        BOOST_LOG_TRIVIAL(error) << "Price Type Error - Provide one of the following price types: 'ASK', 'BID', 'MID'";
        return response_map;
    }

    for (auto const& market_name : market_name_list)
    {
        std::string market_id = "";
        if (market_id_map.count(market_name)) { market_id = market_id_map[market_name]; }
        else
        {
            auto response = get_market_ids({market_name});
            if (response.contains(market_name)) { market_id = market_id_map[market_name]; }
            else
            {
                BOOST_LOG_TRIVIAL(error) << "Failure Fetching Market ID for Market - " << market_name;
                continue;
            }
        }
        // ---------------------------
        cpr::Url url;
        if (from_ts != 0 && to_ts != 0)
        {
            url = cpr::Url {rest_url + "/market/" + market_id + "/tickhistorybetween?fromTimeStampUTC=" + std::to_string(from_ts) +
                            "&toTimestampUTC=" + std::to_string(to_ts) + "&priceType=" + price_type};
        }
        else if (to_ts != 0)
        {
            url = cpr::Url {rest_url + "/market/" + market_id + "/tickhistorybefore?maxResults=" + std::to_string(num_ticks) +
                            "&toTimestampUTC=" + std::to_string(to_ts) + "&priceType=" + price_type};
        }
        else if (from_ts != 0)
        {
            url = cpr::Url {rest_url + "/market/" + market_id + "/tickhistoryafter?maxResults=" + std::to_string(num_ticks) +
                            "&fromTimestampUTC=" + std::to_string(from_ts) + "&priceType=" + price_type};
        }
        else
        {
            url = cpr::Url {rest_url + "/market/" + market_id + "/tickhistory?PriceTicks=" + std::to_string(num_ticks) + "&priceType=" + price_type};
        }
        nlohmann::json resp = make_network_call(session_header, url, "", "GET", "Get Prices - " + market_name);

        if (! resp.empty() && resp.contains("PriceTicks")) { response_map[market_name] = resp["PriceTicks"]; }
    }
    // -------------------
    return response_map;
}

std::unordered_map<std::string, nlohmann::json> GCapiClient::get_ohlc(std::vector<std::string> const& market_name_list, std::string interval,
                                                                      std::size_t const num_ticks, std::size_t span, std::size_t const from_ts,
                                                                      std::size_t const to_ts)
{
    /*  Get the open, high, low, close of a specific market_id
        :param market_name_list: market name (e.g. USD/CAD)
        :param num_ticks: number of price ticks/data to retrieve
        :param interval: MINUTE, HOUR or DAY tick interval
        :param span: it can be a combination of span with interval, 1Hour, 15 MINUTE
        :param from_ts: from timestamp UTC :param to_ts: to timestamp UTC
        :return: ohlc dataframe
    */
    std::unordered_map<std::string, nlohmann::json> response_map;
    // -------------------
    if (! validate_session()) { return response_map; }

    std::transform(interval.begin(), interval.end(), interval.begin(), ::toupper);

    std::vector<int> const SPAN_M = {1, 2, 3, 5, 10, 15, 30};// Span intervals for minutes
    std::vector<int> const SPAN_H = {1, 2, 4, 8};            // Span intervals for hours
    std::vector<std::string> const INTERVAL = {"HOUR", "MINUTE", "DAY", "WEEK", "MONTH"};

    if (std::find(INTERVAL.begin(), INTERVAL.end(), interval) == INTERVAL.end())
    {
        BOOST_LOG_TRIVIAL(error) << "Interval Error - Provide one of the following intervals: 'HOUR', "
                                    "'MINUTE', 'DAY', 'WEEK', 'MONTH'";
        return response_map;
    }
    if (interval == "HOUR")
    {
        if (std::find(SPAN_H.begin(), SPAN_H.end(), span) == SPAN_H.end())
        {
            BOOST_LOG_TRIVIAL(error) << "Span Hour Error - Provide one of the "
                                        "following spans: 1, 2, 4, 8";
            return response_map;
        }
    }
    else if (interval == "MINUTE")
    {
        if (std::find(SPAN_M.begin(), SPAN_M.end(), span) == SPAN_M.end())
        {
            BOOST_LOG_TRIVIAL(error) << "Span Minute Error - Provide one of the "
                                        "following spans: 1, 2, 3, 5, 10, 15, 30";
            return response_map;
        }
    }
    else { span = 1; }

    for (auto const& market_name : market_name_list)
    {
        std::string market_id;
        if (market_id_map.count(market_name)) { market_id = market_id_map[market_name]; }
        else
        {
            auto response = get_market_ids({market_name});
            if (response.contains(market_name)) { market_id = market_id_map[market_name]; }
            else
            {
                BOOST_LOG_TRIVIAL(error) << "Failure Fetching Market ID for Market - " << market_name;
                continue;
            }
        }
        // ---------------------------
        cpr::Url url;
        if (from_ts != 0 && to_ts != 0)
        {
            url = cpr::Url {rest_url + "/market/" + market_id + "/barhistorybetween?interval=" + interval + "&span=" + std::to_string(span) +
                            "&fromTimeStampUTC=" + std::to_string(from_ts) + "&toTimestampUTC=" + std::to_string(to_ts)};
        }
        else if (to_ts != 0)
        {
            url = cpr::Url {rest_url + "/market/" + market_id + "/barhistorybefore?interval=" + interval + "&span=" + std::to_string(span) +
                            "&maxResults=" + std::to_string(num_ticks) + "&toTimestampUTC=" + std::to_string(to_ts)};
        }
        else if (from_ts != 0)
        {
            url = cpr::Url {rest_url + "/market/" + market_id + "/barhistoryafter?interval=" + interval + "&span=" + std::to_string(span) +
                            "&maxResults=" + std::to_string(num_ticks) + "&fromTimestampUTC=" + std::to_string(from_ts)};
        }
        else
        {
            url = cpr::Url {rest_url + "/market/" + market_id + "/barhistory?interval=" + interval + "&span=" + std::to_string(span) +
                            "&PriceBars=" + std::to_string(num_ticks)};
        }
        nlohmann::json resp = make_network_call(session_header, url, "", "GET", "Get OHLC - " + market_name);

        if (! resp.empty() && resp.contains("PriceBars")) { response_map[market_name] = resp["PriceBars"]; }
    }
    // -------------------
    return response_map;
}

std::vector<std::string> GCapiClient::trade_order(nlohmann::json& trade_map, std::string type, std::string tr_account_id)
{
    /*  Makes a new trade order
        :param trade_map: JSON object formatted as shown in the example below
        :param type: Limit or Market order type
        :param trading_acc_id: trading account ID
        :return: error_list: list of symbol names that failed to be executed
        // Market Order
        TRADE_MAP = {{"MARKET_NAME",{
            {"Direction","buy/sell"},
            {"Quantity","1000"}}
            }}
        // Limit Order
        TRADE_MAP = {{"MARKET_NAME",{
            {"Direction","buy/sell"},
            {"Quantity","1000"},
            {"TriggerPrice","1.3245"},
            {'StopPrice", "1.3010 or 0 for None"},
            {'LimitPrice", "1.3521 or 0 for None"}}
            }}
    */
    std::vector<std::string> error_list, input_error_list, market_name_list;

    for (nlohmann::json::iterator it = trade_map.begin(); it != trade_map.end(); ++it) { market_name_list.push_back(it.key()); }
    // -------------------
    if (! validate_session()) { return market_name_list; }

    if (tr_account_id.empty()) { tr_account_id = CLASS_trading_account_id; }

    std::transform(type.begin(), type.end(), type.begin(), ::toupper);

    if (type != "MARKET" && type != "LIMIT")
    {
        BOOST_LOG_TRIVIAL(error) << "Trade Order Type Must Be 'MARKET' or 'LIMIT'";
        return market_name_list;
    }

    std::size_t current_time = (std::chrono::system_clock::now().time_since_epoch()).count() * std::chrono::system_clock::period::num /
                               std::chrono::system_clock::period::den;
    int const OFFSET_SECONDS = 8;
    std::size_t const stop_time = current_time + OFFSET_SECONDS;

    cpr::Url const url = (type == "MARKET") ? cpr::Url {rest_url + "/order/newtradeorder"} : cpr::Url {rest_url + "/order/newstoplimitorder"};

    while (! market_name_list.empty() && current_time <= stop_time)
    {
        // -----------------------
        error_list = {};
        for (auto const& market_name : market_name_list)
        {
            std::string market_id;
            if (market_id_map.count(market_name)) { market_id = market_id_map[market_name]; }
            else
            {
                auto response = get_market_ids({market_name});
                if (response.contains(market_name)) { market_id = market_id_map[market_name]; }
                else
                {
                    error_list.push_back(market_name);
                    BOOST_LOG_TRIVIAL(error) << "Failure Fetching Market ID for Market - " << market_name;
                    continue;
                }
            }

            // Check Trade Map Has Required Fields
            if (trade_map[market_name]["Direction"].dump() == "null")
            {
                BOOST_LOG_TRIVIAL(error) << market_name << " - Direction Required for All Orders";
                input_error_list.push_back(market_name);
                continue;
            }
            if (trade_map[market_name]["Quantity"].dump() == "null")
            {
                BOOST_LOG_TRIVIAL(error) << market_name << " - Quantity Required for All Orders";
                input_error_list.push_back(market_name);
                continue;
            }
            if (type == "LIMIT" && trade_map[market_name]["TriggerPrice"].dump() == "null")
            {
                BOOST_LOG_TRIVIAL(error) << market_name << " - Trigger Price Required for Limit Orders";
                input_error_list.push_back(market_name);
                continue;
            }

            std::string bid_price, offer_price;

            auto bid_response = get_prices({market_name}, 1, 0, 0, "BID");
            auto offer_response = get_prices({market_name}, 1, 0, 0, "ASK");
            if (bid_response.contains(market_name) && offer_response.contains(market_name) && bid_response[market_name][0]["Price"].dump() != "null" &&
                offer_response[market_name][0]["Price"].dump() != "null")
            {
                bid_price = bid_response[market_name][0]["Price"].dump();
                offer_price = offer_response[market_name][0]["Price"].dump();
            }
            else
            {
                error_list.push_back(market_name);
                BOOST_LOG_TRIVIAL(error) << "Failure Fetching Prices for Market - " << market_name;
                continue;
            }
            // ---------------------------
            std::vector<nlohmann::json> if_done;
            if (type == "LIMIT")
            {
                if (trade_map[market_name]["StopPrice"].dump() != "null")
                {
                    std::string opp_direction = (trade_map[market_name]["Direction"] == "sell") ? "buy" : "sell";
                    nlohmann::json stop_order = {{"Stop",
                                                  {{"TriggerPrice", trade_map[market_name]["StopPrice"].dump()},
                                                   {"Direction", opp_direction},
                                                   {"Quantity", trade_map[market_name]["Quantity"].dump()}}}};
                    if_done.push_back(stop_order);
                }

                if (trade_map[market_name]["LimitPrice"].dump() != "null")
                {
                    std::string opp_direction = (trade_map[market_name]["Direction"] == "sell") ? "buy" : "sell";
                    nlohmann::json limit_order = {{"Limit",
                                                   {{"TriggerPrice", trade_map[market_name]["LimitPrice"].dump()},
                                                    {"Direction", opp_direction},
                                                    {"Quantity", trade_map[market_name]["Quantity"].dump()}}}};
                    if_done.push_back(limit_order);
                }
            }
            nlohmann::json trade_payload = {
                {"Direction", trade_map[market_name]["Direction"]},
                // {"AuditId", audit_id},
                {"MarketId", market_id},
                {"Quantity", trade_map[market_name]["Quantity"].dump()},
                {"MarketName", market_name},
                {"TradingAccountId", tr_account_id},
                {"OfferPrice", offer_price},
                {"BidPrice", bid_price},
            };

            if (type == "LIMIT")
            {
                nlohmann::json additional_payload = {{"TriggerPrice", trade_map[market_name]["TriggerPrice"].dump()}, {"IfDone", if_done}};
                trade_payload.update(additional_payload);
            }
            else
            {
                nlohmann::json additional_payload = {{"PriceTolerance", "0"}};
                trade_payload.update(additional_payload);
            }

            nlohmann::json resp = make_network_call(session_header, url, trade_payload.dump(), "POST", "Trade Order - " + market_name);
            // Set Logging to 'Info' to suppress output.
            BOOST_LOG_TRIVIAL(debug) << "Order Response: " << market_name << " " << resp;
            // ---------------------------
            if (! resp.contains("OrderId") || ! resp["OrderId"].is_number_integer() || resp["OrderId"] == 0) { error_list.push_back(market_name); }
        }
        // -----------------------
        market_name_list = error_list;
        current_time = (std::chrono::system_clock::now().time_since_epoch()).count() * std::chrono::system_clock::period::num /
                       std::chrono::system_clock::period::den;
        if (! market_name_list.empty()) { sleep(1); }
    }
    // -------------------
    error_list.insert(error_list.end(), input_error_list.begin(), input_error_list.end());
    return error_list;
}

nlohmann::json GCapiClient::list_open_positions(std::string tr_account_id)
{
    /* List of Open Positions in the trading account.
       :param trading_acc_id: trading account ID
       :return JSON response
    */
    nlohmann::json resp;
    // -------------------
    if (! validate_session()) { return resp; }

    if (tr_account_id.empty()) { tr_account_id = CLASS_trading_account_id; }

    cpr::Url const url {rest_url + "/order/openpositions?TradingAccountId=" + tr_account_id};

    resp = make_network_call(session_header, url, "", "GET", "List Open Positions");

    if (resp.contains("OpenPositions")) { resp = resp["OpenPositions"]; }
    // -------------------
    return resp;
}

nlohmann::json GCapiClient::list_active_orders(std::string tr_account_id)
{
    /* List of Active Order in the trading account.
       :param trading_acc_id: trading account ID
       :return JSON response
    */
    nlohmann::json resp;
    // -------------------
    if (! validate_session()) { return resp; }

    if (tr_account_id.empty()) { tr_account_id = CLASS_trading_account_id; }

    cpr::Url const url {rest_url + "/order/activeorders"};
    nlohmann::json active_order_payload = {{"TradingAccountId", tr_account_id}, {"MaxResults", "100"}};

    resp = make_network_call(session_header, url, active_order_payload.dump(), "POST", "List Active Orders");

    if (resp.contains("ActiveOrders")) { resp = resp["ActiveOrders"]; }
    // -------------------
    return resp;
}

nlohmann::json GCapiClient::cancel_order(std::string const& order_id, std::string tr_account_id)
{
    /* Cancels an Active Order
       :order_id: Order ID of the Order to Cancel
       :param trading_acc_id: trading account ID
       :return JSON response
    */
    nlohmann::json resp;
    // -------------------
    if (! validate_session()) { return resp; }

    if (tr_account_id.empty()) { tr_account_id = CLASS_trading_account_id; }

    cpr::Url const url {rest_url + "/order/cancel"};
    nlohmann::json cancel_order_payload = {{"TradingAccountId", tr_account_id}, {"OrderId", order_id}};
    // -------------------
    return make_network_call(session_header, url, cancel_order_payload.dump(), "POST", "Cancel Order");
}

// =================================================================================================================
// UTILITIES
// =================================================================================================================

nlohmann::json GCapiClient::make_network_call(cpr::Header const& header, cpr::Url const& url, std::string const& payload, std::string const& type,
                                              std::string const& func_name)
{
    nlohmann::json resp;
    int const COUNT_MAX = 2;
    for (int iteration {}; iteration < COUNT_MAX; ++iteration)
    {
        try
        {
            cpr::Response r;
            if (type == "POST") { r = cpr::Post(url, header, cpr::Body {payload}); }
            else if (type == "GET") { r = cpr::Get(url, header); }
            if (r.status_code != 200) { throw r; }
            resp = nlohmann::json::parse(r.text);
            break;
        }
        catch (cpr::Response const& r)
        {
            if (! r.status_code)
            {
                BOOST_LOG_TRIVIAL(fatal) << func_name << " Error - Status Code: " << r.status_code << "; Message: No Internet Connection";
            }
            else { BOOST_LOG_TRIVIAL(fatal) << func_name << " Error - Status Code: " << r.status_code << "; Message: " << r.text; }

            if (iteration < COUNT_MAX - 1 && (r.status_code == 500 || r.status_code == 504))
            {
                BOOST_LOG_TRIVIAL(info) << "Retrying Request";
                sleep(1);
                continue;
            }
            break;
        }
        catch (nlohmann::json::exception const& e)
        {
            BOOST_LOG_TRIVIAL(fatal) << func_name << " JSON Error - " << e.what();
            break;
        }
    }
    return resp;
}

void GCapiClient::add_console_log(bool const enable)
{
    /* Optional: Boost Logging to STD Output */
    static auto console_sink = boost::log::add_console_log(std::cout, boost::log::keywords::format = ">> %Message%");

    if (! enable) { boost::log::core::get()->remove_sink(console_sink); }
}

void GCapiClient::initialize_logging_file(std::string const& file_path, std::string const& file_name, std::string severity)
{
    /* * Optional: Boost Logging to File */
    std::string file_name_concat = file_path + "/" + file_name + ".log";
    static auto file_sink =
        boost::log::add_file_log(boost::log::keywords::file_name = file_name_concat, boost::log::keywords::format = "[%TimeStamp%]: %Message%",
                                 boost::log::keywords::auto_flush = true);

    std::transform(severity.begin(), severity.end(), severity.begin(), ::tolower);

    if (severity == "fatal") { boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::fatal); }
    else if (severity == "error") { boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::error); }
    else if (severity == "warning") { boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::warning); }
    else if (severity == "info") { boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info); }
    else if (severity == "debug") { boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug); }
    else if (severity == "trace") { boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace); }
    else
    {
        std::cerr
            << "No match to Boost Logging Severity Level. Provide one of the following: 'fatal', 'error', 'warning', 'info', 'debug', or 'trace'";
    }

    boost::log::add_common_attributes();
}

bool GCapiClient::validate_session_header() const
{
    if (session_header.empty())
    {
        BOOST_LOG_TRIVIAL(fatal) << "Session Not Authenticated, Run 'authenticate_session' Command";
        return false;
    }
    return true;
}

bool GCapiClient::validate_auth_payload() const
{
    if (auth_payload.empty())
    {
        BOOST_LOG_TRIVIAL(fatal) << "Failed to pass 'Username', 'Password', and 'APIKey' to constructor";
        return false;
    }
    return true;
}

bool GCapiClient::validate_account_ids() const noexcept
{
    if (CLASS_trading_account_id == "" || CLASS_client_account_id == "") { return false; }
    return true;
}

void GCapiClient::set_testing_rest_urls(std::string const& url) { rest_url = rest_url_v2 = url; }

}// namespace gaincapital
