// Copyright 2024, Andrew Drogalis
// GNU License

#include <string>
#include <iostream>
#include <typeinfo>

#include "gtest/gtest.h"
#include "httpmockserver/mock_server.h"
#include "httpmockserver/test_environment.h"

#include "gain_capital_api.h"
#include "gain_capital_exception.h"

namespace 
{

namespace GC = gaincapital;

std::string URL = "http://localhost:9200";

class HTTPMock: public httpmock::MockServer {
  public:
    /// Create HTTP server on port 9200
    explicit HTTPMock(int port = 9200): MockServer(port) {}
  private:

    /// Handler called by MockServer on HTTP request.
    Response responseHandler(
            const std::string &url,
            const std::string &method,
            const std::string &data,
            const std::vector<UrlArg> &urlArguments,
            const std::vector<Header> &headers)
    {
        // Authenticate Session
        if (method == "POST" && matchesPrefix(url, "/Session") && matchesAuth(url)) {
            
            return Response(200, "{\"statusCode\": 0, \"session\": \"123\"}");
        }
        // Validate Session
        else if (method == "POST" && matchesPrefix(url, "/Session/validate")) {
            
            return Response(200, "{\"isAuthenticated\": true}");
        }
        // Account Info
        else if (method == "GET" && matchesPrefix(url, "/userAccount/ClientAndTradingAccount")) {
            
            return Response(400, "{\"tradingAccounts\": [{\"tradingAccountId\":\"TradingTestID\", \"clientAccountId\":\"ClientTestID\",\"SampleParam\":\"123\"}]}");
        }
        // Margin Info
        else if (method == "GET" && matchesMargin(url, "/margin/clientAccountMargin")) {
            
            return Response(400, "{\"SampleParam\":\"123\"}");
        }
        // Market IDs & Market Info
        else if (method == "GET" && matchesMarkets(url, "/cfd/markets")) {
            
            return Response(400, "{\"Markets\": [{\"MarketId\": 123,\"SampleParam\":\"123\"}]}");
        }
        // Prices
        else if (method == "GET" && matchesPrices(url, "/market/123/tickhistory")) {
            
            return Response(400, "{\"PriceTicks\":[{\"Price\" : 1.0}]}");
        }
        // OHLC
        else if (method == "GET" && matchesOHLC(url, "/market/123/barhistory")) {
        
            return Response(400, "{\"PriceBars\": \"123\"}");
        }
        // Trade Market Order
        else if (method == "POST" && matchesPrefix(url, "/order/newtradeorder")) {
            
            return Response(200, "{\"OrderId\": 0}");
        }
        // Trade Limit Order
        else if (method == "POST" && matchesPrefix(url, "/order/newstoplimitorder")) {
            
            return Response(200, "{\"OrderId\": 0}");
        }
        // List Open Positons
        else if (method == "GET" && matchesOpenPositions(url, "/order/openpositions")) {
            
            return Response(400, "{\"OpenPositions\": \"123\"}");
        }
        // List Active Orders
        else if (method == "POST" && matchesPrefix(url, "/order/activeorders")) {
            
            return Response(400, "{\"ActiveOrders\": \"123\"}");
        }
        // Cancel Order
        else if (method == "POST" && matchesPrefix(url, "/order/cancel")) {
            
            return Response(400, "{\"RESPONSE\": 123}");
        }
        // Return "URI not found" for the undefined methods
        return Response(404, "Not Found");
    }

    /// Return true if \p url starts with \p str.
    bool matchesPrefix(const std::string &url, const std::string &str) const {
        return url.substr(0, str.size()) == str;
    }

    bool matchesAuth(const std::string &url) const {
        return url == "/Session";
    }

    bool matchesMargin(const std::string &url, const std::string &str) const {
        return url.substr(0, 27) == str.substr(0, 27);
    }

    bool matchesMarkets(const std::string &url, const std::string &str) const {
        return url.substr(0, 12) == str.substr(0, 12);
    }

    bool matchesOpenPositions(const std::string &url, const std::string &str) const {
        return url.substr(0, 20) == str.substr(0, 20);
    }

    bool matchesPrices(const std::string &url, const std::string &str) const {
        return url.substr(0, 23) == str.substr(0, 23);
    }

