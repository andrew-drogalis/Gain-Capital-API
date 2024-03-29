// Copyright 2024, Andrew Drogalis
// GNU License

#include "gain_capital_api.h"

#include "gtest/gtest.h"

namespace {

TEST(GainCapital, DefaultConstructor) {
    const gaincapital::GCapiClient g("TEST_USER", "TEST_PASSWORD", "TEST_APIKEY");
    
    EXPECT_EQ(g.trading_account_id, "");
    EXPECT_EQ(g.client_account_id, "");
}



} // namespace