// Copyright 2024, Andrew Drogalis
// GNU License

#include "gain_capital_api.h"

#include "json/json.hpp"
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

int main()
{
    // Forex.com Account Info
    string username = "BLANK", password = "BLANK", apikey = "BLANK";

    // List of Currencies to Trade
    vector<string> currency_pairs = {"USD/CHF", "EUR/USD", "GBP/USD"};

    // Initialize GCapiClient
    gaincapital::GCapiClient gc_api = gaincapital::GCapiClient(username, password, apikey);

    // Send Logging to STD Ouput
    gc_api.add_console_log(true);

    // Required for First Authentication
    if (! gc_api.authenticate_session()) { return 1; }

    // Get Account Information
    nlohmann::json account_response = gc_api.get_account_info();

    // Get Margin Information
    nlohmann::json margin_response = gc_api.get_margin_info();

    // Get Market IDs
    // Sets Class Map with Market IDs
    std::unordered_map<std::string, int> market_ids_response = gc_api.get_market_ids(currency_pairs);

    // Get Currency Prices
    std::unordered_map<std::string, nlohmann::json> price_response = gc_api.get_prices(currency_pairs);

    // Get OHLC Bars
    string interval = "MINUTE";
    int num_ticks = 10;
    std::unordered_map<std::string, nlohmann::json> ohlc_response = gc_api.get_ohlc(currency_pairs, interval, num_ticks);

    // Place Market Order
    nlohmann::json trades_map_market = {};
    for (string const& symbol : currency_pairs) { trades_map_market[symbol] = {{"Direction", "sell"}, {"Quantity", 1000}}; }
    std::vector<std::string> market_order_response = gc_api.trade_order(trades_map_market, "MARKET");

    // Place Limit Order
    nlohmann::json trades_map_limit = {};
    for (string const& symbol : currency_pairs)
    {
        float mid_price = static_cast<float>(price_response[symbol][0]["Price"]);
        float trigger_price = mid_price * 1.1;
        float stop_price = mid_price * 0.9;

        trades_map_limit[symbol] = {{"Direction", "buy"}, {"Quantity", 1000}, {"TriggerPrice", trigger_price}, {"StopPrice", stop_price}};
    }
    std::vector<std::string> limit_order_response = gc_api.trade_order(trades_map_limit, "LIMIT");

    // Order Management
    nlohmann::json active_order_response = gc_api.list_active_orders();

    nlohmann::json open_position_response = gc_api.list_open_positions();

    // Cancel Active Orders
    for (nlohmann::json active_order : active_order_response)
    {
        // Cancel Market Orders
        if (active_order.contains("TradeOrder"))
        {
            string order_id = active_order["TradeOrder"]["OrderId"].dump();
            gc_api.cancel_order(order_id);
        }

        // Cancel Limit Orders
        if (active_order.contains("StopLimitOrder"))
        {
            string order_id = active_order["StopLimitOrder"]["OrderId"].dump();
            gc_api.cancel_order(order_id);
        }
    }
}