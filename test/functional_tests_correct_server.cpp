// Copyright 2024, Andrew Drogalis
// GNU License

#include "gain_capital_api.h"

#include <string>
#include <iostream>

#include "json/json.hpp"
#include "gtest/gtest.h"
#include "httpmockserver/mock_server.h"
#include "httpmockserver/test_environment.h"

namespace 
{

namespace GC = gaincapital;

std::string URL = "http://localhost:9201";

class HTTPMock: public httpmock::MockServer {
  public:
    /// Create HTTP server on port 9201
    explicit HTTPMock(int port = 9201): MockServer(port) {}
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
            
            return Response(200, "{\"tradingAccounts\": [{\"tradingAccountId\":\"TradingTestID\", \"clientAccountId\":\"ClientTestID\",\"SampleParam\":\"123\"}]}");
        }
        // Margin Info
        else if (method == "GET" && matchesMargin(url, "/margin/clientAccountMargin")) {
            
            return Response(200, "{\"SampleParam\":\"123\"}");
        }
        // Market IDs & Market Info
        else if (method == "GET" && matchesMarkets(url, "/cfd/markets")) {
            
            return Response(200, "{\"Markets\": [{\"MarketId\": 123,\"SampleParam\":\"123\"}]}");
        }
        // Prices
        else if (method == "GET" && matchesPrices(url, "/market/123/tickhistory")) {
            
            return Response(200, "{\"PriceTicks\":[{\"Price\" : 1.0}]}");
        }
        // OHLC
        else if (method == "GET" && matchesOHLC(url, "/market/123/barhistory")) {
        
            return Response(200, "{\"PriceBars\": \"123\"}");
        }
        // Trade Market Order
        else if (method == "POST" && matchesPrefix(url, "/order/newtradeorder")) {
            
            return Response(200, "{\"OrderId\": 1}");
        }
        // Trade Limit Order
        else if (method == "POST" && matchesPrefix(url, "/order/newstoplimitorder")) {
            
            return Response(200, "{\"OrderId\": 1}");
        }
        // List Open Positons
        else if (method == "GET" && matchesOpenPositions(url, "/order/openpositions")) {
            
            return Response(200, "{\"OpenPositions\": \"123\"}");
        }
        // List Active Orders
        else if (method == "POST" && matchesPrefix(url, "/order/activeorders")) {
            
            return Response(200, "{\"ActiveOrders\": \"123\"}");
        }
        // Cancel Order
        else if (method == "POST" && matchesPrefix(url, "/order/cancel")) {
            
            return Response(200, "{\"RESPONSE\": 123}");
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

// =================================================================================
// Single or Double Function Tests
// =================================================================================

TEST(GainCapital_Functional_Server, Authenticate_Session_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);

    EXPECT_EQ(gc.authenticate_session(), true);
    EXPECT_EQ(gc.CLASS_trading_account_id, "\"TradingTestID\"");
    EXPECT_EQ(gc.CLASS_client_account_id, "\"ClientTestID\"");
}


TEST(GainCapital_Functional_Server, Validate_Session_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    EXPECT_EQ(gc.validate_session(), true);
}


TEST(GainCapital_Functional_Server, Get_Account_Info_No_Param_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    nlohmann::json response = nlohmann::json::parse("{\"tradingAccounts\": [{\"tradingAccountId\":\"TradingTestID\", \"clientAccountId\":\"ClientTestID\",\"SampleParam\":\"123\"}]}");

    EXPECT_EQ(gc.get_account_info(), response);
}


TEST(GainCapital_Functional_Server, Get_Account_Info_With_Param_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    EXPECT_EQ(gc.get_account_info("SampleParam"), "123");
}


TEST(GainCapital_Functional_Server, Get_Account_Info_With_BAD_Param_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    nlohmann::json response = nlohmann::json::parse("{\"tradingAccounts\": [{\"tradingAccountId\":\"TradingTestID\", \"clientAccountId\":\"ClientTestID\",\"SampleParam\":\"123\"}]}");

    EXPECT_EQ(gc.get_account_info("BadParam"), response);
}


TEST(GainCapital_Functional_Server, Get_Margin_Info_No_Param_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();
    gc.CLASS_trading_account_id = "TEST";
    gc.CLASS_client_account_id = "TEST";

    nlohmann::json response = nlohmann::json::parse("{\"SampleParam\":\"123\"}");

    EXPECT_EQ(gc.get_margin_info(), response);
}


TEST(GainCapital_Functional_Server, Get_Margin_Info_With_Param_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();
    gc.CLASS_trading_account_id = "TEST";
    gc.CLASS_client_account_id = "TEST";

    EXPECT_EQ(gc.get_margin_info("SampleParam"), "123");
}


TEST(GainCapital_Functional_Server, Get_Margin_Info_With_BAD_Param_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();
    gc.CLASS_trading_account_id = "TEST";
    gc.CLASS_client_account_id = "TEST";

    nlohmann::json response = nlohmann::json::parse("{\"SampleParam\":\"123\"}");

    EXPECT_EQ(gc.get_margin_info("BadParam"), response);
}


TEST(GainCapital_Functional_Server, Get_Market_IDs_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    std::unordered_map<std::string, std::string> response = {{"USD/CAD", "123"}};

    EXPECT_EQ(gc.get_market_ids({"USD/CAD"}), response);
}


TEST(GainCapital_Functional_Server, Get_Market_Info_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    std::unordered_map<std::string, std::string> response = {{"USD/CAD", "\"123\""}};

    EXPECT_EQ(gc.get_market_info({"USD/CAD"}, "SampleParam"), response);
}


// =================================================================================
// Multi Function Tests
// =================================================================================

TEST(GainCapital_Functional_Server, Get_Prices_Basic_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    nlohmann::json json = nlohmann::json::parse("[{\"Price\" : 1.0}]");
    std::unordered_map<std::string, nlohmann::json> response = {{"TEST_MARKET", json}};

    EXPECT_EQ(gc.get_prices({"TEST_MARKET"}), response);
}


TEST(GainCapital_Functional_Server, Get_Prices_Test1) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    nlohmann::json json = nlohmann::json::parse("[{\"Price\" : 1.0}]");
    std::unordered_map<std::string, nlohmann::json> response = {{"TEST_MARKET", json}};

