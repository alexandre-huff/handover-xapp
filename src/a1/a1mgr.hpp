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

#ifndef A1_HANDLER_HPP
#define A1_HANDLER_HPP

#include <string>
#include <vector>

typedef struct {
    std::string mcc;
    std::string mnc;
    std::string e2_node_id;
    std::vector<std::string> ues;   // list of imsi values
} e2_node_policy_t;

enum class A1Status {
    OK,
    ERROR,
    DELETED
};

class A1Manager {
public:
    A1Manager(std::string xapp_id) : handler_id(xapp_id) { };
    ~A1Manager() { };

    void parse_a1_policy(const std::string &message);
    std::string serialize_a1_response(A1Status status);

    std::string getOperation() const { return operation; };
    std::string getPolicyTypeId() const { return policy_type_id; };
    std::string getPolicyInstanceId() const { return policy_instance_id; };
    std::string getHandlerId() const { return handler_id; };
    std::string getPayload() const { return payload; };

    std::vector<e2_node_policy_t> const &getPolicyData() const { return policy_data; };

private:
    void parse_a1_payload();

    std::string operation;
	std::string policy_type_id;
	std::string policy_instance_id;
	std::string handler_id;
    std::string payload;

    std::vector<e2_node_policy_t> policy_data;
};

#endif
