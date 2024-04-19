// Copyright 2024, Andrew Drogalis
// GNU License

#include <iostream>
#include <string>

#include "json/json.hpp"

#include "gain_capital_api.h"

int main()
{
    // Forex.com Account Info
    std::string const username = "BLANK", password = "BLANK", apikey = "BLANK";

    // List of Currencies to Trade
    std::vector<std::string> const currency_pairs = {"USD/CHF", "EUR/USD", "GBP/USD"};

    // Initialize GCapiClient
    gaincapital::GCapiClient gc_api = gaincapital::GCapiClient(username, password, apikey);

    // Required for First Authentication
    auto authentication_response = gc_api.authenticate_session();

    if (! authentication_response)
    {
        std::cout << authentication_response.error().what() << '\n';
        return 1;
    }

    // Get Account Information
    auto account_response = gc_api.get_account_info();

    if (! account_response)
    {
        std::cout << account_response.error().what() << '\n';
        return 1;
    }

    // Access Account Information Json Response
    nlohmann::json account_json = account_response.value();

    // Get Margin Information
    auto margin_response = gc_api.get_margin_info();

    if (! margin_response)
    {
        std::cout << margin_response.error().what() << '\n';
        return 1;
    }

    // Access Margin Information Json Response
    nlohmann::json margin_json = margin_response.value();

    // Get Info for Each Market
    for (std::string const& market_name : currency_pairs)
    {
        // Get Market IDs
        auto market_id_response = gc_api.get_market_id(market_name);

        if (! market_id_response)
        {
            std::cout << market_id_response.error().what() << '\n';
            return 1;
        }

        // Get Currency Prices
        auto price_response = gc_api.get_prices(market_name);

        if (! price_response)
        {
            std::cout << price_response.error().what() << '\n';
            return 1;
        }

        // Access Currency Prices Json Response
        nlohmann::json price_json = price_response.value();

        // Get OHLC Bars
        std::string const interval = "MINUTE";
        int const num_ticks = 10;
        auto ohlc_response = gc_api.get_ohlc(market_name, interval, num_ticks);

        if (! ohlc_response)
        {
            std::cout << ohlc_response.error().what() << '\n';
            return 1;
        }

        // Access OHLC Bars Json Response
        nlohmann::json ohlc_json = ohlc_response.value();

        // Place Market Order
        nlohmann::json trades_map_market = {};
        for (std::string const& symbol : currency_pairs) { trades_map_market[symbol] = {{"Direction", "sell"}, {"Quantity", 1000}}; }
        auto market_order_response = gc_api.trade_order(trades_map_market, "MARKET");

        if (! market_order_response)
        {
            std::cout << market_order_response.error().what() << '\n';
            return 1;
        }

        // Access Market Order Json Response
        nlohmann::json market_order_json = market_order_response.value(); 
        

        // Place Limit Order
        nlohmann::json trades_map_limit = {};
        for (std::string const& symbol : currency_pairs)
        {
            float const mid_price = static_cast<float>(price_json["PriceTicks"][0]["Price"]);
            float const trigger_price = mid_price * 1.1;
            float const stop_price = mid_price * 0.9;

            trades_map_limit[symbol] = {{"Direction", "buy"}, {"Quantity", 1000}, {"TriggerPrice", trigger_price}, {"StopPrice", stop_price}};
        }
        auto limit_order_response = gc_api.trade_order(trades_map_limit, "LIMIT");

        if (! limit_order_response)
        {
            std::cout << limit_order_response.error().what() << '\n';
            return 1;
        }

        // Access Limit Order Json Response
        nlohmann::json limit_order_json = limit_order_response.value(); 
    }

    // Get Open Positions
    auto open_position_response = gc_api.list_open_positions();

    if (! open_position_response)
    {
        std::cout << open_position_response.error().what() << '\n';
        return 1;
    }

    // Access Open Positions Json Response
    nlohmann::json open_position_json = open_position_response.value();

    // Get Active Orders
    auto active_order_response = gc_api.list_active_orders();

    if (! active_order_response)
    {
        std::cout << active_order_response.error().what() << '\n';
        return 1;
    }

    // Access Active Order Json Response
    nlohmann::json active_order_json = active_order_response.value();

    // Cancel Active Orders
    for (nlohmann::json& active_order : active_order_json)
    {
        // Cancel Market Orders
        if (active_order.contains("TradeOrder"))
        {
            std::string const order_id = active_order["TradeOrder"]["OrderId"].dump();
            auto cancel_order_response = gc_api.cancel_order(order_id);

            if (! cancel_order_response)
            {
                std::cout << cancel_order_response.error().what() << '\n';
                return 1;
            }
        }

        // Cancel Limit Orders
        if (active_order.contains("StopLimitOrder"))
        {
            std::string const order_id = active_order["StopLimitOrder"]["OrderId"].dump();
            auto cancel_order_response = gc_api.cancel_order(order_id);

            if (! cancel_order_response)
            {
                std::cout << cancel_order_response.error().what() << '\n';
                return 1;
            }
        }
    }
}