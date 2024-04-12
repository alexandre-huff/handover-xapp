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

cmake_minimum_required(VERSION 3.16)

project(
    Handover-xApp
    VERSION 1.0.0
    LANGUAGES CXX C
)

# set( CMAKE_POSITION_INDEPENDENT_CODE ON )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g ")
set(CMAKE_CPP_FLAGS "${CMAKE_CPP_FLAGS} -g ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17")

add_definitions(-DASN_DISABLE_OER_SUPPORT)

include(GNUInstallDirs)

add_subdirectory(ASN1c/e2ap)
add_subdirectory(ASN1c/e2sm)
add_subdirectory(src)
