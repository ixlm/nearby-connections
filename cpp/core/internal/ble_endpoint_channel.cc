// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "core/internal/ble_endpoint_channel.h"

#include <string>

namespace location {
namespace nearby {
namespace connections {

template <typename Platform>
Ptr<BLEEndpointChannel<Platform> >
BLEEndpointChannel<Platform>::createOutgoing(
    Ptr<MediumManager<Platform> > medium_manager, const string& channel_name,
    Ptr<BLESocket> ble_socket) {
  return MakePtr(
      new BLEEndpointChannel<Platform>(channel_name, ble_socket));
}

template <typename Platform>
Ptr<BLEEndpointChannel<Platform> >
BLEEndpointChannel<Platform>::createIncoming(
    Ptr<MediumManager<Platform> > medium_manager, const string& channel_name,
    Ptr<BLESocket> ble_socket) {
  return MakePtr(
      new BLEEndpointChannel<Platform>(channel_name, ble_socket));
}

template <typename Platform>
BLEEndpointChannel<Platform>::BLEEndpointChannel(
    const string& channel_name, Ptr<BLESocket> ble_socket)
    : BaseEndpointChannel<Platform>(channel_name,
                                    ble_socket->getInputStream(),
                                    ble_socket->getOutputStream()),
      ble_socket_(ble_socket) {}

template <typename Platform>
BLEEndpointChannel<Platform>::~BLEEndpointChannel() {}

template <typename Platform>
proto::connections::Medium BLEEndpointChannel<Platform>::getMedium() {
  return proto::connections::Medium::BLE;
}

template <typename Platform>
void BLEEndpointChannel<Platform>::closeImpl() {
  Exception::Value exception = ble_socket_->close();
  if (exception != Exception::NONE) {
    if (exception == Exception::IO) {
      // TODO(ahlee): Add logging.
    }
  }
}

}  // namespace connections
}  // namespace nearby
}  // namespace location