    bool matchesOHLC(const std::string &url, const std::string &str) const {
        return url.substr(0, 22) == str.substr(0, 22);
    }
};


TEST(GainCapital_Failed_Server, Authenticate_Session_Failed_Server_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    auto resp = gc.authenticate_session();

    if (! resp) 
    {
        EXPECT_EQ(typeid(resp.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(resp.error().where()), "std::expected<bool, gaincapital::GCException> gaincapital::GCapiClient::set_trading_account_id()");
    }
    else
    {
        FAIL();
    }
}


TEST(GainCapital_Failed_Server, Validate_Session_Failed_Server_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    /* This Should be True to allow other tests to see Further Errors
       This Base case was tested on all API Calls in the Unit Tests */
    auto resp = gc.validate_session();

    if (resp) 
    {
        EXPECT_EQ(resp.value(), true);
    }
    else
    {
        FAIL();
    }
}


TEST(GainCapital_Failed_Server, Get_Account_Info_Failed_Server_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto resp = gc.get_account_info();

    if (! resp) 
    {
        EXPECT_EQ(typeid(resp.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(resp.error().where()), "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> gaincapital::GCapiClient::get_account_info()");
    }
    else
    {
        FAIL();
    }
}


TEST(GainCapital_Failed_Server, Get_Margin_Info_Failed_Server_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto resp = gc.get_margin_info();

    if (! resp) 
    {
        EXPECT_EQ(typeid(resp.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(resp.error().where()), "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> gaincapital::GCapiClient::get_margin_info()");
    }
    else
    {
        FAIL();
    }
}


TEST(GainCapital_Failed_Server, Get_Market_IDs_Failed_Server_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto resp = gc.get_market_id("USD/CAD");

    if (! resp) 
    {
        EXPECT_EQ(typeid(resp.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(resp.error().where()), "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> gaincapital::GCapiClient::get_market_id(const std::string&)");
    }
    else
    {
        FAIL();
    }
}


TEST(GainCapital_Failed_Server, Get_Market_Info_Failed_Server_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto resp = gc.get_market_info("USD/CAD");

    if (! resp) 
    {
        EXPECT_EQ(typeid(resp.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(resp.error().where()), "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> gaincapital::GCapiClient::get_market_info(const std::string&)");
    }
    else
    {
        FAIL();
    }
}


TEST(GainCapital_Failed_Server, Get_Prices_Failed_Server_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto resp = gc.get_prices("TEST_MARKET");

    if (! resp) 
    {
        EXPECT_EQ(typeid(resp.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(resp.error().where()), "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> gaincapital::GCapiClient::get_prices(const std::string&, std::size_t, std::size_t, std::size_t, std::string)");
    }
    else
    {
        FAIL();
    }
}


TEST(GainCapital_Failed_Server, Get_OHLC_Failed_Server_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto resp = gc.get_ohlc("TEST_MARKET", "MINUTE");

    if (! resp) 
    {
        EXPECT_EQ(typeid(resp.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(resp.error().where()), "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> gaincapital::GCapiClient::get_ohlc(const std::string&, std::string, std::size_t, std::size_t, std::size_t, std::size_t)");
    }
    else
    {
        FAIL();
    }
}


TEST(GainCapital_Failed_Server, Trade_Order_Failed_Server_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    nlohmann::json trades_map_limit = {};

    trades_map_limit["TEST_MARKET"] = {{"Direction", "buy"}, {"Quantity", 1000}};

    auto resp = gc.trade_order(trades_map_limit, "MARKET");

    if (! resp) 
    {
        EXPECT_EQ(typeid(resp.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(resp.error().where()), "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> gaincapital::GCapiClient::trade_order(nlohmann::json_abi_v3_11_3::json&, std::string, std::string)");
    }
    else
    {
        FAIL();
    }
}


TEST(GainCapital_Failed_Server, List_Open_Positions_Failed_Server_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto resp = gc.list_open_positions();

    if (! resp) 
    {
        EXPECT_EQ(typeid(resp.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(resp.error().where()), "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> gaincapital::GCapiClient::list_open_positions(std::string)");
    }
    else
    {
        FAIL();
    }
}


TEST(GainCapital_Failed_Server, List_Active_Failed_Server_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto resp = gc.list_active_orders();

    if (! resp) 
    {
        EXPECT_EQ(typeid(resp.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(resp.error().where()), "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> gaincapital::GCapiClient::list_active_orders(std::string)");
    }
    else
    {
        FAIL();
    }
}


TEST(GainCapital_Failed_Server, Cancel_Order_Failed_Server_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    auto _ = gc.authenticate_session();

    auto resp = gc.cancel_order("123456");

    if (! resp) 
    {
        EXPECT_EQ(typeid(resp.error()), typeid(GC::GCException));
        EXPECT_EQ(std::string(resp.error().where()), "std::expected<nlohmann::json_abi_v3_11_3::basic_json<>, gaincapital::GCException> gaincapital::GCapiClient::cancel_order(const std::string&, std::string)");
    }
    else
    {
        FAIL();
    }
}

}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new httpmock::TestEnvironment<HTTPMock>());
    return RUN_ALL_TESTS();
}
