// Copyright 2024, Andrew Drogalis
// GNU License

#include "gain_capital_client.h"

#include <algorithm>       // for transform
#include <array>           // for array
#include <bits/chrono.h>   // for system_clock
#include <ctype.h>         // for toupper
#include <expected>        // for expected
#include <initializer_list>// for initialize...
#include <iostream>        // for operator<<
#include <source_location> // for source_location...
#include <string>          // for basic_string
#include <unistd.h>        // for sleep
#include <unordered_map>   // for unordered_map
#include <vector>          // for vector

#include "cpr/api.h"     // for Get, Post
#include "cpr/body.h"    // for Body
#include "cpr/response.h"// for Response
#include "json/json.hpp" // for json_ref

#include "gain_capital_exception.h"// for GCException

namespace gaincapital
{

GCClient::GCClient(std::string const& username, std::string const& password, std::string const& apikey)
{
    auth_payload = {{"UserName", username}, {"Password", password}, {"AppKey", apikey}};
}

// =================================================================================================================
// AUTHENTICATION
// =================================================================================================================

std::expected<bool, GCException> GCClient::authenticate_session()
{
    /* * The first authentication of the user. This method MUST run before any other API request. */
    auto validation_response = validate_auth_payload();
    if (! validation_response) { return validation_response; }
    // -------------------
    cpr::Header const headers {{"Content-Type", "application/json"}};
    cpr::Url const url {rest_url_v2 + "/Session"};
    // -------------------
    auto resp = make_network_call(headers, url, auth_payload.dump(), "POST");

    if (! resp) { return std::expected<bool, GCException> {std::unexpect, std::move(resp.error())}; }

    nlohmann::json json = resp.value();

    if (! json["statusCode"].is_number_integer() || json["statusCode"] != 0)
    {
        return std::expected<bool, GCException> {std::unexpect, std::source_location::current().function_name(),
                                                 "API Response is Valid, but Gain Capital Status Code Error: " + json["statusCode"].dump()};
    }
    // -------------------
    session_header = {{"Content-Type", "application/json"}, {"UserName", auth_payload["UserName"]}, {"Session", json["session"].dump()}};

    auto trading_account_resp = set_trading_account_id();
    if (! trading_account_resp) { return trading_account_resp; }
    // -------------------
    return std::expected<bool, GCException> {true};
}

std::expected<bool, GCException> GCClient::set_trading_account_id()
{
    /* * Sets the member variables CLASS_trading_account_id & CLASS_client_account_id. */
    cpr::Url const url {rest_url_v2 + "/userAccount/ClientAndTradingAccount"};
    // -------------------
    auto resp = make_network_call(session_header, url, "", "GET");

    if (! resp) { return std::expected<bool, GCException> {std::unexpect, std::move(resp.error())}; }

    nlohmann::json json = resp.value();

    CLASS_trading_account_id = json["tradingAccounts"][0]["tradingAccountId"].dump();
    CLASS_client_account_id = json["tradingAccounts"][0]["clientAccountId"].dump();

    if (CLASS_trading_account_id == "null" || CLASS_client_account_id == "null")
    {
        return std::expected<bool, GCException> {std::unexpect, std::source_location::current().function_name(),
                                                 "JSON Key Error - Response: " + json.dump()};
    }
    // -------------------
    return std::expected<bool, GCException> {true};
}

std::expected<bool, GCException> GCClient::validate_session()
{
    /* * Validates current session and updates if token expired. */
    auto validation_response = validate_session_header();
    if (! validation_response) { return validation_response; }
    // -------------------
    nlohmann::json payload = {{"ClientAccountId", CLASS_client_account_id},
                              {"UserName", session_header["Username"]},
                              {"Session", session_header["Session"]},
                              {"TradingAccountId", CLASS_trading_account_id}};
    cpr::Url const url {rest_url_v2 + "/Session/validate"};

    auto resp = make_network_call(session_header, url, payload.dump(), "POST");

    if (! resp) { return std::expected<bool, GCException> {std::unexpect, std::move(resp.error())}; }

    nlohmann::json json = resp.value();

    if (json["isAuthenticated"].dump() != "true")
    {
        auto authentication_response = authenticate_session();
        if (! authentication_response) { return authentication_response; }
    }
    // -------------------
    return std::expected<bool, GCException> {true};
}

// =================================================================================================================
// API CALLS
// =================================================================================================================
std::expected<nlohmann::json, GCException> GCClient::get_account_info()
{
    /* Gets the trading account's general information.
       :return: trading account information
    */
    auto validation_response = validate_session_header();
    if (! validation_response) { return validation_response; }
    // -------------------
    cpr::Url const url {rest_url_v2 + "/userAccount/ClientAndTradingAccount"};
    // -------------------
    return make_network_call(session_header, url, "", "GET");
}

std::expected<nlohmann::json, GCException> GCClient::get_margin_info()
{
    /*  Gets the trading account's margin information.
        :return: trading account margin information
    */
    auto validation_response = validate_session_header();
    if (! validation_response) { return validation_response; }
    // -------------------
    cpr::Url const url {rest_url_v2 + "/margin/clientAccountMargin?clientAccountId=" + CLASS_client_account_id};
    // -------------------
    return make_network_call(session_header, url, "", "GET");
}

std::expected<nlohmann::json, GCException> GCClient::get_market_id(std::string const& market_name)
{
    /* Gets the market information.
       :market_name: market name (e.g. USD/CAD)
       :return: market information
    */
    auto validation_response = validate_session_header();
    if (! validation_response) { return validation_response; }
    // -------------------
    cpr::Url const url {rest_url + "/cfd/markets?MarketName=" + market_name};

    auto resp = make_network_call(session_header, url, "", "GET");

    if (! resp) { return resp; }

    nlohmann::json json = resp.value();

    std::string const market_id = json["Markets"][0]["MarketId"].dump();

    if (market_id == "null")
    {
        return std::expected<nlohmann::json, GCException> {std::unexpect, std::source_location::current().function_name(),
                                                           "JSON Key Error - Response: " + json.dump()};
    }
    market_id_map[market_name] = market_id;
    // -------------------
    return std::expected<nlohmann::json, GCException> {market_id};
}

std::expected<nlohmann::json, GCException> GCClient::get_market_info(std::string const& market_name)
{
    /* Gets the market information.
       :market_name: market name (e.g. USD/CAD)
       :return: market information
    */
    auto validation_response = validate_session_header();
    if (! validation_response) { return validation_response; }
    // -------------------
    cpr::Url const url {rest_url + "/cfd/markets?MarketName=" + market_name};
    // -------------------
    return make_network_call(session_header, url, "", "GET");
}

std::expected<nlohmann::json, GCException> GCClient::get_prices(std::string const& market_name, std::size_t const num_ticks,
                                                                std::size_t const from_ts, std::size_t const to_ts, std::string price_type)
{
    /*  Get prices
        :param market_name: market name (e.g. USD/CAD)
        :param num_ticks: number of price ticks/data to retrieve
        :param from_ts: from timestamp UTC
        :param to_ts: to timestamp UTC
        :return: price data
  */
    auto validation_response = validate_session_header();
    if (! validation_response) { return validation_response; }
    // -------------------
    std::transform(price_type.begin(), price_type.end(), price_type.begin(), ::toupper);

    if (price_type != "BID" && price_type != "ASK" && price_type != "MID")
    {
        return std::expected<nlohmann::json, GCException> {std::unexpect, std::source_location::current().function_name(),
                                                           "Price Type Error - Provide one of the following price types: 'ASK', 'BID', 'MID'"};
    }
    // -------------------
    std::string market_id = "";
    if (market_id_map.count(market_name)) { market_id = market_id_map[market_name]; }
    else
    {
        auto response = get_market_id(market_name);
        if (market_id_map.contains(market_name)) { market_id = market_id_map[market_name]; }
        else
        {
            return std::expected<nlohmann::json, GCException> {std::unexpect, std::source_location::current().function_name(),
                                                               "Failure Fetching Market ID"};
        }
    }
    // -------------------
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
    else { url = cpr::Url {rest_url + "/market/" + market_id + "/tickhistory?PriceTicks=" + std::to_string(num_ticks) + "&priceType=" + price_type}; }
    // -------------------
    return make_network_call(session_header, url, "", "GET");
}

std::expected<nlohmann::json, GCException> GCClient::get_ohlc(std::string const& market_name, std::string interval, std::size_t const num_ticks,
                                                              std::size_t span, std::size_t const from_ts, std::size_t const to_ts)
{
    /*  Get the open, high, low, close of a specific market_id
        :param market_name: market name (e.g. USD/CAD)
        :param num_ticks: number of price ticks/data to retrieve
        :param interval: MINUTE, HOUR or DAY tick interval
        :param span: it can be a combination of span with interval, 1Hour, 15 MINUTE
        :param from_ts: from timestamp UTC :param to_ts: to timestamp UTC
        :return: ohlc dataframe
    */
    auto validation_response = validate_session_header();
    if (! validation_response) { return validation_response; }
    // -------------------
    std::transform(interval.begin(), interval.end(), interval.begin(), ::toupper);

    std::array<int, 7> const SPAN_M = {1, 2, 3, 5, 10, 15, 30};// Span intervals for minutes
    std::array<int, 4> const SPAN_H = {1, 2, 4, 8};            // Span intervals for hours
    std::array<std::string, 5> const INTERVAL = {"HOUR", "MINUTE", "DAY", "WEEK", "MONTH"};

    if (std::find(INTERVAL.begin(), INTERVAL.end(), interval) == INTERVAL.end())
    {
        return std::expected<nlohmann::json, GCException> {
            std::unexpect, std::source_location::current().function_name(),
            "Interval Error - Provide one of the following intervals: 'HOUR', 'MINUTE', 'DAY', 'WEEK', 'MONTH'"};
    }
    // -------------------
    if (interval == "HOUR")
    {
        if (std::find(SPAN_H.begin(), SPAN_H.end(), span) == SPAN_H.end())
        {
            return std::expected<nlohmann::json, GCException> {std::unexpect, std::source_location::current().function_name(),
                                                               "Span Hour Error - Provide one of the following spans: 1, 2, 4, 8"};
        }
    }
    else if (interval == "MINUTE")
    {
        if (std::find(SPAN_M.begin(), SPAN_M.end(), span) == SPAN_M.end())
        {
            return std::expected<nlohmann::json, GCException> {std::unexpect, std::source_location::current().function_name(),
                                                               "Span Minute Error - Provide one of the following spans: 1, 2, 3, 5, 10, 15, 30"};
        }
    }
    else { span = 1; }
    // -------------------
    std::string market_id;
    if (market_id_map.count(market_name)) { market_id = market_id_map[market_name]; }
    else
    {
        auto response = get_market_id(market_name);
        if (market_id_map.contains(market_name)) { market_id = market_id_map[market_name]; }
        else
        {
            return std::expected<nlohmann::json, GCException> {std::unexpect, std::source_location::current().function_name(),
                                                               "Failure Fetching Market ID"};
        }
    }
    // -------------------
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
    // -------------------
    return make_network_call(session_header, url, "", "GET");
}

std::expected<nlohmann::json, GCException> GCClient::trade_order(nlohmann::json& trade_map, std::string type, std::string tr_account_id)
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
    auto validation_response = validate_session_header();
    if (! validation_response) { return validation_response; }
    // -------------------
    if (tr_account_id.empty()) { tr_account_id = CLASS_trading_account_id; }
    // -------------------
    std::transform(type.begin(), type.end(), type.begin(), ::toupper);

