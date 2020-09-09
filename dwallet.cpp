#include <arisen/arisen.hpp>
#include "abcounter.cpp"

using namespace arisen;

class [[arisen::contract("dwallet")]] dwallet : public arisen::contract {

public:

    dwallet(name receiver, name code, datastream<const char *> ds): contract(receiver);
    vector<string> blockchains {"bitcoin", "ethereum", "litecoin", "beos", "tron", "eos", "bitshares", "steem", "arisen"};

    [[arisen::action]]
    void key(name peepsid, std::string btype, std::string exkey) {
        require_auth(peepsid);
        dwallet_index dwallet(get_first_receiver(), get_first_receiver.value);
        auto iterator = dwallet.find(peepsid.value);
        if (iterator == dwallet.end()) {
            //extended keys only exist for Bitcoin, Ethereum, Litecoin and TRON
            if (btype.value == "bitcoin" || btype.value == "ethereum" || btype.value == "litecoin" || btype.value == "tron") {
                dwallet.store(peepsid, [&]( auto& row) {
                    row.key = peepsid;
                    row.btype = btype;
                    row.exkey = exkey;
                });
                send_summary(peepsid, "successfully added extended key");
                increment_counter(peepsid, "create");
            }
            else {
                send_summary(peepsid, "already has an extended key for the blockchain type.");
            }
        }
    } // end key action
    
    void add(name peepsid, std::string address, std::string btype) {
        require_auth(peepsid);
        dwallet_index dwallet(get_first_receiver(), get_first_receiver.value);
        auto iterator = dwallet.find(peepsid.value, address.value, btype.value);
        auto typelocator = find(blockchains.begin(), blockchains.end(), btype.value);
        if (typelocator != blockchains.end() && iterator == dwallet.end()) {
            dwallet.newaddy(peepsid, [&]( auto& row) {
                row.key = peepsid;
                row.address = address;
                row.btype = btype;
            });
            send_summary(peepsid, "successfully added a new address.");
            increment_counter(peepsid, "newaddy");
        } else if ( typelocator == blockchains.end()) {
            send_summary(peepsid, "cannot add address because blockchain type is invalid.");
        } else {
            send_summary(peepsid, "already has added this address.");
        }
    } // end add action

    void increment_counter(name peepsid, std::string type) {
        abcounter::count_action count("abcounter"_n, (get_self(), "active"_n));
        count.send(peepsid, type);
    }

private: 

    void send_summary(name peepsid, std::string message) {
        action(
            permission_level(get_self(), "active"_n),
            get_self(),
            "notify"_n,
            std::make_tuple(peepsid, name{peepsid}.to_string() + message)
        ).send();
    }
} // end contract