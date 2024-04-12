// Copyright 2024, Andrew Drogalis
// GNU License

#include "gain_capital_api.h"

#include <algorithm>                                     // for transform
#include <bits/chrono.h>                                 // for system_clock
#include <ctype.h>                                       // for toupper
#include <exception>                                     // for exception
#include <initializer_list>                              // for initialize...
#include <iostream>                                      // for operator<<
#include <string>                                        // for basic_string
#include <unistd.h>                                      // for sleep
#include <unordered_map>                                 // for unordered_map
#include <utility>                                       // for pair
#include <vector>                                        // for vector

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

GCapiClient::GCapiClient() {}

GCapiClient::GCapiClient(const std::string& username, const std::string& password, const std::string& apikey) noexcept
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
    std::string resp_session;
    // ---------------------------
    int iteration = 0;
    while (iteration < 2)
    {
        try
        {
            cpr::Response r = cpr::Post(url, headers, cpr::Body {auth_payload.dump()});
            if (r.status_code != 200) { throw r; }
            nlohmann::json resp = nlohmann::json::parse(r.text);
            if (static_cast<int>(resp["statusCode"]) != 0)
            {
                BOOST_LOG_TRIVIAL(fatal) << "Response is Valid, but Gain Capital Status Code Error: " << resp["statusCode"];
                return false;
            }
            resp_session = resp["session"];
            break;
        }
        catch (const cpr::Response& r)
        {
            BOOST_LOG_TRIVIAL(fatal) << "Authentication Error - Status Code: " << r.status_code << "; Message: " << r.text;
            if (! iteration && (r.status_code == 500 || r.status_code == 504))
            {
                BOOST_LOG_TRIVIAL(info) << "Retrying Request";
                sleep(1);
                continue;
            }
            return false;
        }
        catch (const std::exception& e)
        {
            BOOST_LOG_TRIVIAL(fatal) << "Authentication Error - " << e.what();
            break;
        }
        catch (...)
        {
            BOOST_LOG_TRIVIAL(fatal) << "Unknown Authentication Error - Check Internet Connection";
            return false;
        }
        ++iteration;
    }
    // ---------------------------
    session_header = {{"Content-Type", "application/json"}, {"UserName", auth_payload["UserName"]}, {"Session", resp_session}};
    if (! set_trading_account_id()) { return false; }
    return true;
}

bool GCapiClient::set_trading_account_id()
{
    /* * Sets the member variables trading_account_id & client_account_id. */
    cpr::Url const url {rest_url_v2 + "/userAccount/ClientAndTradingAccount"};
    // ---------------------------
    int iteration = 0;
    while (iteration < 2)
    {
        try
        {
            cpr::Response r = cpr::Get(url, session_header);
            if (r.status_code != 200) { throw r; }
            nlohmann::json resp = nlohmann::json::parse(r.text);
            trading_account_id = resp["tradingAccounts"][0]["tradingAccountId"].dump();
            client_account_id = resp["tradingAccounts"][0]["clientAccountId"].dump();

            return true;
        }
        catch (const cpr::Response& r)
        {
            BOOST_LOG_TRIVIAL(fatal) << "Set Trading Account Error - Status Code: " << r.status_code << "; Message: " << r.text;
            if (! iteration && (r.status_code == 500 || r.status_code == 504))
            {
                BOOST_LOG_TRIVIAL(info) << "Retrying Request";
                sleep(1);
                continue;
            }
            return false;
        }
        catch (const std::exception& e)
        {
            BOOST_LOG_TRIVIAL(fatal) << "Set Trading Account Error - " << e.what();
            break;
        }
        catch (...)
        {
            BOOST_LOG_TRIVIAL(fatal) << "Set Trading Account - Unknown Failure";
            return false;
        }
        ++iteration;
    }
    return true;
}