    EXPECT_EQ(gc.get_prices({"TEST_MARKET"}, 1, 0, 0, "MID"), response);
}


TEST(GainCapital_Functional_Server, Get_Prices_Test2) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    nlohmann::json json = nlohmann::json::parse("[{\"Price\" : 1.0}]");
    std::unordered_map<std::string, nlohmann::json> response = {{"TEST_MARKET", json}};

    EXPECT_EQ(gc.get_prices({"TEST_MARKET"}, 1, 0, 100, "BID"), response);
}


TEST(GainCapital_Functional_Server, Get_Prices_Test3) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    nlohmann::json json = nlohmann::json::parse("[{\"Price\" : 1.0}]");
    std::unordered_map<std::string, nlohmann::json> response = {{"TEST_MARKET", json}};

    EXPECT_EQ(gc.get_prices({"TEST_MARKET"}, 1, 1000, 0, "ASK"), response);
}


TEST(GainCapital_Functional_Server, Get_Prices_FAILURE_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    EXPECT_EQ(gc.get_prices({"TEST_MARKET"}, 1, 1000, 0, "X"), (std::unordered_map<std::string, nlohmann::json>()));
}


TEST(GainCapital_Functional_Server, Get_OHLC_Basic_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    std::unordered_map<std::string, nlohmann::json> response = {{"TEST_MARKET", "123"}};

    EXPECT_EQ(gc.get_ohlc({"TEST_MARKET"}, "MINUTE"), response);
}


TEST(GainCapital_Functional_Server, Get_OHLC_Test1) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    std::unordered_map<std::string, nlohmann::json> response = {{"TEST_MARKET", "123"}};

    EXPECT_EQ(gc.get_ohlc({"TEST_MARKET"}, "MINUTE", 5, 1, 0, 0), response);
}


TEST(GainCapital_Functional_Server, Get_OHLC_Test2) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    std::unordered_map<std::string, nlohmann::json> response = {{"TEST_MARKET", "123"}};

    EXPECT_EQ(gc.get_ohlc({"TEST_MARKET"}, "MINUTE", 5, 1, 0, 100), response);
}


TEST(GainCapital_Functional_Server, Get_OHLC_Test3) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    std::unordered_map<std::string, nlohmann::json> response = {{"TEST_MARKET", "123"}};

    EXPECT_EQ(gc.get_ohlc({"TEST_MARKET"}, "HOUR", 5, 1, 1000, 0), response);
}


