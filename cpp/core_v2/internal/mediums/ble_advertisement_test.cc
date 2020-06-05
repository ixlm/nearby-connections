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

#include "core_v2/internal/mediums/ble_advertisement.h"

#include <algorithm>

#include "gtest/gtest.h"

namespace location {
namespace nearby {
namespace connections {
namespace mediums {
namespace {

const BleAdvertisement::Version kVersion = BleAdvertisement::Version::kV2;
const BleAdvertisement::SocketVersion kSocketVersion =
    BleAdvertisement::SocketVersion::kV2;
const char kServiceIDHashBytes[] = "\x0a\x0b\x0c";
const char kData[] =
    "How much wood can a woodchuck chuck if a wood chuck would chuck wood?";
// This corresponds to the length of a specific BleAdvertisement packed with the
// kData given above. Be sure to update this if kData ever changes.
const size_t kAdvertisementLength = 77;
const size_t kLongAdvertisementLength = kAdvertisementLength + 1000;

TEST(BleAdvertisementTest, ConstructionWorksV1) {
  ByteArray service_id_hash{kServiceIDHashBytes};
  ByteArray data{kData};

  BleAdvertisement ble_advertisement{BleAdvertisement::Version::kV1,
                                     BleAdvertisement::SocketVersion::kV1,
                                     service_id_hash, data};

  EXPECT_TRUE(ble_advertisement.IsValid());
  EXPECT_EQ(BleAdvertisement::Version::kV1, ble_advertisement.GetVersion());
  EXPECT_EQ(BleAdvertisement::SocketVersion::kV1,
            ble_advertisement.GetSocketVersion());
  EXPECT_EQ(service_id_hash, ble_advertisement.GetServiceIdHash());
  EXPECT_EQ(data.size(), ble_advertisement.GetData().size());
  EXPECT_EQ(data, ble_advertisement.GetData());
}

TEST(BleAdvertisementTest, ConstructionFailsWithBadVersion) {
  BleAdvertisement::Version bad_version =
      static_cast<BleAdvertisement::Version>(666);

  ByteArray service_id_hash{kServiceIDHashBytes};
  ByteArray data{kData};

  BleAdvertisement ble_advertisement{bad_version, kSocketVersion,
                                     service_id_hash, data};

  EXPECT_FALSE(ble_advertisement.IsValid());
}

TEST(BleAdvertisementTest, ConstructionFailsWithBadSocketVersion) {
  BleAdvertisement::SocketVersion bad_socket_version =
      static_cast<BleAdvertisement::SocketVersion>(666);

  ByteArray service_id_hash{kServiceIDHashBytes};
  ByteArray data{kData};

  BleAdvertisement ble_advertisement{kVersion, bad_socket_version,
                                     service_id_hash, data};

  EXPECT_FALSE(ble_advertisement.IsValid());
}

TEST(BleAdvertisementTest, ConstructionFailsWithShortServiceIdHash) {
  char short_service_id_hash_bytes[] = "\x0a\x0b";

  ByteArray bad_service_id_hash{short_service_id_hash_bytes};
  ByteArray data{kData};

  BleAdvertisement ble_advertisement{kVersion, kSocketVersion,
                                     bad_service_id_hash, data};

  EXPECT_FALSE(ble_advertisement.IsValid());
}

TEST(BleAdvertisementTest, ConstructionFailsWithLongServiceIdHash) {
  char long_service_id_hash_bytes[] = "\x0a\x0b\x0c\x0d";

  ByteArray bad_service_id_hash{long_service_id_hash_bytes};
  ByteArray data{kData};

  BleAdvertisement ble_advertisement{kVersion, kSocketVersion,
                                     bad_service_id_hash, data};

  EXPECT_FALSE(ble_advertisement.IsValid());
}

TEST(BleAdvertisementTest, ConstructionFailsWithLongData) {
  // BleAdvertisement shouldn't be able to support data with the max GATT
  // attribute length because it needs some room for the preceding fields.
  char long_data[512]{};

  ByteArray service_id_hash{kServiceIDHashBytes};
  ByteArray bad_data{long_data, 512};

  BleAdvertisement ble_advertisement{kVersion, kSocketVersion, service_id_hash,
                                     bad_data};

  EXPECT_FALSE(ble_advertisement.IsValid());
}

TEST(BleAdvertisementTest, ConstructionFromSerializedBytesWorks) {
  ByteArray service_id_hash{kServiceIDHashBytes};
  ByteArray data{kData};

  BleAdvertisement org_ble_advertisement{kVersion, kSocketVersion,
                                         service_id_hash, data};
  ByteArray ble_advertisement_bytes{org_ble_advertisement};
  BleAdvertisement ble_advertisement{ble_advertisement_bytes};

  EXPECT_TRUE(ble_advertisement.IsValid());
  EXPECT_EQ(kVersion, ble_advertisement.GetVersion());
  EXPECT_EQ(kSocketVersion, ble_advertisement.GetSocketVersion());
  EXPECT_EQ(service_id_hash, ble_advertisement.GetServiceIdHash());
  EXPECT_EQ(data.size(), ble_advertisement.GetData().size());
  EXPECT_EQ(data, ble_advertisement.GetData());
}

TEST(BleAdvertisementTest, ConstructionFromSerializedBytesWithEmptyDataWorks) {
  char empty_data[0]{};

  ByteArray service_id_hash{kServiceIDHashBytes};
  ByteArray data{empty_data};

  BleAdvertisement org_ble_advertisement{kVersion, kSocketVersion,
                                         service_id_hash, data};
  ByteArray ble_advertisement_bytes{org_ble_advertisement};
  BleAdvertisement ble_advertisement{ble_advertisement_bytes};

  EXPECT_TRUE(ble_advertisement.IsValid());
  EXPECT_EQ(kVersion, ble_advertisement.GetVersion());
  EXPECT_EQ(kSocketVersion, ble_advertisement.GetSocketVersion());
  EXPECT_EQ(service_id_hash, ble_advertisement.GetServiceIdHash());
  EXPECT_EQ(data.size(), ble_advertisement.GetData().size());
  EXPECT_EQ(data, ble_advertisement.GetData());
}

TEST(BleAdvertisementTest, ConstructionFromExtraSerializedBytesWorks) {
  ByteArray service_id_hash{kServiceIDHashBytes};
  ByteArray data{kData};

  BleAdvertisement org_ble_advertisement{kVersion, kSocketVersion,
                                         service_id_hash, data};
  ByteArray org_ble_advertisement_bytes{org_ble_advertisement};

  // Copy the bytes into a new array with extra bytes. We must explicitly
  // define how long our array is because we can't use variable length arrays.
  char raw_ble_advertisement_bytes[kLongAdvertisementLength]{};
  memcpy(raw_ble_advertisement_bytes, org_ble_advertisement_bytes.data(),
         std::min(sizeof(raw_ble_advertisement_bytes),
                  org_ble_advertisement_bytes.size()));

  // Re-parse the Ble advertisement using our extra long advertisement bytes.
  ByteArray long_ble_advertisement_bytes{raw_ble_advertisement_bytes,
                                         kLongAdvertisementLength};
  BleAdvertisement long_ble_advertisement{long_ble_advertisement_bytes};

  EXPECT_TRUE(long_ble_advertisement.IsValid());
  EXPECT_EQ(kVersion, long_ble_advertisement.GetVersion());
  EXPECT_EQ(kSocketVersion, long_ble_advertisement.GetSocketVersion());
  EXPECT_EQ(service_id_hash, long_ble_advertisement.GetServiceIdHash());
  EXPECT_EQ(data.size(), long_ble_advertisement.GetData().size());
  EXPECT_EQ(data, long_ble_advertisement.GetData());
}

TEST(BleAdvertisementTest, ConstructionFromNullBytesFails) {
  BleAdvertisement ble_advertisement{ByteArray{}};

  EXPECT_FALSE(ble_advertisement.IsValid());
}

TEST(BleAdvertisementTest, ConstructionFromShortLengthSerializedBytesFails) {
  ByteArray service_id_hash{kServiceIDHashBytes};
  ByteArray data{kData};

  BleAdvertisement org_ble_advertisement{kVersion, kSocketVersion,
                                         service_id_hash, data};
  ByteArray org_ble_advertisement_bytes{org_ble_advertisement};

  // Cut off the advertisement so that it's too short.
  ByteArray short_ble_advertisement_bytes{org_ble_advertisement_bytes.data(),
                                          7};
  BleAdvertisement short_ble_advertisement{short_ble_advertisement_bytes};

  EXPECT_FALSE(short_ble_advertisement.IsValid());
}

TEST(BleAdvertisementTest,
     ConstructionFromSerializedBytesWithInvalidDataLengthFails) {
  ByteArray service_id_hash{kServiceIDHashBytes};
  ByteArray data{kData};

  BleAdvertisement org_ble_advertisement{kVersion, kSocketVersion,
                                         service_id_hash, data};
  ByteArray org_ble_advertisement_bytes{org_ble_advertisement};

  // Corrupt the DATA_SIZE bits. Start by making a raw copy of the Ble
  // advertisement bytes so we can modify it. We must explicitly define how
  // long our array is because we can't use variable length arrays.
  char raw_ble_advertisement_bytes[kAdvertisementLength];
  memcpy(raw_ble_advertisement_bytes, org_ble_advertisement_bytes.data(),
         kAdvertisementLength);

  // The data size field lives in indices 4-7. Corrupt it.
  memset(raw_ble_advertisement_bytes + 4, 0xFF, 4);

  // Try to parse the Ble advertisement using our corrupted advertisement bytes.
  ByteArray corrupted_ble_advertisement_bytes{raw_ble_advertisement_bytes,
                                              kAdvertisementLength};
  BleAdvertisement corrupted_ble_advertisement{
      corrupted_ble_advertisement_bytes};

  EXPECT_FALSE(corrupted_ble_advertisement.IsValid());
}

}  // namespace
}  // namespace mediums
}  // namespace connections
}  // namespace nearby
}  // namespace location