bool GCapiClient::validate_session()
{
    /* * Validates current session and updates if token expired. */
    if (! validate_session_header()) { return false; }

    nlohmann::json resp;
    nlohmann::json payload = {{"ClientAccountId", client_account_id},
                              {"UserName", session_header["Username"]},
                              {"Session", session_header["Session"]},
                              {"TradingAccountId", trading_account_id}};
    cpr::Url const url {rest_url_v2 + "/Session/validate"};

    int iteration = 0;
    while (iteration < 2)
    {
        try
        {
            cpr::Response r = cpr::Post(url, session_header, cpr::Body {payload.dump()});
            if (r.status_code != 200) { throw r; }
            resp = nlohmann::json::parse(r.text);

            if (resp["isAuthenticated"] == false)
            {
                if (! authenticate_session()) { return false; }
            }
            break;
        }
        catch (const cpr::Response& r)
        {
            BOOST_LOG_TRIVIAL(fatal) << "Validate Session Error - Status Code: " << r.status_code << "; Message: " << r.text;
            if (! iteration && (r.status_code == 500 || r.status_code == 504))
            {
                BOOST_LOG_TRIVIAL(info) << "Retrying Request";
                sleep(1);
                continue;
            }
            return false;
        }
        catch (const std::exception& e)
        {
            BOOST_LOG_TRIVIAL(fatal) << "Validate Session Error - " << e.what();
            break;
        }
        catch (...)
        {
            BOOST_LOG_TRIVIAL(fatal) << "Validate Session - Unknown Failure";
            return false;
        }
        ++iteration;
    }
    return true;
}

// =================================================================================================================
// UTILITIES
// =================================================================================================================

void GCapiClient::add_console_log(const bool enable) noexcept
{
    /* Optional: Boost Logging to STD Output */
    static auto console_sink = boost::log::add_console_log(std::cout, boost::log::keywords::format = ">> %Message%");

    if (! enable) { boost::log::core::get()->remove_sink(console_sink); }
}

void GCapiClient::initialize_logging_file(const std::string& file_path, const std::string& file_name, std::string severity) noexcept
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

bool GCapiClient::validate_session_header() const noexcept
{
    if (session_header.empty())
    {
        BOOST_LOG_TRIVIAL(fatal) << "Session Not Authenticated, Run 'authenticate_session' Command";
        return false;
    }
    return true;
}

bool GCapiClient::validate_auth_payload() const noexcept
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
    if (trading_account_id == "" || client_account_id == "") { return false; }
    return true;
}

void GCapiClient::set_testing_rest_urls(const std::string& url) noexcept { rest_url = rest_url_v2 = url; }

// =================================================================================================================
// API CALLS
// =================================================================================================================
nlohmann::json GCapiClient::get_account_info(const std::string& param)
{
    /* Gets trading account general information
       :param: retrieve specific information (e.g. TradingAccountId)
       :return: trading account information
    */
    nlohmann::json resp;
    cpr::Url const url {rest_url_v2 + "/userAccount/ClientAndTradingAccount"};

    if (validate_session())
    {
        int iteration = 0;
        while (iteration < 2)
        {
            try
            {
                cpr::Response r = cpr::Get(url, session_header);
                if (r.status_code != 200) { throw r; }
                resp = nlohmann::json::parse(r.text);
                trading_account_id = resp["tradingAccounts"][0]["tradingAccountId"].dump();
                client_account_id = resp["tradingAccounts"][0]["clientAccountId"].dump();

                if (! param.empty()) { return resp["tradingAccounts"][0][param]; }
                break;
            }
            catch (const cpr::Response& r)
            {
                BOOST_LOG_TRIVIAL(fatal) << "Account Info Error - Status Code: " << r.status_code << "; Message: " << r.text;
                if (! iteration && (r.status_code == 500 || r.status_code == 504))
                {
                    BOOST_LOG_TRIVIAL(info) << "Retrying Request";
                    sleep(1);
                    continue;
                }
                break;
            }
            catch (const std::exception& e)
            {
                BOOST_LOG_TRIVIAL(fatal) << "Get Account Info Error - " << e.what();
                break;
            }
            catch (...)
            {
                BOOST_LOG_TRIVIAL(fatal) << "Get Account Info - Unknown Failure";
                break;
            }
            ++iteration;
        }
    }
    // -------------------
    return resp;
}

