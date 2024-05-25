// Copyright 2024, Andrew Drogalis
// GNU License

#include <iostream>
#include <string>
#include <typeinfo>

#include "httpmockserver/mock_server.h"
#include "httpmockserver/test_environment.h"
#include "gtest/gtest.h"

#include "gain_capital_client.h"
#include "gain_capital_exception.h"

namespace
{

namespace GC = gaincapital;

std::string URL = "http://localhost:9201";

class HTTPMock : public httpmock::MockServer
{
  public:
    /// Create HTTP server on port 9201
    explicit HTTPMock(int port = 9201) : MockServer(port) {}

  private:
    /// Handler called by MockServer on HTTP request.
    Response responseHandler(std::string const& url, std::string const& method, std::string const& data, std::vector<UrlArg> const& urlArguments,
                             std::vector<Header> const& headers)
    {
        // Authenticate Session
        if (method == "POST" && matchesPrefix(url, "/Session") && matchesAuth(url))
        {
            return Response(200, "{\"statusCode\": 0, \"session\": \"123\"}");
        }
        // Validate Session
        else if (method == "POST" && matchesPrefix(url, "/Session/validate"))
        {
            return Response(200, "{\"isAuthenticated\": true}");
        }
        // Account Info
        else if (method == "GET" && matchesPrefix(url, "/userAccount/ClientAndTradingAccount"))
        {
            return Response(
                200,
                "{\"tradingAccounts\": [{\"tradingAccountId\":\"TradingTestID\", \"clientAccountId\":\"ClientTestID\",\"SampleParam\":\"123\"}]}");
        }
        // Margin Info
        else if (method == "GET" && matchesMargin(url, "/margin/clientAccountMargin"))
        {
            return Response(200, "{\"SampleParam\":\"123\"}");
        }
        // Market IDs & Market Info
        else if (method == "GET" && matchesMarkets(url, "/cfd/markets"))
        {
            return Response(200, "{\"Markets\": [{\"MarketId\": 123,\"SampleParam\":\"123\"}]}");
        }
        // Prices
        else if (method == "GET" && matchesPrices(url, "/market/123/tickhistory"))
        {
            return Response(200, "{\"PriceTicks\":[{\"Price\" : 1.0}]}");
        }
        // OHLC
        else if (method == "GET" && matchesOHLC(url, "/market/123/barhistory"))
        {
            return Response(200, "{\"PriceBars\": \"123\"}");
        }
        // Trade Market Order
        else if (method == "POST" && matchesPrefix(url, "/order/newtradeorder"))
        {
            return Response(200, "{\"OrderId\": 1}");
        }
        // Trade Limit Order
        else if (method == "POST" && matchesPrefix(url, "/order/newstoplimitorder"))
        {
            return Response(200, "{\"OrderId\": 1}");
        }
        // List Open Positons
        else if (method == "GET" && matchesOpenPositions(url, "/order/openpositions"))
        {
            return Response(200, "{\"OpenPositions\": \"123\"}");
        }
        // List Active Orders
        else if (method == "POST" && matchesPrefix(url, "/order/activeorders"))
        {
            return Response(200, "{\"ActiveOrders\": \"123\"}");
        }
        // Cancel Order
        else if (method == "POST" && matchesPrefix(url, "/order/cancel"))
        {
            return Response(200, "{\"RESPONSE\": 123}");
        }
        // Return "URI not found" for the undefined methods
        return Response(404, "Not Found");
    }

    /// Return true if \p url starts with \p str.
    bool matchesPrefix(std::string const& url, std::string const& str) const { return url.substr(0, str.size()) == str; }

    bool matchesAuth(std::string const& url) const { return url == "/Session"; }

    bool matchesMargin(std::string const& url, std::string const& str) const { return url.substr(0, 27) == str.substr(0, 27); }

    bool matchesMarkets(std::string const& url, std::string const& str) const { return url.substr(0, 12) == str.substr(0, 12); }

    bool matchesOpenPositions(std::string const& url, std::string const& str) const { return url.substr(0, 20) == str.substr(0, 20); }

    bool matchesPrices(std::string const& url, std::string const& str) const { return url.substr(0, 23) == str.substr(0, 23); }

