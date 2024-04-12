/*
# ==================================================================================
# Copyright 2024 Alexandre Huff.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==================================================================================
*/

#include "e2mgr.hpp"

#include <sstream>
#include <ricxfcpp/xapp.hpp>
#include <ricxfcpp/RestClient.h>
#include <mdclog/mdclog.h>
#include"nlohmann/json.hpp"


E2MgrCli::E2MgrCli() {
    std::string plt_ns;
    if (char *envptr = std::getenv("PLT_NAMESPACE")) {
        plt_ns = envptr;
    } else {
        plt_ns = DEFAULT_PLT_NAMESPACE;
    }

    std::stringstream ss;
    // building e2mgr service name
    ss << "http://service-" << plt_ns << "-e2mgr-http." << plt_ns << ":3800";
    this->baseUrl = ss.str();
}

bool E2MgrCli::fetch_connected_e2_nodes() {
    xapp::cpprestclient client(this->baseUrl);
    xapp::response_t response = client.do_get("/v1/nodeb/states");
    if (response.status_code != 200) {
        mdclog_write(MDCLOG_ERR, "Unable to fetch connected E2 Nodes from E2MGR. HTTP status code is %ld", response.status_code);
        return false;
    }

    std::string inventory_name;
    std::string plmnid;
    std::string nbId;
    /*
        Parsing something like this
        [
            {
                "inventoryName": "gnb_734_373_00000001",
                "globalNbId": {
                    "plmnId": "373437",
                    "nbId": "00000000000000000000000000001"
                },
                "connectionStatus": "CONNECTED"
            }
        ]

    */
    for (auto &element : response.body) {
        inventory_name.clear();
        plmnid.clear();
        nbId.clear();

        for (auto &val : element.items()) {
            if (val.key() == "connectionStatus") {
                if (val.value() == "CONNECTED") {
                    continue;
                } else {
                    break;  // we only want the connected e2 nodes
                }
            }

            if (val.key() == "inventoryName") {
                inventory_name = val.value();

            } else if (val.key() == "globalNbId") {
                for (auto &e2 : val.value().items()) {
                    if (e2.key() == "plmnId") {
                        plmnid = e2.value();

                    } else if (e2.key() == "nbId") {
                        nbId = e2.value();
                    }
                }
            }

        }

        if (inventory_name.empty() || plmnid.empty() || nbId.empty()) {
            mdclog_write(MDCLOG_ERR, "Unable to fetch all inventoryName, plmnId, and nbId from %s", element.dump().c_str());
            continue;
        }
        if (!add_e2_node_inventory_name(plmnid, nbId, inventory_name)) {
            mdclog_write(MDCLOG_ERR, "Unable to map inventoryName=%s with plmnId=%s and nbId=%s",
                inventory_name.c_str(), plmnid.c_str(), nbId.c_str());
        }
    }

    mdclog_write(MDCLOG_INFO, "E2 Node list has been fetched and size is %lu", this->e2nodes.size());

    return true;
}

std::string E2MgrCli::get_e2_node_inventory_name(std::string plmn_id, std::string nb_id) {
    std::string str;

    std::string key = plmn_id + nb_id;

    auto e2node = e2nodes.find(key);

    if (e2node != e2nodes.end()) {
        str = e2node->second;
    }

    return str;
}

bool E2MgrCli::add_e2_node_inventory_name(std::string plmn_id, std::string nb_id, std::string inventory_name) {
    std::string key = plmn_id + nb_id;

    auto it = e2nodes.emplace(key, inventory_name);

    return it.second;
}
