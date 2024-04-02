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
        if (method == "POST" && matchesPrefix(url, "/Session")) {
            
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
            
            return Response(200, "{\"PriceTicks\": \"123\"}");
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
    
    bool matchesMargin(const std::string &url, const std::string &str) const {
        return url.substr(0, 50) == str.substr(0, 50);
    }

    bool matchesMarkets(const std::string &url, const std::string &str) const {
        return url.substr(0, 35) == str.substr(0, 35);
    }

    bool matchesOpenPositions(const std::string &url, const std::string &str) const {
        return url.substr(0, 43) == str.substr(0, 43);
    }

    bool matchesPrices(const std::string &url, const std::string &str) const {
        return url.substr(0, 46) == str.substr(0, 46);
    }

    bool matchesOHLC(const std::string &url, const std::string &str) const {
        return url.substr(0, 45) == str.substr(0, 45);
    }
};

// =================================================================================
// Single or Double Function Tests
// =================================================================================
TEST(GainCapitalFunctional, AuthenticateSessionTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9201");

    EXPECT_EQ(g.authenticate_session(), true);
}

TEST(GainCapitalFunctional, ValidateSessionTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9201");
    g.authenticate_session();

    EXPECT_EQ(g.validate_session(), true);
}

TEST(GainCapitalFunctional, GetAccountInfoNoParamTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9201");
    g.authenticate_session();

    nlohmann::json response = nlohmann::json::parse("{\"tradingAccounts\": [{\"tradingAccountId\":\"TradingTestID\", \"clientAccountId\":\"ClientTestID\",\"SampleParam\":\"123\"}]}");

    EXPECT_EQ(g.get_account_info(), response);
    EXPECT_EQ(g.trading_account_id, "\"TradingTestID\"");
    EXPECT_EQ(g.client_account_id, "\"ClientTestID\"");
}

TEST(GainCapitalFunctional, GetAccountInfoWithParamTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9201");
    g.authenticate_session();

    EXPECT_EQ(g.get_account_info("SampleParam"), "123");
    EXPECT_EQ(g.trading_account_id, "\"TradingTestID\"");
    EXPECT_EQ(g.client_account_id, "\"ClientTestID\"");
}

TEST(GainCapitalFunctional, GetMarginInfoNoParamTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9201");
    g.authenticate_session();
    g.trading_account_id = "TEST";
    g.client_account_id = "TEST";

    nlohmann::json response = nlohmann::json::parse("{\"SampleParam\":\"123\"}");

    EXPECT_EQ(g.get_margin_info(), response);
}

TEST(GainCapitalFunctional, GetMarginInfoWithParamTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9201");
    g.authenticate_session();
    g.trading_account_id = "TEST";
    g.client_account_id = "TEST";

    EXPECT_EQ(g.get_margin_info("SampleParam"), "123");
}

TEST(GainCapitalFunctional, GetMarketIDsTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9201");
    g.authenticate_session();

    std::unordered_map<std::string, int> response = {{"USD/CAD", 123}};

    EXPECT_EQ(g.get_market_ids({"USD/CAD"}), response);
}

TEST(GainCapitalFunctional, GetMarketInfoTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9201");
    g.authenticate_session();

    std::unordered_map<std::string, std::string> response = {{"USD/CAD", "\"123\""}};

    EXPECT_EQ(g.get_market_info({"USD/CAD"}, "SampleParam"), response);
}

// =================================================================================
// Multi Function Tests
// =================================================================================
TEST(GainCapitalFunctional, GetPricesTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9201");
    g.authenticate_session();

    std::unordered_map<std::string, nlohmann::json> response = {{"TEST_MARKET", "123"}};

    EXPECT_EQ(g.get_prices({"TEST_MARKET"}), response);
}



TEST(GainCapitalFunctional, GetOHLCTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9201");
    g.authenticate_session();

    std::unordered_map<std::string, nlohmann::json> response = {{"TEST_MARKET", "123"}};

    EXPECT_EQ(g.get_ohlc({"TEST_MARKET"}, "MINUTE"), response);
}

// =================================================================================
// Single Function Tests
// =================================================================================
TEST(GainCapitalFunctional, ListOpenPositionsTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9201");
    g.authenticate_session();

    EXPECT_EQ(g.list_open_positions(), "123");
}

TEST(GainCapitalFunctional, ListActiveOrdersTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9201");
    g.authenticate_session();

    EXPECT_EQ(g.list_active_orders(), "123");
}

TEST(GainCapitalFunctional, CancelOrderTest) {
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9201");
    g.authenticate_session();

    nlohmann::json response = nlohmann::json::parse("{\"RESPONSE\": 123}");

    EXPECT_EQ(g.cancel_order("123456"), response);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new httpmock::TestEnvironment<HTTPMock>());
    return RUN_ALL_TESTS();
}