    bool matchesOHLC(std::string const& url, std::string const& str) const { return url.substr(0, 22) == str.substr(0, 22); }
};

// =================================================================================
// Single or Double Function Tests
// =================================================================================

TEST(GainCapital_Functional_Server, Authenticate_Session_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    auto network_response = gc.authenticate_session();

    if (network_response)
    {
        EXPECT_EQ(network_response.value(), true);
        EXPECT_EQ(gc.CLASS_trading_account_id, "\"TradingTestID\"");
        EXPECT_EQ(gc.CLASS_client_account_id, "\"ClientTestID\"");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Validate_Session_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto network_response = gc.validate_session();

    if (network_response)
    {
        EXPECT_EQ(network_response.value(), true);
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Get_Account_Info_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json response = nlohmann::json::parse(
        "{\"tradingAccounts\": [{\"tradingAccountId\":\"TradingTestID\", \"clientAccountId\":\"ClientTestID\",\"SampleParam\":\"123\"}]}");

    auto network_response = gc.get_account_info();

    if (network_response)
    {
        EXPECT_EQ(network_response.value(), response);
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Get_Margin_Info_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();
    gc.CLASS_trading_account_id = "TEST";
    gc.CLASS_client_account_id = "TEST";

    nlohmann::json response = nlohmann::json::parse("{\"SampleParam\":\"123\"}");

    auto network_response = gc.get_margin_info();

    if (network_response)
    {
        EXPECT_EQ(network_response.value(), response);
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Get_Market_IDs_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json response = "123";

    auto network_response = gc.get_market_id("USD/CAD");

    if (network_response)
    {
        EXPECT_EQ(gc.market_id_map.contains("USD/CAD"), true);
        EXPECT_EQ(gc.market_id_map["USD/CAD"], "123");
        EXPECT_EQ(network_response.value(), response);
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Get_Market_Info_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json response = nlohmann::json::parse("{\"Markets\": [{\"MarketId\": 123,\"SampleParam\":\"123\"}]}");

    auto network_response = gc.get_market_info("USD/CAD");

    if (network_response)
    {
        EXPECT_EQ(network_response.value(), response);
    }
    else
    {
        FAIL();
    }
}

// =================================================================================
// Multi Function Tests
// =================================================================================

TEST(GainCapital_Functional_Server, Get_Prices_Basic_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json response = nlohmann::json::parse("{\"PriceTicks\":[{\"Price\" : 1.0}]}");

    auto network_response = gc.get_prices("TEST_MARKET");

    if (network_response)
    {
        EXPECT_EQ(network_response.value(), response);
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Get_Prices_Test1)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json response = nlohmann::json::parse("{\"PriceTicks\":[{\"Price\" : 1.0}]}");

    auto network_response = gc.get_prices("TEST_MARKET", 1, 0, 0, "MID");

    if (network_response)
    {
        EXPECT_EQ(network_response.value(), response);
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Get_Prices_Test2)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json response = nlohmann::json::parse("{\"PriceTicks\":[{\"Price\" : 1.0}]}");

    auto network_response = gc.get_prices("TEST_MARKET", 1, 0, 100, "BID");

    if (network_response)
    {
        EXPECT_EQ(network_response.value(), response);
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Get_Prices_Test3)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json response = nlohmann::json::parse("{\"PriceTicks\":[{\"Price\" : 1.0}]}");

    auto network_response = gc.get_prices("TEST_MARKET", 1, 1000, 0, "ASK");

    if (network_response)
    {
        EXPECT_EQ(network_response.value(), response);
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Get_Prices_FAILURE_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto network_response = gc.get_prices("TEST_MARKET", 1, 1000, 0, "X");

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> gaincapital::GCClient::get_prices(const "
                  "std::string&, std::size_t, std::size_t, std::size_t, std::string)");
        EXPECT_EQ(std::string(network_response.error().what()), "Price Type Error - Provide one of the following price types: 'ASK', 'BID', 'MID'");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Get_OHLC_Basic_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json response = nlohmann::json::parse("{\"PriceBars\": \"123\"}");

    auto network_response = gc.get_ohlc("TEST_MARKET", "MINUTE");

    if (network_response)
    {
        EXPECT_EQ(network_response.value(), response);
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Get_OHLC_Test1)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json response = nlohmann::json::parse("{\"PriceBars\": \"123\"}");

    auto network_response = gc.get_ohlc("TEST_MARKET", "MINUTE", 5, 1, 0, 0);

    if (network_response)
    {
        EXPECT_EQ(network_response.value(), response);
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Get_OHLC_Test2)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json response = nlohmann::json::parse("{\"PriceBars\": \"123\"}");

    auto network_response = gc.get_ohlc("TEST_MARKET", "MINUTE", 5, 1, 0, 100);

    if (network_response)
    {
        EXPECT_EQ(network_response.value(), response);
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Get_OHLC_Test3)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json response = nlohmann::json::parse("{\"PriceBars\": \"123\"}");

    auto network_response = gc.get_ohlc("TEST_MARKET", "HOUR", 5, 1, 1000, 0);

    if (network_response)
    {
        EXPECT_EQ(network_response.value(), response);
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Get_OHLC_FAILURE_Test1)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto network_response = gc.get_ohlc({"TEST_MARKET"}, "MINUTE", 5, 1000, 0, 0);

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> gaincapital::GCClient::get_ohlc(const "
                  "std::string&, std::string, std::size_t, std::size_t, std::size_t, std::size_t)");
        EXPECT_EQ(std::string(network_response.error().what()), "Span Minute Error - Provide one of the following spans: 1, 2, 3, 5, 10, 15, 30");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Get_OHLC_FAILURE_Test2)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto network_response = gc.get_ohlc("TEST_MARKET", "MIN", 5, 1, 0, 100);

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> gaincapital::GCClient::get_ohlc(const "
                  "std::string&, std::string, std::size_t, std::size_t, std::size_t, std::size_t)");
        EXPECT_EQ(std::string(network_response.error().what()),
                  "Interval Error - Provide one of the following intervals: 'HOUR', 'MINUTE', 'DAY', 'WEEK', 'MONTH'");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Get_OHLC_FAILURE_Test3)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto network_response = gc.get_ohlc("TEST_MARKET", "HOUR", 5, 10, 1000, 0);

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> gaincapital::GCClient::get_ohlc(const "
                  "std::string&, std::string, std::size_t, std::size_t, std::size_t, std::size_t)");
        EXPECT_EQ(std::string(network_response.error().what()), "Span Hour Error - Provide one of the following spans: 1, 2, 4, 8");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Trade_Order_Market_Basic_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json response = nlohmann::json::parse("{\"OrderId\": 1}");

    nlohmann::json trades_map_limit = {};

    trades_map_limit["TEST_MARKET"] = {{"Direction", "buy"}, {"Quantity", 1000}};

    auto network_response = gc.trade_order(trades_map_limit, "MARKET");

    if (network_response)
    {
        EXPECT_EQ(network_response.value(), response);
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Trade_Order_Limit_Basic_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json response = nlohmann::json::parse("{\"OrderId\": 1}");

    nlohmann::json trades_map_limit = {};

    trades_map_limit["TEST_MARKET"] = {{"Direction", "buy"}, {"Quantity", 1000}, {"TriggerPrice", 1.0}, {"StopPrice", 1.2}, {"LimitPrice", 2.0}};

    auto network_response = gc.trade_order(trades_map_limit, "LIMIT");

    if (network_response)
    {
        EXPECT_EQ(network_response.value(), response);
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Trade_Order_FAILURE_Test1)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json trades_map_limit = {};

    trades_map_limit["TEST_MARKET"] = {{"Direction", "buy"}, {"Quantity", 1000}, {"TriggerPrice", 1.0}, {"StopPrice", 1.2}};

    auto network_response = gc.trade_order(trades_map_limit, "NONE");

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> "
                  "gaincapital::GCClient::trade_order(nlohmann::json_abi_v3_11_3::json&, std::string, std::string)");
        EXPECT_EQ(std::string(network_response.error().what()), "Trade Order Type Must Be 'MARKET' or 'LIMIT'");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Trade_Order_FAILURE_Test2)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json trades_map_limit = {};

    trades_map_limit["TEST_MARKET"] = {{"Direction", "buy"}};

    auto network_response = gc.trade_order(trades_map_limit, "MARKET");

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> "
                  "gaincapital::GCClient::trade_order(nlohmann::json_abi_v3_11_3::json&, std::string, std::string)");
        EXPECT_EQ(std::string(network_response.error().what()), "Quantity Required for All Orders");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Trade_Order_FAILURE_Test3)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json trades_map_limit = {};

    trades_map_limit["TEST_MARKET"] = {{"Quantity", 1000}};

    auto network_response = gc.trade_order(trades_map_limit, "MARKET");

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> "
                  "gaincapital::GCClient::trade_order(nlohmann::json_abi_v3_11_3::json&, std::string, std::string)");
        EXPECT_EQ(std::string(network_response.error().what()), "Direction Required for All Orders");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Trade_Order_FAILURE_Test4)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json trades_map_limit = {};

