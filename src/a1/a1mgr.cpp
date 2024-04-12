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

#include "a1mgr.hpp"

#include <exception>
#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/schema.h>
#include <rapidjson/prettywriter.h>

void A1Manager::parse_a1_policy(const std::string &message) {
    rapidjson::Document doc;
	if (doc.Parse<rapidjson::kParseStopWhenDoneFlag>(message.c_str()).HasParseError()) {
		throw std::runtime_error("Unable to parse A1 JSON message");
	}

	//Extract Operation
	rapidjson::Pointer op("/operation");
	rapidjson::Value *ref_op = op.Get(doc);
	if (ref_op == NULL){
		throw std::runtime_error("Unable to extract operation from A1 message");
	}
	this->operation = ref_op->GetString();

	// Extract policy id type
	rapidjson::Pointer type("/policy_type_id");
	rapidjson::Value *ref_type = type.Get(doc);
	if (ref_type == NULL){
		throw std::runtime_error("Unable to extract policy type id from A1 message");
	}
	this->policy_type_id = ref_type->GetString();

	// Extract policy instance id
	rapidjson::Pointer instance("/policy_instance_id");
	rapidjson::Value *ref_instance = instance.Get(doc);
	if (ref_instance == NULL){
		throw std::runtime_error("Unable to extract policy instance id from A1 message");
	}
	this->policy_instance_id = ref_instance->GetString();

	// Extract payload
	rapidjson::Pointer payload_ptr("/payload");
	rapidjson::Value *payload_val = payload_ptr.Get(doc);
	if (ref_instance == NULL){
		throw std::runtime_error("Unable to extract payload from A1 message");
	}
	this->payload = payload_val->GetString();

	parse_a1_payload();
}

void A1Manager::parse_a1_payload() {
	rapidjson::Document doc;
	if (doc.Parse<rapidjson::kParseStopWhenDoneFlag>(this->payload.c_str()).HasParseError()){
		throw std::runtime_error("Unable to parse payload in A1 JSON message");
	}

	rapidjson::Pointer e2_list("/E2NodeList");
	rapidjson::Value *ref_e2_list = e2_list.Get(doc);
	if (ref_e2_list == NULL) {
		std::stringstream ss;
		ss << "Unable to extract E2NodeList from: " << doc.GetString();
        throw std::runtime_error(ss.str());
	}

	for (auto &v : ref_e2_list->GetArray()) {
		e2_node_policy_t e2_node;
		e2_node.mcc = v["mcc"].GetString();
		e2_node.mnc = v["mnc"].GetString();
		e2_node.e2_node_id = v["nodebid"].GetString();

		if (!v["UEList"].IsArray()) {
			rapidjson::StringBuffer buffer;
			rapidjson::PrettyWriter<rapidjson::StringBuffer> pwriter(buffer);
			v.Accept(pwriter);
			std::string str = buffer.GetString();
			throw std::runtime_error("Unable to extract UEList from: " + str);
		}

		auto ues = v["UEList"].GetArray();
		if (ues.Empty()) {
			throw std::runtime_error("There is no UE in UEList");
		}

		for (auto &ue : ues) {
			e2_node.ues.emplace_back(ue["imsi"].GetString());
		}

		this->policy_data.emplace_back(e2_node);
	}
}

std::string A1Manager::serialize_a1_response(A1Status status) {
    rapidjson::Document doc;
	doc.SetObject();

	rapidjson::Document::AllocatorType& alloc = doc.GetAllocator();

    rapidjson::Value handler_id;	// this is the xapp_id from registration
    handler_id.SetString(this->handler_id.c_str(), this->handler_id.length(), alloc);

	std::string str_status;
	switch (status) {
	case A1Status::OK:
		str_status = "OK";
		break;
	case A1Status::ERROR:
		str_status = "ERROR";
		break;
	case A1Status::DELETED:
		str_status = "DELETED";
		break;
	default:
		str_status = "UNKNOWN";
		break;
	}

    rapidjson::Value jstatus;
    jstatus.SetString(str_status.c_str(), str_status.length(), alloc);

    rapidjson::Value policy_id;
    policy_id.SetInt(stoi(this->policy_type_id));

	rapidjson::Value policy_instance_id;
	policy_instance_id.SetString(this->policy_instance_id.c_str(), this->policy_instance_id.length(), alloc);

    doc.AddMember("policy_type_id", policy_id, alloc);
	doc.AddMember("policy_instance_id", policy_instance_id, alloc);
    doc.AddMember("handler_id", handler_id, alloc);
    doc.AddMember("status", jstatus, alloc);

	rapidjson::StringBuffer sbuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sbuf);
    doc.Accept(writer);

	return sbuf.GetString();
}
