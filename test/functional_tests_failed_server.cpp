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

std::string URL = "http://localhost:9200";

class HTTPMock : public httpmock::MockServer
{
  public:
    /// Create HTTP server on port 9200
    explicit HTTPMock(int port = 9200) : MockServer(port) {}

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
                400,
                "{\"tradingAccounts\": [{\"tradingAccountId\":\"TradingTestID\", \"clientAccountId\":\"ClientTestID\",\"SampleParam\":\"123\"}]}");
        }
        // Margin Info
        else if (method == "GET" && matchesMargin(url, "/margin/clientAccountMargin"))
        {
            return Response(400, "{\"SampleParam\":\"123\"}");
        }
        // Market IDs & Market Info
        else if (method == "GET" && matchesMarkets(url, "/cfd/markets"))
        {
            return Response(400, "{\"Markets\": [{\"MarketId\": 123,\"SampleParam\":\"123\"}]}");
        }
        // Prices
        else if (method == "GET" && matchesPrices(url, "/market/123/tickhistory"))
        {
            return Response(400, "{\"PriceTicks\":[{\"Price\" : 1.0}]}");
        }
        // OHLC
        else if (method == "GET" && matchesOHLC(url, "/market/123/barhistory"))
        {
            return Response(400, "{\"PriceBars\": \"123\"}");
        }
        // Trade Market Order
        else if (method == "POST" && matchesPrefix(url, "/order/newtradeorder"))
        {
            return Response(200, "{\"OrderId\": 0}");
        }
        // Trade Limit Order
        else if (method == "POST" && matchesPrefix(url, "/order/newstoplimitorder"))
        {
            return Response(200, "{\"OrderId\": 0}");
        }
        // List Open Positons
        else if (method == "GET" && matchesOpenPositions(url, "/order/openpositions"))
        {
            return Response(400, "{\"OpenPositions\": \"123\"}");
        }
        // List Active Orders
        else if (method == "POST" && matchesPrefix(url, "/order/activeorders"))
        {
            return Response(400, "{\"ActiveOrders\": \"123\"}");
        }
        // Cancel Order
        else if (method == "POST" && matchesPrefix(url, "/order/cancel"))
        {
            return Response(400, "{\"RESPONSE\": 123}");
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

TEST(GainCapital_Failed_Server, Authenticate_Session_Failed_Server_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    auto network_response = gc.authenticate_session();

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()), "std::expected<bool, gaincapital::GCException> gaincapital::GCClient::set_trading_account_id()");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Failed_Server, Validate_Session_Failed_Server_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    /* This Should be True to allow other tests to see Further Errors
       This Base case was tested on all API Calls in the Unit Tests */
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

TEST(GainCapital_Failed_Server, Get_Account_Info_Failed_Server_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto network_response = gc.get_account_info();

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> gaincapital::GCClient::get_account_info()");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Failed_Server, Get_Margin_Info_Failed_Server_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto network_response = gc.get_margin_info();

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> gaincapital::GCClient::get_margin_info()");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Failed_Server, Get_Market_IDs_Failed_Server_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto network_response = gc.get_market_id("USD/CAD");

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()), "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> "
                                                     "gaincapital::GCClient::get_market_id(const std::string&)");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Failed_Server, Get_Market_Info_Failed_Server_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto network_response = gc.get_market_info("USD/CAD");

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()), "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> "
                                                     "gaincapital::GCClient::get_market_info(const std::string&)");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Failed_Server, Get_Prices_Failed_Server_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto network_response = gc.get_prices("TEST_MARKET");

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> gaincapital::GCClient::get_prices(const "
                  "std::string&, std::size_t, std::size_t, std::size_t, std::string)");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Failed_Server, Get_OHLC_Failed_Server_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto network_response = gc.get_ohlc("TEST_MARKET", "MINUTE");

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> gaincapital::GCClient::get_ohlc(const "
                  "std::string&, std::string, std::size_t, std::size_t, std::size_t, std::size_t)");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Failed_Server, Trade_Order_Failed_Server_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json trades_map_limit = {};

    trades_map_limit["TEST_MARKET"] = {{"Direction", "buy"}, {"Quantity", 1000}};

    auto network_response = gc.trade_order(trades_map_limit, "MARKET");

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()),
                  "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> "
                  "gaincapital::GCClient::trade_order(nlohmann::json_abi_v3_11_3::json&, std::string, std::string)");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Failed_Server, List_Open_Positions_Failed_Server_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto network_response = gc.list_open_positions();

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()), "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> "
                                                     "gaincapital::GCClient::list_open_positions(std::string)");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Failed_Server, List_Active_Failed_Server_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto network_response = gc.list_active_orders();

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()), "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> "
                                                     "gaincapital::GCClient::list_active_orders(std::string)");
    }
    else
    {
        FAIL();
    }
}

TEST(GainCapital_Failed_Server, Cancel_Order_Failed_Server_Test)
{
    GC::GCClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto network_response = gc.cancel_order("123456");

    if (! network_response)
    {
        EXPECT_EQ(typeid(network_response.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(network_response.error().where()), "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> "
                                                     "gaincapital::GCClient::cancel_order(const std::string&, std::string)");
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
