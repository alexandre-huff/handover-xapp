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

add_executable(handover-xapp main.cpp)

set_target_properties(handover-xapp PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_link_libraries(handover-xapp PRIVATE
    a1_objects
    e2_objects
    e2ap
    e2sm_rc
    ricxfcpp
    rmr_si
    pthread
    mdclog
    cpprest
    crypto
    nlohmann_json_schema_validator
    pistache
)

target_include_directories(handover-xapp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

add_subdirectory(a1)
add_subdirectory(e2)

install(
    TARGETS handover-xapp
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
