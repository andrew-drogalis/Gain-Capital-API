// Copyright 2024, Andrew Drogalis
// GNU License

#include <typeinfo>

#include "gtest/gtest.h"

#include "gain_capital_client.h"
#include "gain_capital_exception.h"

namespace
{

namespace GC = gaincapital;

std::string URL = "http://localhost:9200";

TEST(GainCapitalUnit, Default_Constructor)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    EXPECT_EQ(gc.CLASS_trading_account_id, "");
    EXPECT_EQ(gc.CLASS_client_account_id, "");
    EXPECT_EQ(gc.market_id_map, (std::unordered_map<std::string, std::string>()));
}

TEST(GainCapitalUnit, Payload_Set_Correctly)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    auto network_response = gc.validate_auth_payload();

    if (network_response)
    {
        EXPECT_EQ(network_response.value(), true);
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapitalUnit, Payload_Set_InCorrectly)
{
    GC::GCClient gc {};
    gc.set_testing_rest_urls(URL);

    auto network_response = gc.validate_auth_payload();

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<bool, gaincapital::GCException> gaincapital::GCClient::validate_auth_payload() const");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapitalUnit, Account_IDs_Set_InCorrectly)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    EXPECT_EQ(gc.validate_account_ids(), false);
}

TEST(GainCapitalUnit, Account_IDs_Set_Correctly)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    gc.CLASS_trading_account_id = "TEST";
    gc.CLASS_client_account_id = "TEST";

    EXPECT_EQ(gc.validate_account_ids(), true);
}

TEST(GainCapitalUnit, Session_Header_Set_InCorrectly)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    auto network_response = gc.validate_session_header();

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<bool, gaincapital::GCException> gaincapital::GCClient::validate_session_header() const");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapitalUnit, Authentication_API_Call_FailEarly)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    auto network_response = gc.authenticate_session();

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<bool, gaincapital::GCException> gaincapital::GCClient::authenticate_session()");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapitalUnit, Validate_Session_API_Call_FailEarly)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    auto network_response = gc.validate_session();

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<bool, gaincapital::GCException> gaincapital::GCClient::validate_session_header() const");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapitalUnit, Account_Info_API_Call_FailEarly)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    auto network_response = gc.get_account_info();

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<bool, gaincapital::GCException> gaincapital::GCClient::validate_session_header() const");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapitalUnit, Margin_Info_API_Call_FailEarly)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    auto network_response = gc.get_margin_info();

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<bool, gaincapital::GCException> gaincapital::GCClient::validate_session_header() const");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapitalUnit, Market_IDs_APIC_all_FailEarly)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    auto network_response = gc.get_market_id("USD/CAD");

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<bool, gaincapital::GCException> gaincapital::GCClient::validate_session_header() const");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapitalUnit, Market_Info_API_Call_FailEarly)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    auto network_response = gc.get_market_info("USD/CAD");

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<bool, gaincapital::GCException> gaincapital::GCClient::validate_session_header() const");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapitalUnit, Get_Prices_API_Call_FailEarly)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    auto network_response = gc.get_prices("USD/CAD");

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<bool, gaincapital::GCException> gaincapital::GCClient::validate_session_header() const");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapitalUnit, OHLC_API_Call_FailEarly)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    auto network_response = gc.get_ohlc("USD/CAD", "MINUTE", 1);

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<bool, gaincapital::GCException> gaincapital::GCClient::validate_session_header() const");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapitalUnit, Trade_Order_API_Call_FailEarly)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    nlohmann::json trades_map_limit = {};

    trades_map_limit["USD/CAD"] = {{"Direction", "buy"}, {"Quantity", 1000}, {"TriggerPrice", 1.0}, {"StopPrice", 1.2}};

    auto network_response = gc.trade_order(trades_map_limit, "LIMIT");

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<bool, gaincapital::GCException> gaincapital::GCClient::validate_session_header() const");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapitalUnit, List_Open_Positions_API_Call_FailEarly)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    auto network_response = gc.list_open_positions();

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<bool, gaincapital::GCException> gaincapital::GCClient::validate_session_header() const");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapitalUnit, List_Active_Orders_API_Call_FailEarly)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    auto network_response = gc.list_active_orders();

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<bool, gaincapital::GCException> gaincapital::GCClient::validate_session_header() const");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapitalUnit, Cancel_Order_API_Call_FailEarly)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    auto network_response = gc.cancel_order("123");

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<bool, gaincapital::GCException> gaincapital::GCClient::validate_session_header() const");
    }
    else
    {
        FAIL();
    }
}

}// namespace