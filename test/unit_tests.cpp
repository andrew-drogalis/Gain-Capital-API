// Copyright 2024, Andrew Drogalis
// GNU License

#include "gain_capital_api.h"

#include "gtest/gtest.h"
#include "json/json.hpp"

namespace {

namespace GC = gaincapital;

std::string URL = "http://localhost:9200";


TEST(GainCapitalUnit, Default_Constructor) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    
    EXPECT_EQ(gc.CLASS_trading_account_id, "");
    EXPECT_EQ(gc.CLASS_client_account_id, "");
    EXPECT_EQ(gc.market_id_map, (std::unordered_map<std::string, std::string>()));
}


TEST(GainCapitalUnit, Payload_Set_Correctly) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    
    EXPECT_EQ(gc.validate_auth_payload(), true);
}


TEST(GainCapitalUnit, Payload_Set_InCorrectly) {
    GC::GCapiClient gc{};
    gc.set_testing_rest_urls(URL);
    
    EXPECT_EQ(gc.validate_auth_payload(), false);
}


TEST(GainCapitalUnit, Account_IDs_Set_InCorrectly) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    
    EXPECT_EQ(gc.validate_account_ids(), false);
}


TEST(GainCapitalUnit, Account_Ds_Set_Correctly) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    
    gc.CLASS_trading_account_id = "TEST";
    gc.CLASS_client_account_id = "TEST";

    EXPECT_EQ(gc.validate_account_ids(), true);
}


TEST(GainCapitalUnit, Session_Header_Set_InCorrectly) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    
    EXPECT_EQ(gc.validate_session_header(), false);
}


TEST(GainCapitalUnit, Authentication_API_Call_FailEarly) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    EXPECT_EQ(gc.authenticate_session(), false);
}


TEST(GainCapitalUnit, Validate_Session_API_Call_FailEarly) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    EXPECT_EQ(gc.validate_session(), false);
}


TEST(GainCapitalUnit, Account_Info_API_Call_FailEarly) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    EXPECT_EQ(gc.get_account_info(), nlohmann::json {});
}


TEST(GainCapitalUnit, Margin_Info_API_Call_FailEarly) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    EXPECT_EQ(gc.get_margin_info(), nlohmann::json {});
}


TEST(GainCapitalUnit, Market_IDs_APIC_all_FailEarly) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    EXPECT_EQ(gc.get_market_ids({"USD/CAD"}), (std::unordered_map<std::string, std::string>()));
}


TEST(GainCapitalUnit, Market_Info_API_Call_FailEarly) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    EXPECT_EQ(gc.get_market_info({"USD/CAD"}), (std::unordered_map<std::string, std::string>()));
}


TEST(GainCapitalUnit, Get_Prices_API_Call_FailEarly) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    EXPECT_EQ(gc.get_prices({"USD/CAD"}), (std::unordered_map<std::string, nlohmann::json>()));
}


TEST(GainCapitalUnit, OHLC_API_Call_FailEarly) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    EXPECT_EQ(gc.get_ohlc({"USD/CAD"}, "MINUTE", 1), (std::unordered_map<std::string, nlohmann::json>()));
}


TEST(GainCapitalUnit, Trade_Order_API_Call_FailEarly) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    nlohmann::json trades_map_limit = {};

    trades_map_limit["USD/CAD"] = {{"Direction", "buy"}, {"Quantity", 1000}, {"TriggerPrice", 1.0}, {"StopPrice", 1.2}};

    std::vector<std::string> response = {"USD/CAD"};

    EXPECT_EQ(gc.trade_order(trades_map_limit, "LIMIT"), response);
}


TEST(GainCapitalUnit, List_Open_Positions_API_Call_FailEarly) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    EXPECT_EQ(gc.list_open_positions(), nlohmann::json {});
}


TEST(GainCapitalUnit, List_Active_Orders_API_Call_FailEarly) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    EXPECT_EQ(gc.list_active_orders(), nlohmann::json {});
}


TEST(GainCapitalUnit, Cancel_Order_API_Call_FailEarly) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    EXPECT_EQ(gc.cancel_order("123"), nlohmann::json {});
}


} // namespace