nlohmann::json GCapiClient::get_margin_info(const std::string& param)
{
    /*  Gets trading account margin information
        :param: retrieve specific information (e.g. Cash)
        :return: trading account margin information
        -- Example of param options
        float margin_total = resp["margin"];
        float equity_total = resp["netEquity"];
    */
    nlohmann::json resp;
    // -------------------
    if (validate_session())
    {
        cpr::Url const url {rest_url_v2 + "/margin/clientAccountMargin?clientAccountId=" + client_account_id};

        int iteration = 0;
        while (iteration < 2)
        {
            try
            {
                cpr::Response r = cpr::Get(url, session_header);
                if (r.status_code != 200) { throw r; }
                resp = nlohmann::json::parse(r.text);

                if (! param.empty()) { return resp[param]; }
                break;
            }
            catch (const cpr::Response& r)
            {
                BOOST_LOG_TRIVIAL(fatal) << "Margin Info Error - Status Code: " << r.status_code << "; Message: " << r.text;
                if (! iteration && (r.status_code == 500 || r.status_code == 504))
                {
                    BOOST_LOG_TRIVIAL(info) << "Retrying Request";
                    sleep(1);
                    continue;
                }
                break;
            }
            catch (const std::exception& e)
            {
                BOOST_LOG_TRIVIAL(fatal) << "Get Margin Info Error - " << e.what();
                break;
            }
            catch (...)
            {
                BOOST_LOG_TRIVIAL(fatal) << "Get Margin Info - Unknown Failure";
                break;
            }
            ++iteration;
        }
    }
    // -------------------
    return resp;
}

std::unordered_map<std::string, int> GCapiClient::get_market_ids(const std::vector<std::string>& market_name_list)
{
    /* Gets market information
       :market_name_list: market name (e.g. USD/CAD)
       :return: market information
    */
    if (validate_session())
    {
        for (auto const& market_name : market_name_list)
        {
            try
            {
                cpr::Url const url {rest_url + "/cfd/markets?MarketName=" + market_name};
                cpr::Response r = cpr::Get(url, session_header);
                if (r.status_code != 200) { throw r; }
                nlohmann::json resp = nlohmann::json::parse(r.text);

                int const market_id = static_cast<int>(resp["Markets"][0]["MarketId"]);
                market_id_map[market_name] = market_id;
            }
            catch (const cpr::Response& r)
            {
                BOOST_LOG_TRIVIAL(error) << "Error Fetching Market ID for " << market_name << "; Status Code: " << r.status_code
                                         << "; Message: " << r.text;
            }
            catch (const std::exception& e)
            {
                BOOST_LOG_TRIVIAL(fatal) << "Error Fetching Market ID for " << market_name << " - " << e.what();
                break;
            }
            catch (...)
            {
                BOOST_LOG_TRIVIAL(error) << "Unknown Error in Fetching Market Id for " << market_name;
            }
        }
    }
    // -------------------
    return market_id_map;
}

std::unordered_map<std::string, std::string> GCapiClient::get_market_info(const std::vector<std::string>& market_name_list, const std::string& param)
{
    /* Gets market information
       :market_name_list: market name (e.g. USD/CAD)
       :param: retrieve specific information (e.g. MarketId)
       :return: market information
    */
    std::unordered_map<std::string, std::string> response_map;
    // -------------------
    if (validate_session())
    {
        for (auto const& market_name : market_name_list)
        {
            try
            {
                cpr::Url const url {rest_url + "/cfd/markets?MarketName=" + market_name};
                cpr::Response r = cpr::Get(url, session_header);
                if (r.status_code != 200) { throw r; }
                nlohmann::json resp = nlohmann::json::parse(r.text);

                if (param.empty()) { response_map[market_name] = resp["Markets"][0]["MarketId"].dump(); }
                else { response_map[market_name] = resp["Markets"][0][param].dump(); }
            }
            catch (const cpr::Response& r)
            {
                BOOST_LOG_TRIVIAL(error) << "Error Fetching Market ID for " << market_name << "; Status Code: " << r.status_code
                                         << "; Message: " << r.text;
            }
            catch (const std::exception& e)
            {
                BOOST_LOG_TRIVIAL(fatal) << "Error Fetching Market ID for " << market_name << " - " << e.what();
                break;
            }
            catch (...)
            {
                BOOST_LOG_TRIVIAL(error) << "Unknown Error in Fetching Market Id for " << market_name;
            }
        }
    }
    // -------------------
    return response_map;
}

