// Copyright 2024, Andrew Drogalis
// GNU License

#include "gain_capital_api.h"

#include "gtest/gtest.h"
#include "json/json.hpp"

namespace {

TEST(GainCapitalUnit, DefaultConstructor) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");
    
    EXPECT_EQ(g.CLASS_trading_account_id, "");
    EXPECT_EQ(g.CLASS_client_account_id, "");
    EXPECT_EQ(g.market_id_map, (std::unordered_map<std::string, int>()));
}

TEST(GainCapitalUnit, PayloadSetCorrectly) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");
    
    EXPECT_EQ(g.validate_auth_payload(), true);
}

TEST(GainCapitalUnit, PayloadSetInCorrectly) {
    gaincapital::GCapiClient g{};
    g.set_testing_rest_urls("http://localhost:9200");
    
    EXPECT_EQ(g.validate_auth_payload(), false);
}

TEST(GainCapitalUnit, AccountIDsInCorrectly) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");
    
    EXPECT_EQ(g.validate_account_ids(), false);
}

TEST(GainCapitalUnit, AccountIDsCorrectly) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");
    
    g.CLASS_trading_account_id = "TEST";
    g.CLASS_client_account_id = "TEST";

    EXPECT_EQ(g.validate_account_ids(), true);
}

TEST(GainCapitalUnit, SessionHeaderSetInCorrectly) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");
    
    EXPECT_EQ(g.validate_session_header(), false);
}

TEST(GainCapitalUnit, AuthenticationAPICallFailEarly) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");

    EXPECT_EQ(g.authenticate_session(), false);
}

TEST(GainCapitalUnit, ValidateSessionAPICallFailEarly) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");

    EXPECT_EQ(g.validate_session(), false);
}

TEST(GainCapitalUnit, AccountInfoAPICallFailEarly) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");

    EXPECT_EQ(g.get_account_info(), (nlohmann::json()));
}

TEST(GainCapitalUnit, MarginInfoAPICallFailEarly) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");

    EXPECT_EQ(g.get_margin_info(), (nlohmann::json()));
}

TEST(GainCapitalUnit, MarketIDsAPICallFailEarly) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");

    EXPECT_EQ(g.get_market_ids({"USD/CAD"}), (std::unordered_map<std::string, int>()));
}

TEST(GainCapitalUnit, MarketInfoAPICallFailEarly) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");

    EXPECT_EQ(g.get_market_info({"USD/CAD"}), (std::unordered_map<std::string, std::string>()));
}

TEST(GainCapitalUnit, GetPricesAPICallFailEarly) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");

    EXPECT_EQ(g.get_prices({"USD/CAD"}), (std::unordered_map<std::string, nlohmann::json>()));
}

TEST(GainCapitalUnit, OHLC_APICallFailEarly) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");

    EXPECT_EQ(g.get_ohlc({"USD/CAD"}, "MINUTE", 1), (std::unordered_map<std::string, nlohmann::json>()));
}

TEST(GainCapitalUnit, TradeOrderAPICallFailEarly) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");

    nlohmann::json trades_map_limit = {};

    trades_map_limit["USD/CAD"] = {{"Direction", "buy"}, {"Quantity", 1000}, {"TriggerPrice", 1.0}, {"StopPrice", 1.2}};

    std::vector<std::string> response = {"USD/CAD"};

    EXPECT_EQ(g.trade_order(trades_map_limit, "LIMIT"), response);
}

TEST(GainCapitalUnit, ListOpenPositionsAPICallFailEarly) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");

    EXPECT_EQ(g.list_open_positions(), (nlohmann::json()));
}

TEST(GainCapitalUnit, ListActiveOrdersAPICallFailEarly) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");

    EXPECT_EQ(g.list_active_orders(), (nlohmann::json()));
}

TEST(GainCapitalUnit, CancelOrderAPICallFailEarly) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");

    EXPECT_EQ(g.cancel_order("123"), (nlohmann::json()));
}

} // namespace