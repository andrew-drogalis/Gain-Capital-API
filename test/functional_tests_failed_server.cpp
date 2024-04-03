// Copyright 2024, Andrew Drogalis
// GNU License

#include "gain_capital_api.h"

#include <string>
#include <iostream>

#include "json/json.hpp"
#include "gtest/gtest.h"
#include "httpmockserver/mock_server.h"
#include "httpmockserver/test_environment.h"


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
        if (method == "POST" && matchesPrefix(url, "/Session")) {
            
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


TEST(GainCapitalFunctional, AuthenticateSessionFailedServerTest) {
    // Here should be implementation of test case using HTTP server.
    // HTTP requests are processed by HTTPMock::responseHandler(...)
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");

    EXPECT_EQ(g.authenticate_session(), false);
}

TEST(GainCapitalFunctional, GetAccountInfoFailedServerTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");
    g.authenticate_session();

    nlohmann::json response{};

    EXPECT_EQ(g.get_account_info(), response);
}

TEST(GainCapitalFunctional, GetMarginInfoFailedServerTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");
    g.authenticate_session();

    nlohmann::json response{};

    EXPECT_EQ(g.get_margin_info("SampleParam"), response);
}

TEST(GainCapitalFunctional, GetMarketIDsFailedServerTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");
    g.authenticate_session();

    std::unordered_map<std::string, int> response = {};

    EXPECT_EQ(g.get_market_ids({"USD/CAD"}), response);
}

TEST(GainCapitalFunctional, GetMarketInfoFailedServerTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");
    g.authenticate_session();

    std::unordered_map<std::string, std::string> response = {};

    EXPECT_EQ(g.get_market_info({"USD/CAD"}, "SampleParam"), response);
}


TEST(GainCapitalFunctional, GetPricesFailedServerTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");
    g.authenticate_session();

    std::unordered_map<std::string, nlohmann::json> response = {};

    EXPECT_EQ(g.get_prices({"TEST_MARKET"}), response);
}

TEST(GainCapitalFunctional, GetOHLCFailedServerTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");
    g.authenticate_session();

    std::unordered_map<std::string, nlohmann::json> response = {};

    EXPECT_EQ(g.get_ohlc({"TEST_MARKET"}, "MINUTE"), response);
}

TEST(GainCapitalFunctional, TradeOrderFailedServerTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");
    g.authenticate_session();

    std::vector<std::string> response{"TEST_MARKET"};

    nlohmann::json trades_map_limit = {};

    trades_map_limit["TEST_MARKET"] = {{"Direction", "buy"}, {"Quantity", 1000}};

    EXPECT_EQ(g.trade_order(trades_map_limit, "MARKET"), response);
}

TEST(GainCapitalFunctional, ListOpenPositionsFailedServerTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");
    g.authenticate_session();

    nlohmann::json response{};

    EXPECT_EQ(g.list_open_positions(), response);
}

TEST(GainCapitalFunctional, ListActiveFailedServerTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");
    g.authenticate_session();

    nlohmann::json response{};

    EXPECT_EQ(g.list_active_orders(), response);
}

TEST(GainCapitalFunctional, CancelOrderFailedServerTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");
    g.authenticate_session();

    nlohmann::json response{};

    EXPECT_EQ(g.cancel_order("123456"), response);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new httpmock::TestEnvironment<HTTPMock>());
    return RUN_ALL_TESTS();
}
