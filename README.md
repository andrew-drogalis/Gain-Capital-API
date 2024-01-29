
# Gain Capital API C++

## Table of Contents

* [Instructions](#Instructions)
    - [Downloading Dependencies](#Downloading-Dependencies)
    - [Account & Margin Information](#Account-&-Margin-Information)
    - [Market IDs & Information](#Market-IDs-&-Information)
    - [Fetching OHLC Data](#Fetching-OHLC-Data)
    - [Placing Trades](#Placing-Trades)
    - [Monitoring Trades](#Monitoring-Trades)
* [Dependencies](#Dependencies)
* [Lightstreamer](#Lightstreamer)
* [License](#License)
* [Contributing](#Contribution)

## Instructions

This library is dependent upon the API from Gain Capital's Forex.com. To make an account visit their website [Forex.com](https://www.forex.com). Once an account is made you will need to send them an email and request an API key.

### Downloading Dependencies

<b>This repository has been designed for use with Linux Systems.</b> 

The libraries must be re-built from source to target another platform. Please see a link to required dependencies [below](#Dependencies). 

Boost can be installed on Linux with the following commands. 

```
    Fedora:
        dnf install boost-devel
    Debian:
        apt-get install libboost-all-dev 
    Arch: 
        pacman -Ss boost
```

### Account & Margin Information

```cpp


```

### Market IDs & Information

```cpp


```

### Fetching OHLC Data

```cpp


```

### Placing Trades

```cpp

    
```

### Monitoring Trades

```cpp


```

## Dependencies

- [Boost](https://www.boost.org/) - *Must be installed by user*
- [CPR](https://github.com/libcpr/cpr) - Included in CMake Fetch Content
- [JSON](https://github.com/nlohmann/json) - .hpp file included in repository

## Lightstreamer



## License

This software is distributed under the GNU license. Please read [LICENSE](https://github.com/andrew-drogalis/Gain-Capital-API-Cpp/blob/main/LICENSE) for information on the software availability and distribution.


## Contribution

Please open an issue of if you have any questions, suggestions, or feedback.

Please submit bug reports, suggestions, and pull requests to the [GitHub issue tracker](https://github.com/andrew-drogalis/Gain-Capital-API-Cpp/issues).
