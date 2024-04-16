// Copyright 2024, Andrew Drogalis
// GNU License

#ifndef GAIN_CAPITAL_API_H
#define GAIN_CAPITAL_API_H

#include <cstddef>      // for size_t
#include <string>       // for basic_string
#include <unordered_map>// for unordered_map
#include <vector>       // for vector

#include "cpr/cprtypes.h"// for Header
#include "json/json.hpp" // for json_ref

namespace gaincapital
{

class GCapiClient
{

  public:
    std::string CLASS_trading_account_id;
    std::string CLASS_client_account_id;
    std::unordered_map<std::string, int> market_id_map;

    GCapiClient() = default;

    GCapiClient(std::string const& username, std::string const& password, std::string const& appkey);

    ~GCapiClient() = default;

    // Move ONLY | No Copy Constructor
    GCapiClient(GCapiClient const& obj) = delete;

    GCapiClient& operator=(GCapiClient const& obj) = delete;

    GCapiClient(GCapiClient&& obj) = default;

    GCapiClient& operator=(GCapiClient&& obj) = default;

    // =================================================================================================================
    // AUTHENTICATION
    // =================================================================================================================

    [[nodiscard]] bool authenticate_session();

    [[nodiscard]] bool validate_session();

    // =================================================================================================================
    // API CALLS
    // =================================================================================================================

    [[nodiscard]] nlohmann::json get_account_info(std::string const& param = "");

    [[nodiscard]] nlohmann::json get_margin_info(std::string const& param = "");

    [[nodiscard]] std::unordered_map<std::string, int> get_market_ids(std::vector<std::string> const& positions);

    [[nodiscard]] std::unordered_map<std::string, std::string> get_market_info(std::vector<std::string> const& market_name_list,
                                                                               std::string const& param = "");

    [[nodiscard]] std::unordered_map<std::string, nlohmann::json> get_prices(std::vector<std::string> const& market_name_list,
                                                                             std::size_t const num_ticks = 1, std::size_t const from_ts = 0,
                                                                             std::size_t const to_ts = 0, std::string price_type = "MID");

    [[nodiscard]] std::unordered_map<std::string, nlohmann::json> get_ohlc(std::vector<std::string> const& market_name_list, std::string interval,
                                                                           std::size_t const num_ticks = 1, std::size_t span = 1,
                                                                           std::size_t const from_ts = 0, std::size_t const to_ts = 0);

    [[nodiscard]] std::vector<std::string> trade_order(nlohmann::json& trade_map, std::string type, std::string tr_account_id = "");

    [[nodiscard]] nlohmann::json list_open_positions(std::string tr_account_id = "");

    [[nodiscard]] nlohmann::json list_active_orders(std::string tr_account_id = "");

    nlohmann::json cancel_order(std::string const& order_id, std::string tr_account_id = "");

    // =================================================================================================================
    // UTILITIES
    // =================================================================================================================

    static void add_console_log(bool const enable);

    static void initialize_logging_file(std::string const& file_path, std::string const& file_name, std::string severity = "debug");

    [[nodiscard]] bool validate_session_header() const;

    [[nodiscard]] bool validate_auth_payload() const;

    [[nodiscard]] bool validate_account_ids() const noexcept;

    void set_testing_rest_urls(std::string const& url);

  private:
    std::string rest_url_v2 = "https://ciapi.cityindex.com/v2";
    std::string rest_url = "https://ciapi.cityindex.com/TradingAPI";
    cpr::Header session_header;
    nlohmann::json auth_payload;
    nlohmann::json session_payload;

    // =================================================================================================================
    // AUTHENTICATION
    // =================================================================================================================

    [[nodiscard]] bool set_trading_account_id();

    // =================================================================================================================
    // UTILITIES
    // =================================================================================================================

    [[nodiscard]] nlohmann::json make_network_call(cpr::Header const& header, cpr::Url const& url, std::string const& payload,
                                                   std::string const& type, std::string const& func_name);
};

}// namespace gaincapital

#endif