TEST(GainCapital_Functional_Server, Get_OHLC_FAILURE_Test1) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    EXPECT_EQ(gc.get_ohlc({"TEST_MARKET"}, "MINUTE", 5, 1000, 0, 0), (std::unordered_map<std::string, nlohmann::json>()));
}


TEST(GainCapital_Functional_Server, Get_OHLC_FAILURE_Test2) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    EXPECT_EQ(gc.get_ohlc({"TEST_MARKET"}, "MIN", 5, 1, 0, 100), (std::unordered_map<std::string, nlohmann::json>()));
}


TEST(GainCapital_Functional_Server, Get_OHLC_FAILURE_Test3) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    EXPECT_EQ(gc.get_ohlc({"TEST_MARKET"}, "HOUR", 5, 10, 1000, 0), (std::unordered_map<std::string, nlohmann::json>()));
}


TEST(GainCapital_Functional_Server, Trade_Order_Market_Basic_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    nlohmann::json trades_map_limit = {};

    trades_map_limit["TEST_MARKET"] = {{"Direction", "buy"}, {"Quantity", 1000}};

    EXPECT_EQ(gc.trade_order(trades_map_limit, "MARKET"), std::vector<std::string> {});
}


TEST(GainCapital_Functional_Server, Trade_Order_Limit_Basic_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    nlohmann::json trades_map_limit = {};

    trades_map_limit["TEST_MARKET"] = {{"Direction", "buy"}, {"Quantity", 1000}, {"TriggerPrice", 1.0}, {"StopPrice", 1.2}, {"LimitPrice", 2.0}};

    EXPECT_EQ(gc.trade_order(trades_map_limit, "LIMIT"), std::vector<std::string> {});
}


TEST(GainCapital_Functional_Server, Trade_Order_FAILURE_Test1) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    std::vector<std::string> response{"TEST_MARKET"};

    nlohmann::json trades_map_limit = {};

    trades_map_limit["TEST_MARKET"] = {{"Direction", "buy"}, {"Quantity", 1000}, {"TriggerPrice", 1.0}, {"StopPrice", 1.2}};

    EXPECT_EQ(gc.trade_order(trades_map_limit, "NONE"), response);
}


TEST(GainCapital_Functional_Server, Trade_Order_FAILURE_Test2) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    std::vector<std::string> response{"TEST_MARKET"};

    nlohmann::json trades_map_limit = {};

    trades_map_limit["TEST_MARKET"] = {{"Direction", "buy"}};

    EXPECT_EQ(gc.trade_order(trades_map_limit, "MARKET"), response);
}


TEST(GainCapital_Functional_Server, Trade_Order_FAILURE_Test3) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    std::vector<std::string> response{"TEST_MARKET"};

    nlohmann::json trades_map_limit = {};

    trades_map_limit["TEST_MARKET"] = {{"Quantity", 1000}};

    EXPECT_EQ(gc.trade_order(trades_map_limit, "MARKET"), response);
}


TEST(GainCapital_Functional_Server, Trade_Order_FAILURE_Test4) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    std::vector<std::string> response{"TEST_MARKET"};

    nlohmann::json trades_map_limit = {};

    trades_map_limit["TEST_MARKET"] = {{"Direction", "buy"}, {"Quantity", 1000}, {"StopPrice", 1.2}};

    EXPECT_EQ(gc.trade_order(trades_map_limit, "LIMIT"), response);
}


// =================================================================================
// Single Function Tests
// =================================================================================

TEST(GainCapital_Functional_Server, List_Open_Positions_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    EXPECT_EQ(gc.list_open_positions(), "123");
}


TEST(GainCapital_Functional_Server, List_Active_Orders_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    EXPECT_EQ(gc.list_active_orders(), "123");
}


TEST(GainCapital_Functional_Server, Cancel_Order_Test) {
    GC::GCapiClient gc("USER", "PASSWORD", "APIKEY");
    gc.set_testing_rest_urls(URL);
    bool _ = gc.authenticate_session();

    nlohmann::json response = nlohmann::json::parse("{\"RESPONSE\": 123}");

    EXPECT_EQ(gc.cancel_order("123456"), response);
}

}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new httpmock::TestEnvironment<HTTPMock>());
    return RUN_ALL_TESTS();
}