    if (type != "MARKET" && type != "LIMIT")
    {
        return std::expected<nlohmann::json, GCException> {std::unexpect, std::source_location::current().function_name(),
                                                           "Trade Order Type Must Be 'MARKET' or 'LIMIT'"};
    }
    // -------------------
    std::string const market_name = trade_map.begin().key();
    std::string market_id;
    if (market_id_map.count(market_name)) { market_id = market_id_map[market_name]; }
    else
    {
        auto response = get_market_id(market_name);
        if (market_id_map.contains(market_name)) { market_id = market_id_map[market_name]; }
        else
        {
            return std::expected<nlohmann::json, GCException> {std::unexpect, std::source_location::current().function_name(),
                                                               "Failure Fetching Market ID"};
        }
    }
    // -------------------
    // Check Trade Map Has Required Fields
    if (trade_map[market_name]["Direction"].dump() == "null")
    {
        return std::expected<nlohmann::json, GCException> {std::unexpect, std::source_location::current().function_name(),
                                                           "Direction Required for All Orders"};
    }
    if (trade_map[market_name]["Quantity"].dump() == "null")
    {
        return std::expected<nlohmann::json, GCException> {std::unexpect, std::source_location::current().function_name(),
                                                           "Quantity Required for All Orders"};
    }
    if (type == "LIMIT" && trade_map[market_name]["TriggerPrice"].dump() == "null")
    {
        return std::expected<nlohmann::json, GCException> {std::unexpect, std::source_location::current().function_name(),
                                                           "Trigger Price Required for Limit Orders"};
    }
    // ---------------------------
    std::vector<nlohmann::json> if_done;
    if (type == "LIMIT")
    {
        if (trade_map[market_name]["StopPrice"].dump() != "null")
        {
            std::string const opp_direction = (trade_map[market_name]["Direction"] == "sell") ? "buy" : "sell";
            if_done.emplace_back(nlohmann::json {{"Stop",
                                                  {{"TriggerPrice", trade_map[market_name]["StopPrice"].dump()},
                                                   {"Direction", opp_direction},
                                                   {"Quantity", trade_map[market_name]["Quantity"].dump()}}}});
        }

        if (trade_map[market_name]["LimitPrice"].dump() != "null")
        {
            std::string const opp_direction = (trade_map[market_name]["Direction"] == "sell") ? "buy" : "sell";
            if_done.emplace_back(nlohmann::json {{"Limit",
                                                  {{"TriggerPrice", trade_map[market_name]["LimitPrice"].dump()},
                                                   {"Direction", opp_direction},
                                                   {"Quantity", trade_map[market_name]["Quantity"].dump()}}}});
        }
    }
    // -------------------
    std::size_t current_time = (std::chrono::system_clock::now().time_since_epoch()).count() * std::chrono::system_clock::period::num /
                               std::chrono::system_clock::period::den;
    int const RETRY_SECONDS = 5;
    std::size_t const stop_time = current_time + RETRY_SECONDS;
    while (current_time <= stop_time)
    {
        std::string bid_price, offer_price;

        auto bid_response = get_prices(market_name, 1, 0, 0, "BID");
        auto offer_response = get_prices(market_name, 1, 0, 0, "ASK");
        if (! bid_response || ! offer_response)
        {
            return std::expected<nlohmann::json, GCException> {std::unexpect, std::source_location::current().function_name(),
                                                               "Failure Fetching Prices"};
        }

        nlohmann::json bid_json = bid_response.value();
        nlohmann::json offer_json = offer_response.value();

        if (bid_json["PriceTicks"][0]["Price"].dump() != "null" && offer_json["PriceTicks"][0]["Price"].dump() != "null")
        {
            bid_price = bid_json["PriceTicks"][0]["Price"].dump();
            offer_price = offer_json["PriceTicks"][0]["Price"].dump();
        }
        else
        {
            return std::expected<nlohmann::json, GCException> {std::unexpect, std::source_location::current().function_name(),
                                                               "JSON Key Error in Fetching Prices - Response: " + bid_json.dump()};
        }
        // -------------------
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
        // -------------------
        cpr::Url const url = (type == "MARKET") ? cpr::Url {rest_url + "/order/newtradeorder"} : cpr::Url {rest_url + "/order/newstoplimitorder"};

        auto resp = make_network_call(session_header, url, trade_payload.dump(), "POST");

        if (! resp) { return resp; }

        nlohmann::json json = resp.value();

        if (json.contains("OrderId") && json["OrderId"].is_number_integer() || json["OrderId"] != 0) { return resp; }
        // -----------------------
        // Pause Before Retry
        sleep(1);
        current_time = (std::chrono::system_clock::now().time_since_epoch()).count() * std::chrono::system_clock::period::num /
                       std::chrono::system_clock::period::den;
    }
    // -------------------
    return std::expected<nlohmann::json, GCException> {std::unexpect, std::source_location::current().function_name(),
                                                       "Failed to Place Trade - Time Expired"};
}

