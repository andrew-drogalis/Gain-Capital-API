
# Gain Capital API C++

## Table of Contents

* [Instructions](#Instructions)
    - [Downloading Dependencies](#Downloading-Dependencies)
    - [Initializing the Client](#Initializing-the-Client)
    - [Authenticating the Session](#Authenticating-the-Session)
    - [Account and Margin Information](#Account-and-Margin-Information)
    - [Getting Market IDs](#Getting-Market-IDs)
    - [Fetching OHLC Data](#Fetching-OHLC-Data)
    - [Fetching Price Data](#Fetching-Price-Data)
    - [Placing Market Orders](#Placing-Market-Orders)
    - [Placing Limit Orders](#Placing-Limit-Orders)
    - [Monitoring Trades](#Monitoring-Trades)
    - [Canceling Active Orders](#Canceling-Active-Orders)
* [Installing](#Installing)
* [Dependencies](#Dependencies)
* [Lightstreamer](#Lightstreamer)
* [License](#License)
* [Contributing](#Contribution)

## Instructions

A prerequisite for using this library is having an account with Gain Capital's Forex.com. To make an account visit their website [Forex.com](https://www.forex.com). Additionally, an API key will need to be requested from their trading department, see the website for more information.

### Downloading Dependencies

Please see a link to all the required dependencies [below](#Dependencies). If you are using Linux, the dependencies can be installed with the following commands. 

```
Fedora:
    dnf install libmicrohttpd-devel
Debian:
    apt-get install libmicrohttpd-devel
Arch: 
    pacman -Ss libmicrohttpd
```

### Initializing the Client

```c
#include "gain_capital_client.h"

std::string const username = "Username", password = "Password", apikey = "ApiKey";

gaincapital::GCClient gc_client = gaincapital::GCClient(username, password, apikey);
```

### Authenticating the Session

```c
// Must Be Run Before All Other API Requests
auto authentication_response = gc_client.authenticate_session();

if (! authentication_response)
{
    std::cout << "Error Location: " << authentication_response.error().where() << '\n';
    std::cout << authentication_response.error().what() << '\n';
    return 1;
}
```


### Account and Margin Information

```c
// Get Account Information
auto account_response = gc_client.get_account_info();

if (! account_response)
{
    std::cout << "Error Location: " << account_response.error().where() << '\n';
    std::cout << account_response.error().what() << '\n';
    return 1;
}

// Access Account Information Json Response
nlohmann::json account_json = account_response.value();

// Get Margin Information
auto margin_response = gc_client.get_margin_info();

if (! margin_response)
{
    std::cout << "Error Location: " << margin_response.error().where() << '\n';
    std::cout << margin_response.error().what() << '\n';
    return 1;
}

// Access Margin Information Json Response
nlohmann::json margin_json = margin_response.value();

```

### Getting Market IDs

```c
// Get Market ID for Each Market
for (std::string const& market_name : currency_pairs)
{
    // Get Market ID
    auto market_id_response = gc_client.get_market_id(market_name);

    if (! market_id_response)
    {
        std::cout << "Error Location: " << market_id_response.error().where() << '\n';
        std::cout << market_id_response.error().what() << '\n';
        return 1;
    }
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
    std::cout << "Error Location: " << ohlc_response.error().where() << '\n';
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
    std::cout << "Error Location: " << price_response.error().where() << '\n';
    std::cout << price_response.error().what() << '\n';
    return 1;
}

// Access Currency Prices Json Response
nlohmann::json price_json = price_response.value();
```

### Placing Market Orders

```c
// Place Market Order
nlohmann::json trades_map_market = {};

for (std::string const& symbol : currency_pairs) { trades_map_market[symbol] = {{"Direction", "sell"}, {"Quantity", 1000}}; }

auto market_order_response = gc_client.trade_order(trades_map_market, "MARKET");

if (! market_order_response)
{
    std::cout << "Error Location: " << market_order_response.error().where() << '\n';
    std::cout << market_order_response.error().what() << '\n';
    return 1;
}

// Access Msrket Order Json Response
nlohmann::json market_order_json = market_order_response.value(); 
```
### Placing Limit Orders

```c
// Place Limit Order
nlohmann::json trades_map_limit = {};

for (std::string const& symbol : currency_pairs) 
{
    float const mid_price = price_response["PriceTicks"][0]["Price"];
    float const trigger_price = mid_price * 1.1;
    float const stop_price = mid_price * 0.9;

    trades_map_limit[symbol] = {{"Direction", "buy"}, {"Quantity", 1000}, {"TriggerPrice", trigger_price}, {"StopPrice", stop_price}};
}

auto limit_order_response = gc_client.trade_order(trades_map_limit, "LIMIT");

if (! limit_order_response)
{
    std::cout << "Error Location: " << limit_order_response.error().where() << '\n';
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
    std::cout << "Error Location: " << open_position_response.error().where() << '\n';
    std::cout << open_position_response.error().what() << '\n';
    return 1;
}

// Access Open Positions Json Response
nlohmann::json open_position_json = open_position_response.value();

// Get Active Orders
auto active_order_response = gc_client.list_active_orders();

if (! active_order_response)
{
    std::cout << "Error Location: " << active_order_response.error().where() << '\n';
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
            std::cout << "Error Location: " << cancel_order_response.error().where() << '\n';
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
            std::cout << "Error Location: " << cancel_order_response.error().where() << '\n';
            std::cout << cancel_order_response.error().what() << '\n';
            return 1;
        }
    }
}
```

## Installing

To build and install the shared library, run the commands below.

```
    $ mkdir gcapi_library
    $ cmake -S . -B gcapi_library
    $ cmake --build gcapi_library --target gain_capital_api
    $ cmake install gcapi_library
```

## Dependencies


This repository contains a .devcontainer directory. The .devcontainer has all the required dependencies and can be run inside Docker with the Dev Containers VSCode extension.

#### Included In Repository

- [C++ Requests Library](https://github.com/libcpr/cpr) 
- [Nlohmann JSON Library](https://github.com/nlohmann/json) 
- [httpmockserver](https://github.com/seznam/httpmockserver) | Testing Only

#### User Install Required

- [Google Tests](https://github.com/google/googletest) | Testing Only
- [libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/) | Testing Only

## Lightstreamer

Live data streaming is provided by the Lightstreamer client, currently this feature is not integrated into this repository. If you are interested in getting started, see the C++ Lightstreamer repository provided below. This will be the jumping off point for future integration into this repository.

- [Lightstreamer for C++](https://github.com/AndrewCarterUK/LightstreamerCpp)

## License

This software is distributed under the GNU license. Please read [LICENSE](https://github.com/andrew-drogalis/Gain-Capital-API-Cpp/blob/main/LICENSE) for information on the software availability and distribution.


## Contribution

Please open an issue of if you have any questions, suggestions, or feedback.

Please submit bug reports, suggestions, and pull requests to the [GitHub issue tracker](https://github.com/andrew-drogalis/Gain-Capital-API-Cpp/issues).
