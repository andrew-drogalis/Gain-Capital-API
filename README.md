
# Gain Capital API C++

## Table of Contents

* [Instructions](#Instructions)
    - [Downloading Dependencies](#Downloading-Dependencies)
    - [Initializing API](#Initializing-API)
    - [Authenticating Session](#Authenticating-Session)
    - [Account & Margin Information](#Account-&-Margin-Information)
    - [Market IDs & Information](#Market-IDs-&-Information)
    - [Fetching OHLC Data](#Fetching-OHLC-Data)
    - [Fetching Price Data](#Fetching-Price-Data)
    - [Placing Trades](#Placing-Trades)
    - [Monitoring Trades](#Monitoring-Trades)
    - [Canceling Active Orders](#Canceling-Active-Orders)
* [Installing](#Installing)
* [Dependencies](#Dependencies)
* [Lightstreamer](#Lightstreamer)
* [License](#License)
* [Contributing](#Contribution)

## Instructions

This library is dependent upon the API from Gain Capital's Forex.com. To make an account visit their website [Forex.com](https://www.forex.com). Once an account is made you will need to send them an email and request an API key.

### Downloading Dependencies

Please see a link to required dependencies [below](#Dependencies). If you are using Linux, the dependencies can be installed with the following commands below. Additionally, this repository comes with a .devcontainer directory. The .devcontainer has all the required dependencies and can be run inside a Docker container.

```
    Fedora:
        dnf install libmicrohttpd-devel
    Debian:
        apt-get install libmicrohttpd-devel
    Arch: 
        pacman -Ss libmicrohttpd
```

### Initializing API

```c
    #include <gain_capital_client.h>

    std::string const username = "Username", password = "Password", apikey = "ApiKey";

    // Initialize GCClient
    gaincapital::GCClient gc_client = gaincapital::GCClient(username, password, apikey);
```

### Authenticating Session

```c
    // Required for First Authentication
    auto authentication_response = gc_client.authenticate_session();

    if (! authentication_response)
    {
        std::cout << authentication_response.error().what() << '\n';
        return 1;
    }
```


### Account & Margin Information

```c
    // Get Account Information
    auto account_response = gc_client.get_account_info();

    if (! account_response)
    {
        std::cout << account_response.error().what() << '\n';
        return 1;
    }

    // Access Account Information Json Response
    nlohmann::json account_json = account_response.value();

    // Get Margin Information
    auto margin_response = gc_client.get_margin_info();

    if (! margin_response)
    {
        std::cout << margin_response.error().what() << '\n';
        return 1;
    }

    // Access Margin Information Json Response
    nlohmann::json margin_json = margin_response.value();

```

### Market IDs & Information

```c
    // Get Info for Each Market
    for (std::string const& market_name : currency_pairs)
    {
        // Get Market IDs
        auto market_id_response = gc_client.get_market_id(market_name);

        if (! market_id_response)
        {
            std::cout << market_id_response.error().what() << '\n';
            return 1;
        }
```

### Fetching OHLC Data

```c
    // Get OHLC Bars
    std::string const interval = "MINUTE";
    int const num_ticks = 10;
    auto ohlc_response = gc_client.get_ohlc(market_name, interval, num_ticks);

    if (! ohlc_response)
    {
        std::cout << ohlc_response.error().what() << '\n';
        return 1;
    }

    // Access OHLC Bars Json Response
    nlohmann::json ohlc_json = ohlc_response.value();
```

### Fetching Price Data

```c
    // Get Currency Prices
    auto price_response = gc_client.get_prices(market_name);

    if (! price_response)
    {
        std::cout << price_response.error().what() << '\n';
        return 1;
    }

    // Access Currency Prices Json Response
    nlohmann::json price_json = price_response.value();
```

### Placing Trades

```c
    // Place Market Order
    nlohmann::json trades_map_market = {};
    for (std::string const& symbol : currency_pairs) { trades_map_market[symbol] = {{"Direction", "sell"}, {"Quantity", 1000}}; }
    auto market_order_response = gc_client.trade_order(trades_map_market, "MARKET");

    if (! market_order_response)
    {
        std::cout << market_order_response.error().what() << '\n';
        return 1;
    }

    // Access Msrket Order Json Response
    nlohmann::json market_order_json = market_order_response.value(); 

    // Place Limit Order
    nlohmann::json trades_map_limit = {};
    for (std::string const& symbol : currency_pairs) 
    {
        float const mid_price = price_response[symbol][0]["Price"];
        float const trigger_price = mid_price * 1.1;
        float const stop_price = mid_price * 0.9;

        trades_map_limit[symbol] = {{"Direction", "buy"}, {"Quantity", 1000}, {"TriggerPrice", trigger_price}, {"StopPrice", stop_price}};
    }
    auto limit_order_response = gc_client.trade_order(trades_map_limit, "LIMIT");

    if (! limit_order_response)
    {
        std::cout << limit_order_response.error().what() << '\n';
        return 1;
    }

    // Access Limit Order Json Response
    nlohmann::json limit_order_json = limit_order_response.value(); 
```

### Monitoring Trades

```c
    // Get Open Positions
    auto open_position_response = gc_client.list_open_positions();

    if (! open_position_response)
    {
        std::cout << open_position_response.error().what() << '\n';
        return 1;
    }

    // Access Open Positions Json Response
    nlohmann::json open_position_json = open_position_response.value();

    // Get Active Orders
    auto active_order_response = gc_client.list_active_orders();

    if (! active_order_response)
    {
        std::cout << active_order_response.error().what() << '\n';
        return 1;
    }

    // Access Active Order Json Response
    nlohmann::json active_order_json = active_order_response.value();
```

### Canceling Active Orders

```c
    // Cancel Active Orders
    for (nlohmann::json& active_order : active_order_json)
    {
        // Cancel Market Orders
        if (active_order.contains("TradeOrder"))
        {
            std::string const order_id = active_order["TradeOrder"]["OrderId"].dump();
            auto cancel_order_response = gc_client.cancel_order(order_id);

            if (! cancel_order_response)
            {
                std::cout << cancel_order_response.error().what() << '\n';
                return 1;
            }
        }

        // Cancel Limit Orders
        if (active_order.contains("StopLimitOrder"))
        {
            std::string const order_id = active_order["StopLimitOrder"]["OrderId"].dump();
            auto cancel_order_response = gc_client.cancel_order(order_id);

            if (! cancel_order_response)
            {
                std::cout << cancel_order_response.error().what() << '\n';
                return 1;
            }
        }
    }
```

## Installing

To build the shared library and header files, run the commands below.

```
    $ mkdir gcapi_library
    $ cmake -S . -B gcapi_library
    $ cmake --build gcapi_library --target gain_capital_api
    $ cmake install gcapi_library
```

## Dependencies

- [libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/) - Required for Tests Only - *Must be installed by user*
- [CPR](https://github.com/libcpr/cpr) - Included in CMake Fetch Content
- [JSON](https://github.com/nlohmann/json) - .hpp file included in repository
- [httpmockserver](https://github.com/seznam/httpmockserver) - Required for Tests Only - Library included in repository

## Lightstreamer

Live data streaming is provided by the Lightstreamer client, currently this feature is not integrated into this repository. In the future, I hope to incorporate this feature, but please see the C++ Lightstreamer repository provided below. This will be the jumping off point for future integration into this repository.

- [Lightstreamer for C++](https://github.com/AndrewCarterUK/LightstreamerCpp)

## License

This software is distributed under the GNU license. Please read [LICENSE](https://github.com/andrew-drogalis/Gain-Capital-API-Cpp/blob/main/LICENSE) for information on the software availability and distribution.


## Contribution

Please open an issue of if you have any questions, suggestions, or feedback.

Please submit bug reports, suggestions, and pull requests to the [GitHub issue tracker](https://github.com/andrew-drogalis/Gain-Capital-API-Cpp/issues).