std::expected<nlohmann::json, GCException> GCClient::list_open_positions(std::string tr_account_id)
{
    /* List of Open Positions in the trading account.
       :param trading_acc_id: trading account ID
       :return JSON response
    */
    auto validate_response = validate_session();
    if (! validate_response) { return validate_response; }
    // -------------------
    if (tr_account_id.empty()) { tr_account_id = CLASS_trading_account_id; }

    cpr::Url const url {rest_url + "/order/openpositions?TradingAccountId=" + tr_account_id};
    // -------------------
    return make_network_call(session_header, url, "", "GET");// ["OpenPositions"]
}

std::expected<nlohmann::json, GCException> GCClient::list_active_orders(std::string tr_account_id)
{
    /* List of Active Order in the trading account.
       :param trading_acc_id: trading account ID
       :return JSON response
    */
    auto validate_response = validate_session();
    if (! validate_response) { return validate_response; }
    // -------------------
    if (tr_account_id.empty()) { tr_account_id = CLASS_trading_account_id; }

    cpr::Url const url {rest_url + "/order/activeorders"};
    nlohmann::json active_order_payload = {{"TradingAccountId", tr_account_id}, {"MaxResults", "100"}};
    // -------------------
    return make_network_call(session_header, url, active_order_payload.dump(), "POST");// ["ActiveOrders"]
}

