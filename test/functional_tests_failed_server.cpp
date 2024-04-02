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
            
            return Response(200, "{\"tradingAccounts\": [{\"tradingAccountId\":\"TradingTestID\", \"clientAccountId\":\"ClientTestID\",\"SampleParam\":\"123\"}]}");
        }
        // Margin Info
        else if (method == "GET" && matchesMargin(url, "/margin/clientAccountMargin")) {
            
            return Response(400, "{\"SampleParam\":\"123\"}");
        }
        // Market IDs & Market Info
        else if (method == "GET" && matchesMarkets(url, "/cfd/markets")) {
            
            return Response(200, "{\"Markets\": [{\"MarketId\": 123,\"SampleParam\":\"123\"}]}");
        }
        // Prices
        else if (method == "GET" && matchesPrices(url, "/market/123/tickhistory")) {
            
            return Response(400, "{\"PriceTicks\": \"123\"}");
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
        return url.substr(0, 50) == str.substr(0, 50);
    }

    bool matchesMarkets(const std::string &url, const std::string &str) const {
        return url.substr(0, 35) == str.substr(0, 35);
    }

    bool matchesOpenPositions(const std::string &url, const std::string &str) const {
        return url.substr(0, 43) == str.substr(0, 43);
    }

    bool matchesPrices(const std::string &url, const std::string &str) const {
        return url.substr(0, 54) == str.substr(0, 54);
    }

    bool matchesOHLC(const std::string &url, const std::string &str) const {
        return url.substr(0, 53) == str.substr(0, 53);
    }
};


TEST(GainCapitalFunctional, Check_All_APIs_Base_Case) {
    // Here should be implementation of test case using HTTP server.
    // HTTP requests are processed by HTTPMock::responseHandler(...)
    gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    g.set_testing_rest_urls("http://localhost:9200");

    EXPECT_EQ(g.authenticate_session(), false);
}


int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new httpmock::TestEnvironment<HTTPMock>());
    return RUN_ALL_TESTS();
}