    trades_map_limit["TEST_MARKET"] = {{"Direction", "buy"}, {"Quantity", 1000}, {"StopPrice", 1.2}};

    auto network_response = gc.trade_order(trades_map_limit, "LIMIT");

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> "
                  "gaincapital::GCClient::trade_order(nlohmann::json_abi_v3_11_3::json&, std::string, std::string)");
        EXPECT_EQ(std::string(network_response.error().what()), "Trigger Price Required for Limit Orders");
    }
    else
    {
        FAIL();
    }
}

// =================================================================================
// Single Function Tests
// =================================================================================

TEST(GainCapital_Functional_Server, List_Open_Positions_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json response = nlohmann::json::parse("{\"OpenPositions\": \"123\"}");

    auto network_response = gc.list_open_positions();

    if (network_response)
    {
        EXPECT_EQ(network_response.value(), response);
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, List_Active_Orders_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json response = nlohmann::json::parse("{\"ActiveOrders\": \"123\"}");

    auto network_response = gc.list_active_orders();

    if (network_response)
    {
        EXPECT_EQ(network_response.value(), response);
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Functional_Server, Cancel_Order_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json response = nlohmann::json::parse("{\"RESPONSE\": 123}");

    auto network_response = gc.cancel_order("123456");

    if (network_response)
    {
        EXPECT_EQ(network_response.value(), response);
    }
    else
    {
        FAIL();
    }
}

}// namespace

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new httpmock::TestEnvironment<HTTPMock>());
    return RUN_ALL_TESTS();
}