std::unordered_map<std::string, nlohmann::json> GCapiClient::get_prices(const std::vector<std::string>& market_name_list, const std::size_t num_ticks,
                                                                        const std::size_t from_ts, const std::size_t to_ts, std::string price_type)
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
    if (validate_session())
    {
        std::transform(price_type.begin(), price_type.end(), price_type.begin(), ::toupper);

        if (price_type != "BID" && price_type != "ASK" && price_type != "MID")
        {
            BOOST_LOG_TRIVIAL(error) << "Price Type Error - Provide one of the following price types: 'ASK', 'BID', 'MID'";
            return response_map;
        }

        for (auto const& market_name : market_name_list)
        {
            std::string market_id = "0";
            if (market_id_map.count(market_name)) { market_id = std::to_string(market_id_map[market_name]); }
            else
            {
                auto response = get_market_ids({market_name});
                market_id = std::to_string(market_id_map[market_name]);
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
                url = cpr::Url {rest_url + "/market/" + market_id + "/tickhistory?PriceTicks=" + std::to_string(num_ticks) +
                                "&priceType=" + price_type};
            }
            int iteration = 0;
            while (iteration < 2)
            {
                try
                {
                    cpr::Response r = cpr::Get(url, session_header);
                    if (r.status_code != 200) { throw r; }
                    nlohmann::json resp = nlohmann::json::parse(r.text);

                    response_map[market_name] = resp["PriceTicks"];
                    break;
                }
                catch (const cpr::Response& r)
                {
                    BOOST_LOG_TRIVIAL(error) << "Error Fetching Price for " << market_name << "; Status Code: " << r.status_code
                                             << "; Message: " << r.text;
                    if (! iteration && (r.status_code == 500 || r.status_code == 504))
                    {
                        BOOST_LOG_TRIVIAL(info) << "Retrying Request";
                        sleep(1);
                        continue;
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    BOOST_LOG_TRIVIAL(fatal) << "Error Fetching Price for " << market_name << " - " << e.what();
                    break;
                }
                catch (...)
                {
                    BOOST_LOG_TRIVIAL(error) << "Unknown Error in Fetching Price for " << market_name;
                    break;
                }
                ++iteration;
            }
        }
    }
    // -------------------
    return response_map;
}

std::unordered_map<std::string, nlohmann::json> GCapiClient::get_ohlc(const std::vector<std::string>& market_name_list, std::string interval,
                                                                      const std::size_t num_ticks, std::size_t span, const std::size_t from_ts,
                                                                      const std::size_t to_ts)
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
    if (validate_session())
    {
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
            if (market_id_map.count(market_name)) { market_id = std::to_string(market_id_map[market_name]); }
            else
            {
                auto response = get_market_ids({market_name});
                market_id = std::to_string(market_id_map[market_name]);
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
            int iteration = 0;
            while (iteration < 2)
            {
                try
                {
                    cpr::Response r = cpr::Get(url, session_header);
                    if (r.status_code != 200) { throw r; }
                    nlohmann::json resp = nlohmann::json::parse(r.text);

                    response_map[market_name] = resp["PriceBars"];
                    break;
                }
                catch (const cpr::Response& r)
                {
                    BOOST_LOG_TRIVIAL(error) << "Error Fetching OHLC for " << market_name << "; Status Code: " << r.status_code
                                             << "; Message: " << r.text;
                    if (! iteration && (r.status_code == 500 || r.status_code == 504))
                    {
                        BOOST_LOG_TRIVIAL(info) << "Retrying Request";
                        sleep(1);
                        continue;
                    }
                    break;
                }
                catch (const std::exception& e)
                {
                    BOOST_LOG_TRIVIAL(fatal) << "Error Fetching OHLC for " << market_name << " - " << e.what();
                    break;
                }
                catch (...)
                {
                    BOOST_LOG_TRIVIAL(error) << "Unknown Error in Fetching OHLC for " << market_name;
                    break;
                }
                ++iteration;
            }
        }
    }
    // -------------------
    return response_map;
}

