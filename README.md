
# Gain Capital API C++

## Table of Contents

* [Instructions](#Instructions)
    - [Downloading Dependencies](#Downloading-Dependencies)
* [Dependencies](#Dependencies)
* [License](#License)
* [Contributing](#Contribution)

## Instructions

This trading system comes with an API for Gain Capital's Forex.com. To make an account visit their website [Forex.com](https://www.forex.com). Once an account is made you will need to send them an email and request an API key.

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

# Dependencies

- [Boost](https://www.boost.org/) - *Must be installed by user*
- [CPR](https://github.com/libcpr/cpr) - Included in CMake Fetch Content
- [JSON](https://github.com/nlohmann/json) - .hpp file included in repository

## License

This software is distributed under the GNU license. Please read [LICENSE](https://github.com/andrew-drogalis/Forex-Trade-Execution-System/blob/main/LICENSE) for information on the software availability and distribution.


## Contribution

Please open an issue of if you have any questions, suggestions, or feedback.

Please submit bug reports, suggestions, and pull requests to the [GitHub issue tracker](https://github.com/andrew-drogalis/Forex-Trade-Execution-System/issues).
