// Copyright 2024, Andrew Drogalis
// GNU License

#ifndef GAIN_CAPITAL_API_H
#define GAIN_CAPITAL_API_H

#include "cpr/cpr.h"

#include "json/json.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace gaincapital
{

class GCapiClient
{

  public:
    std::string trading_account_id = "";
    std::string client_account_id = "";
    std::unordered_map<std::string, int> market_id_map;

    GCapiClient();

    GCapiClient(const std::string& username, const std::string& password, const std::string& appkey) noexcept;

    GCapiClient(const GCapiClient& obj) = delete;

    GCapiClient& operator=(const GCapiClient& obj) = delete;

    // =================================================================================================================
    // AUTHENTICATION
    // =================================================================================================================

    bool authenticate_session();

    bool validate_session();

    // =================================================================================================================
    // UTILITIES
    // =================================================================================================================

    void add_console_log(bool enable) noexcept;

    void initialize_logging_file(const std::string& file_path, const std::string& file_name, std::string severity = "debug") noexcept;

    bool validate_session_header() const noexcept;

    bool validate_auth_payload() const noexcept;

    bool validate_account_ids() const noexcept;

    void set_testing_rest_urls(const std::string& url) noexcept;

    // =================================================================================================================
    // API CALLS
    // =================================================================================================================

    [[nodiscard]] nlohmann::json get_account_info(const std::string& param = "");

    [[nodiscard]] nlohmann::json get_margin_info(const std::string& param = "");

    [[nodiscard]] std::unordered_map<std::string, int> get_market_ids(const std::vector<std::string>& positions);

    [[nodiscard]] std::unordered_map<std::string, std::string> get_market_info(const std::vector<std::string>& market_name_list,
                                                                               const std::string& param = "");

    [[nodiscard]] std::unordered_map<std::string, nlohmann::json> get_prices(const std::vector<std::string>& market_name_list,
                                                                             const std::size_t num_ticks = 1, const std::size_t from_ts = 0,
                                                                             const std::size_t to_ts = 0, std::string price_type = "MID");

    [[nodiscard]] std::unordered_map<std::string, nlohmann::json> get_ohlc(const std::vector<std::string>& market_name_list, std::string interval,
                                                                           const std::size_t num_ticks = 1, std::size_t span = 1,
                                                                           const std::size_t from_ts = 0, const std::size_t to_ts = 0);

    [[nodiscard]] std::vector<std::string> trade_order(nlohmann::json trade_map, std::string type, std::string tr_account_id = "");

    [[nodiscard]] nlohmann::json list_open_positions(std::string tr_account_id = "");

    [[nodiscard]] nlohmann::json list_active_orders(std::string tr_account_id = "");

    nlohmann::json cancel_order(const std::string& order_id, std::string tr_account_id = "");

  private:
    std::string rest_url_v2 = "https://ciapi.cityindex.com/v2";
    std::string rest_url = "https://ciapi.cityindex.com/TradingAPI";
    cpr::Header session_header;
    nlohmann::json auth_payload;
    nlohmann::json session_payload;

    // =================================================================================================================
    // AUTHENTICATION
    // =================================================================================================================

    bool set_trading_account_id();
};

}// namespace gaincapital

#endif
