
#include <gain_capital_api.h>
#include <string>
#include <vector>

using namespace std;

int main () {
    // Forex.com Account Info
    string username = "BLANK";
    string password = "BLANK";
    string api_key = "BLANK";

    // List of Currencies to Trade
    vector<string> currency_pairs = {"USD/CHF", "EUR/USD", "GBP/USD"};

    // Initalize GCapiClient
    GCapiClient gc_api = GCapiClient(username, password, api_key);

    // 

}