std::expected<nlohmann::json, GCException> GCClient::cancel_order(std::string const& order_id, std::string tr_account_id)
{
    /* Cancels an Active Order
       :order_id: Order ID of the Order to Cancel
       :param trading_acc_id: trading account ID
       :return JSON response
    */
    auto validate_response = validate_session();
    if (! validate_response) { return validate_response; }
    // -------------------
    if (tr_account_id.empty()) { tr_account_id = CLASS_trading_account_id; }

    cpr::Url const url {rest_url + "/order/cancel"};
    nlohmann::json cancel_order_payload = {{"TradingAccountId", tr_account_id}, {"OrderId", order_id}};
    // -------------------
    return make_network_call(session_header, url, cancel_order_payload.dump(), "POST");
}

// =================================================================================================================
// UTILITIES
// =================================================================================================================

std::expected<nlohmann::json, GCException> GCClient::make_network_call(cpr::Header const& header, cpr::Url const& url, std::string const& payload,
                                                                       std::string const& type, std::source_location const& location)
{
    cpr::Response r;
    if (type == "POST") { r = cpr::Post(url, header, cpr::Body {payload}); }
    else if (type == "GET") { r = cpr::Get(url, header); }
    // -------------------
    if (r.status_code == 200) { return std::expected<nlohmann::json, GCException> {nlohmann::json::parse(r.text)}; }
    else if (! r.status_code)
    {
        return std::expected<bool, GCException> {std::unexpect, location.function_name(),
                                                 " Error - Status Code: " + std::to_string(r.status_code) + "; Message: No Internet Connection"};
    }
    else
    {
        return std::expected<bool, GCException> {std::unexpect, location.function_name(),
                                                 " Error - Status Code: " + std::to_string(r.status_code) + "; Message: " + r.text};
    }
}

std::expected<bool, GCException> GCClient::validate_session_header() const
{
    if (session_header.empty())
    {
        return std::expected<bool, GCException> {std::unexpect, std::source_location::current().function_name(),
                                                 "Session Not Authenticated, Run 'authenticate_session' Command"};
    }
    return std::expected<bool, GCException> {true};
}

std::expected<bool, GCException> GCClient::validate_auth_payload() const
{
    if (auth_payload.empty())
    {
        return std::expected<bool, GCException> {std::unexpect, std::source_location::current().function_name(),
                                                 "Failed to pass 'Username', 'Password', and 'APIKey' to constructor"};
    }
    return std::expected<bool, GCException> {true};
}

bool GCClient::validate_account_ids() const noexcept
{
    if (CLASS_trading_account_id == "" || CLASS_client_account_id == "") { return false; }
    return true;
}

void GCClient::set_testing_rest_urls(std::string const& url) { rest_url = rest_url_v2 = url; }

}// namespace gaincapital
