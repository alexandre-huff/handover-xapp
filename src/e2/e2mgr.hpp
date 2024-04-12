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

#ifndef E2_MGR_HPP
#define E2_MGR_HPP

#include <string>
#include <unordered_map>

class E2MgrCli {
public:
    E2MgrCli();

    bool fetch_connected_e2_nodes();
    std::string get_e2_node_inventory_name(std::string plmn_id, std::string nb_id);

private:
    bool add_e2_node_inventory_name(std::string plmn_id, std::string nb_id, std::string inventory_name);

    std::string baseUrl;
    std::unordered_map<std::string, std::string> e2nodes; // connected e2nodes concat(plmnid+nbId) are the keys and inventoryName is the value

    // FIXME what if <<<<<<<<<<<  map into map and use the default cpp hash functions <<<<<<<<<<<
    /*
                                            plmnid                nbId    inventoryName
        using Unordered_map = unordered_map<string, unordered_map<string, string>>;

        Unordered_map um;
        um["Region1"]["Candidate1"] = 10;
        cout << um["Region1"]["Candidate1"];    // 10
    */

   // FIXME another posibility would be concatenate the plmnid and nbId in a string and then use it as the key (seems easy)
   /*
                 plmnid+nbid, inventoryName
        unordered_map<string, string>cl
   */


};


#endif