std::vector<std::string> GCapiClient::trade_order(nlohmann::json trade_map, std::string type, std::string tr_account_id)
{
    /*  Makes a new trade order
        :param trade_map: JSON object formatted as in the example below
        :param type: Limit or Market order type
        :param trading_acc_id: trading account ID
        :return: error_list: list of symbol name that failed to place trade
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
    std::vector<std::string> error_list, input_error_list;
    // -------------------
    if (validate_session())
    {
        if (tr_account_id.empty()) { tr_account_id = trading_account_id; }

        std::vector<std::string> market_name_list = {};
        for (nlohmann::json::iterator it = trade_map.begin(); it != trade_map.end(); ++it) { market_name_list.push_back(it.key()); }

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
                if (market_id_map.count(market_name)) { market_id = std::to_string(market_id_map[market_name]); }
                else
                {
                    auto response = get_market_ids({market_name});
                    market_id = std::to_string(market_id_map[market_name]);
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
                    BOOST_LOG_TRIVIAL(error) << market_name << "- Trigger Price Required for Limit Orders";
                    input_error_list.push_back(market_name);
                    continue;
                }

                float bid_price, offer_price;
                try
                {
                    bid_price = static_cast<float>(get_prices({market_name}, 1, 0, 0, "BID")[market_name][0]["Price"]);
                    offer_price = static_cast<float>(get_prices({market_name}, 1, 0, 0, "ASK")[market_name][0]["Price"]);
                }
                catch (...)
                {
                    error_list.push_back(market_name);
                    BOOST_LOG_TRIVIAL(error) << "Failure Fetching Prices for Market - " << market_name;
                    continue;
                }
                // ---------------------------
                std::vector<nlohmann::json> if_done = {};
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
                try
                {
                    nlohmann::json trade_payload = {
                        {"Direction", trade_map[market_name]["Direction"]},
                        // {"AuditId", audit_id},
                        {"MarketId", market_id},
                        {"Quantity", trade_map[market_name]["Quantity"].dump()},
                        {"MarketName", market_name},
                        {"TradingAccountId", tr_account_id},
                        {"OfferPrice", std::to_string(offer_price)},
                        {"BidPrice", std::to_string(bid_price)},
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

                    cpr::Response r = cpr::Post(url, session_header, cpr::Body(trade_payload.dump()));
                    if (r.status_code != 200) { throw r; }
                    nlohmann::json resp = nlohmann::json::parse(r.text);

                    // Set Logging to 'Info' to suppress output.
                    BOOST_LOG_TRIVIAL(debug) << "Order Response: " << market_name << " " << resp;
                    // ---------------------------
                    if (static_cast<int>(resp["OrderId"]) == 0) { error_list.push_back(market_name); }
                }
                catch (const cpr::Response& r)
                {
                    BOOST_LOG_TRIVIAL(error) << "Order Response: Status Code: " << r.status_code << "; Message: " << r.text;
                    if (r.status_code == 500 || r.status_code == 504)
                    {
                        error_list.push_back(market_name);
                    }
                }
                catch (const std::exception& e)
                {
                    BOOST_LOG_TRIVIAL(error) << "Error Placing Trade Order for " << market_name << " - " << e.what();
                }
                catch (...)
                {
                    BOOST_LOG_TRIVIAL(error) << "Error Placing Trade Order for " << market_name;
                }
            }
            // -----------------------
            market_name_list = error_list;
            current_time = (std::chrono::system_clock::now().time_since_epoch()).count() * std::chrono::system_clock::period::num /
                           std::chrono::system_clock::period::den;
            if (! market_name_list.empty()) { sleep(1); }
        }
    }
    // -------------------
    error_list.insert(error_list.end(), input_error_list.begin(), input_error_list.end());
    return error_list;
}

nlohmann::json GCapiClient::list_open_positions(std::string tr_account_id)
{
    /* List of Open Positions in Trading Account
       :param trading_acc_id: trading account ID
       :return JSON response
    */
    nlohmann::json resp;
    // -------------------
    if (validate_session())
    {
        if (tr_account_id.empty()) { tr_account_id = trading_account_id; }
        cpr::Url const url {rest_url + "/order/openpositions?TradingAccountId=" + tr_account_id};
        int iteration = 0;
        while (iteration < 2)
        {
            try
            {
                cpr::Response r = cpr::Get(url, session_header);
                if (r.status_code != 200) { throw r; }
                resp = nlohmann::json::parse(r.text)["OpenPositions"];
                break;
            }
            catch (const cpr::Response& r)
            {
                BOOST_LOG_TRIVIAL(fatal) << "Open Positions Error - Status Code: " << r.status_code << "; Message: " << r.text;
                if (! iteration && (r.status_code == 500 || r.status_code == 504))
                {
                    BOOST_LOG_TRIVIAL(info) << "Retrying Request";
                    sleep(1);
                    continue;
                }
                break;
            }
            catch (const std::exception& e)
            {
                BOOST_LOG_TRIVIAL(fatal) << "List Open Positions Exception - " << e.what();
                break;
            }
            catch (...)
            {
                BOOST_LOG_TRIVIAL(fatal) << "List Open Position - Unknown Failure";
                break;
            }
            ++iteration;
        }
    }
    // -------------------
    return resp;
}

nlohmann::json GCapiClient::list_active_orders(std::string tr_account_id)
{
    /* List of Active Order in Trading Account
       :param trading_acc_id: trading account ID
       :return JSON response
    */
    nlohmann::json resp;
    // -------------------
    if (validate_session())
    {
        if (tr_account_id.empty()) { tr_account_id = trading_account_id; }

        cpr::Url const url {rest_url + "/order/activeorders"};
        nlohmann::json active_order_payload = {{"TradingAccountId", tr_account_id}, {"MaxResults", "100"}};
        int iteration = 0;
        while (iteration < 2)
        {
            try
            {
                cpr::Response r = cpr::Post(url, session_header, cpr::Body {active_order_payload.dump()});
                if (r.status_code != 200) { throw r; }
                resp = nlohmann::json::parse(r.text)["ActiveOrders"];
                break;
            }
            catch (const cpr::Response& r)
            {
                BOOST_LOG_TRIVIAL(fatal) << "Active Orders Error - Status Code: " << r.status_code << "; Message: " << r.text;
                if (! iteration && (r.status_code == 500 || r.status_code == 504))
                {
                    BOOST_LOG_TRIVIAL(info) << "Retrying Request";
                    sleep(1);
                    continue;
                }
                break;
            }
            catch (const std::exception& e)
            {
                BOOST_LOG_TRIVIAL(fatal) << "List Active Order Exception - " << e.what();
                break;
            }
            catch (...)
            {
                BOOST_LOG_TRIVIAL(fatal) << "List Active Order - Unknown Failure" << resp;
                break;
            }
            ++iteration;
        }
    }
    // -------------------
    return resp;
}

nlohmann::json GCapiClient::cancel_order(const std::string& order_id, std::string tr_account_id)
{
    /* Cancels an Active Order
       :order_id: Order ID of the Order to Cancel
       :param trading_acc_id: trading account ID
       :return JSON response
    */
    nlohmann::json resp;
    // -------------------
    if (validate_session())
    {
        if (tr_account_id.empty()) { tr_account_id = trading_account_id; }

        cpr::Url const url {rest_url + "/order/cancel"};
        nlohmann::json cancel_order_payload = {{"TradingAccountId", tr_account_id}, {"OrderId", order_id}};
        int iteration = 0;
        while (iteration < 2)
        {
            try
            {
                cpr::Response r = cpr::Post(url, session_header, cpr::Body {cancel_order_payload.dump()});
                if (r.status_code != 200) { throw r; }
                resp = nlohmann::json::parse(r.text);
                break;
            }
            catch (const cpr::Response& r)
            {
                BOOST_LOG_TRIVIAL(fatal) << "Cancel Order Error - Status Code: " << r.status_code << "; Message: " << r.text;
                if (! iteration && (r.status_code == 500 || r.status_code == 504))
                {
                    BOOST_LOG_TRIVIAL(info) << "Retrying Request";
                    sleep(1);
                    continue;
                }
                break;
            }
            catch (const std::exception& e)
            {
                BOOST_LOG_TRIVIAL(fatal) << "Cancel Order Exception - " << e.what();
                break;
            }
            catch (...)
            {
                BOOST_LOG_TRIVIAL(fatal) << "Cancel Order - Unknown Failure";
                break;
            }
            ++iteration;
        }
    }
    // -------------------
    return resp;
}

}// namespace gaincapital
