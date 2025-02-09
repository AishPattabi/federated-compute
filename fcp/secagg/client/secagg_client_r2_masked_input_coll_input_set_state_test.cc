/*
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "fcp/secagg/client/secagg_client_r2_masked_input_coll_input_set_state.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/container/node_hash_map.h"
#include "fcp/secagg/client/other_client_state.h"
#include "fcp/secagg/client/secagg_client_aborted_state.h"
#include "fcp/secagg/client/secagg_client_r2_masked_input_coll_base_state.h"
#include "fcp/secagg/client/secagg_client_r3_unmasking_state.h"
#include "fcp/secagg/client/send_to_server_interface.h"
#include "fcp/secagg/client/state_transition_listener_interface.h"
#include "fcp/secagg/shared/aes_ctr_prng_factory.h"
#include "fcp/secagg/shared/aes_gcm_encryption.h"
#include "fcp/secagg/shared/aes_key.h"
#include "fcp/secagg/shared/compute_session_id.h"
#include "fcp/secagg/shared/input_vector_specification.h"
#include "fcp/secagg/shared/map_of_masks.h"
#include "fcp/secagg/shared/secagg_messages.pb.h"
#include "fcp/secagg/shared/secagg_vector.h"
#include "fcp/secagg/testing/fake_prng.h"
#include "fcp/secagg/testing/mock_send_to_server_interface.h"
#include "fcp/secagg/testing/mock_state_transition_listener.h"
#include "fcp/testing/testing.h"

namespace fcp {
namespace secagg {
namespace {

using ::testing::Eq;
using ::testing::Pointee;

// For testing purposes, make an AesKey out of a std::string.
AesKey MakeAesKey(const std::string& key) {
  EXPECT_THAT(key.size(), Eq(AesKey::kSize));
  return AesKey(reinterpret_cast<const uint8_t*>(key.c_str()));
}

// Default test session_id.
SessionId session_id = {"session id number, 32 bytes long."};

TEST(SecAggClientR2MaskedInputCollInputSetStateTest, IsAbortedReturnsFalse) {
  auto input_map = std::make_unique<SecAggVectorMap>();
  input_map->insert(std::make_pair("test", SecAggVector({2, 4, 6, 8}, 32)));
  auto input_vector_specs =
      std::make_unique<std::vector<InputVectorSpecification> >();
  input_vector_specs->push_back(InputVectorSpecification("test", 4, 32));
  MockSendToServerInterface* sender = new MockSendToServerInterface();
  MockStateTransitionListener* transition_listener =
      new MockStateTransitionListener();
  SecAggClientR2MaskedInputCollInputSetState r2_state(
      1,  // client_id
      4,  // minimum_surviving_neighbors_for_reconstruction
      6,  // number_of_alive_neighbors
      6,  // number_of_neighbors
      std::move(input_map), std::move(input_vector_specs),
      std::make_unique<std::vector<OtherClientState> >(
          6, OtherClientState::kAlive),
      std::make_unique<std::vector<AesKey> >(
          6, MakeAesKey("test 32 byte AES encryption key.")),
      std::make_unique<std::vector<AesKey> >(
          6, MakeAesKey("other test 32 byte AES prng key.")),
      std::make_unique<ShamirShare>(),
      std::make_unique<AesKey>(MakeAesKey("test 32 byte AES self prng key. ")),
      std::unique_ptr<SendToServerInterface>(sender),
      std::unique_ptr<StateTransitionListenerInterface>(transition_listener),
      std::make_unique<SessionId>(session_id),
      std::make_unique<AesCtrPrngFactory>());

  EXPECT_THAT(r2_state.IsAborted(), Eq(false));
}

TEST(SecAggClientR2MaskedInputCollInputSetStateTest,
     IsCompletedSuccessfullyReturnsFalse) {
  auto input_map = std::make_unique<SecAggVectorMap>();
  input_map->insert(std::make_pair("test", SecAggVector({2, 4, 6, 8}, 32)));
  auto input_vector_specs =
      std::make_unique<std::vector<InputVectorSpecification> >();
  input_vector_specs->push_back(InputVectorSpecification("test", 4, 32));
  MockSendToServerInterface* sender = new MockSendToServerInterface();
  MockStateTransitionListener* transition_listener =
      new MockStateTransitionListener();
  SecAggClientR2MaskedInputCollInputSetState r2_state(
      1,  // client_id
      4,  // minimum_surviving_neighbors_for_reconstruction
      6,  // number_of_alive_neighbors
      6,  // number_of_neighbors
      std::move(input_map), std::move(input_vector_specs),
      std::make_unique<std::vector<OtherClientState> >(
          6, OtherClientState::kAlive),
      std::make_unique<std::vector<AesKey> >(
          6, MakeAesKey("test 32 byte AES encryption key.")),
      std::make_unique<std::vector<AesKey> >(
          6, MakeAesKey("other test 32 byte AES prng key.")),
      std::make_unique<ShamirShare>(),
      std::make_unique<AesKey>(MakeAesKey("test 32 byte AES self prng key. ")),
      std::unique_ptr<SendToServerInterface>(sender),
      std::unique_ptr<StateTransitionListenerInterface>(transition_listener),
      std::make_unique<SessionId>(session_id),
      std::make_unique<AesCtrPrngFactory>());

  EXPECT_THAT(r2_state.IsCompletedSuccessfully(), Eq(false));
}

TEST(SecAggClientR2MaskedInputCollInputSetStateTest, StartRaisesErrorStatus) {
  auto input_map = std::make_unique<SecAggVectorMap>();
  input_map->insert(std::make_pair("test", SecAggVector({2, 4, 6, 8}, 32)));
  auto input_vector_specs =
      std::make_unique<std::vector<InputVectorSpecification> >();
  input_vector_specs->push_back(InputVectorSpecification("test", 4, 32));
  MockSendToServerInterface* sender = new MockSendToServerInterface();
  MockStateTransitionListener* transition_listener =
      new MockStateTransitionListener();
  SecAggClientR2MaskedInputCollInputSetState r2_state(
      1,  // client_id
      4,  // minimum_surviving_neighbors_for_reconstruction
      6,  // number_of_alive_neighbors
      6,  // number_of_neighbors
      std::move(input_map), std::move(input_vector_specs),
      std::make_unique<std::vector<OtherClientState> >(
          6, OtherClientState::kAlive),
      std::make_unique<std::vector<AesKey> >(
          6, MakeAesKey("test 32 byte AES encryption key.")),
      std::make_unique<std::vector<AesKey> >(
          6, MakeAesKey("other test 32 byte AES prng key.")),
      std::make_unique<ShamirShare>(),
      std::make_unique<AesKey>(MakeAesKey("test 32 byte AES self prng key. ")),
      std::unique_ptr<SendToServerInterface>(sender),
      std::unique_ptr<StateTransitionListenerInterface>(transition_listener),
      std::make_unique<SessionId>(session_id),
      std::make_unique<AesCtrPrngFactory>());

  EXPECT_THAT(r2_state.Start().ok(), Eq(false));
}

TEST(SecAggClientR2MaskedInputCollInputSetStateTest,
     SetInputRaisesErrorStatus) {
  auto input_map = std::make_unique<SecAggVectorMap>();
  input_map->insert(std::make_pair("test", SecAggVector({2, 4, 6, 8}, 32)));
  auto input_vector_specs =
      std::make_unique<std::vector<InputVectorSpecification> >();
  input_vector_specs->push_back(InputVectorSpecification("test", 4, 32));
  MockSendToServerInterface* sender = new MockSendToServerInterface();
  MockStateTransitionListener* transition_listener =
      new MockStateTransitionListener();
  SecAggClientR2MaskedInputCollInputSetState r2_state(
      1,  // client_id
      4,  // minimum_surviving_neighbors_for_reconstruction
      6,  // number_of_alive_neighbors
      6,  // number_of_neighbors
      std::move(input_map), std::move(input_vector_specs),
      std::make_unique<std::vector<OtherClientState> >(
          6, OtherClientState::kAlive),
      std::make_unique<std::vector<AesKey> >(
          6, MakeAesKey("test 32 byte AES encryption key.")),
      std::make_unique<std::vector<AesKey> >(
          6, MakeAesKey("other test 32 byte AES prng key.")),
      std::make_unique<ShamirShare>(),
      std::make_unique<AesKey>(MakeAesKey("test 32 byte AES self prng key. ")),
      std::unique_ptr<SendToServerInterface>(sender),
      std::unique_ptr<StateTransitionListenerInterface>(transition_listener),
      std::make_unique<SessionId>(session_id),
      std::make_unique<AesCtrPrngFactory>());

  EXPECT_THAT(r2_state.SetInput(std::make_unique<SecAggVectorMap>()).ok(),
              Eq(false));
}

TEST(SecAggClientR2MaskedInputCollInputSetStateTest,
     ErrorMessageRaisesErrorStatus) {
  auto input_map = std::make_unique<SecAggVectorMap>();
  input_map->insert(std::make_pair("test", SecAggVector({2, 4, 6, 8}, 32)));
  auto input_vector_specs =
      std::make_unique<std::vector<InputVectorSpecification> >();
  input_vector_specs->push_back(InputVectorSpecification("test", 4, 32));
  MockSendToServerInterface* sender = new MockSendToServerInterface();
  MockStateTransitionListener* transition_listener =
      new MockStateTransitionListener();
  SecAggClientR2MaskedInputCollInputSetState r2_state(
      1,  // client_id
      4,  // minimum_surviving_neighbors_for_reconstruction
      6,  // number_of_alive_neighbors
      6,  // number_of_neighbors
      std::move(input_map), std::move(input_vector_specs),
      std::make_unique<std::vector<OtherClientState> >(
          6, OtherClientState::kAlive),
      std::make_unique<std::vector<AesKey> >(
          6, MakeAesKey("test 32 byte AES encryption key.")),
      std::make_unique<std::vector<AesKey> >(
          6, MakeAesKey("other test 32 byte AES prng key.")),
      std::make_unique<ShamirShare>(),
      std::make_unique<AesKey>(MakeAesKey("test 32 byte AES self prng key. ")),
      std::unique_ptr<SendToServerInterface>(sender),
      std::unique_ptr<StateTransitionListenerInterface>(transition_listener),
      std::make_unique<SessionId>(session_id),
      std::make_unique<AesCtrPrngFactory>());

  EXPECT_THAT(r2_state.ErrorMessage().ok(), Eq(false));
}

TEST(SecAggClientR2MaskedInputCollInputSetStateTest,
     AbortReturnsValidAbortStateAndNotifiesServer) {
  auto input_map = std::make_unique<SecAggVectorMap>();
  input_map->insert(std::make_pair("test", SecAggVector({2, 4, 6, 8}, 32)));
  auto input_vector_specs =
      std::make_unique<std::vector<InputVectorSpecification> >();
  input_vector_specs->push_back(InputVectorSpecification("test", 4, 32));
  MockSendToServerInterface* sender = new MockSendToServerInterface();
  MockStateTransitionListener* transition_listener =
      new MockStateTransitionListener();
  SecAggClientR2MaskedInputCollInputSetState r2_state(
      1,  // client_id
      4,  // minimum_surviving_neighbors_for_reconstruction
      6,  // number_of_alive_neighbors
      6,  // number_of_neighbors
      std::move(input_map), std::move(input_vector_specs),
      std::make_unique<std::vector<OtherClientState> >(
          6, OtherClientState::kAlive),
      std::make_unique<std::vector<AesKey> >(
          6, MakeAesKey("test 32 byte AES encryption key.")),
      std::make_unique<std::vector<AesKey> >(
          6, MakeAesKey("other test 32 byte AES prng key.")),
      std::make_unique<ShamirShare>(),
      std::make_unique<AesKey>(MakeAesKey("test 32 byte AES self prng key. ")),
      std::unique_ptr<SendToServerInterface>(sender),
      std::unique_ptr<StateTransitionListenerInterface>(transition_listener),
      std::make_unique<SessionId>(session_id),
      std::make_unique<AesCtrPrngFactory>());

  std::string error_string =
      "Abort upon external request for reason <Abort reason>.";
  ClientToServerWrapperMessage expected_message;
  expected_message.mutable_abort()->set_diagnostic_info(error_string);
  EXPECT_CALL(*sender, Send(Pointee(EqualsProto(expected_message))));

  StatusOr<std::unique_ptr<SecAggClientState> > new_state =
      r2_state.Abort("Abort reason");
  ASSERT_THAT(new_state.ok(), Eq(true));
  EXPECT_THAT(new_state.value()->StateName(), Eq("ABORTED"));
  EXPECT_THAT(new_state.value()->ErrorMessage().value(), Eq(error_string));
}

TEST(SecAggClientR2MaskedInputCollInputSetStateTest,
     AbortFailureMessageCausesAbortWithoutNotifyingServer) {
  auto input_map = std::make_unique<SecAggVectorMap>();
  input_map->insert(std::make_pair("test", SecAggVector({2, 4, 6, 8}, 32)));
  auto input_vector_specs =
      std::make_unique<std::vector<InputVectorSpecification> >();
  input_vector_specs->push_back(InputVectorSpecification("test", 4, 32));
  MockSendToServerInterface* sender = new MockSendToServerInterface();
  MockStateTransitionListener* transition_listener =
      new MockStateTransitionListener();
  SecAggClientR2MaskedInputCollInputSetState r2_state(
      1,  // client_id
      4,  // minimum_surviving_neighbors_for_reconstruction
      6,  // number_of_alive_neighbors
      6,  // number_of_neighbors
      std::move(input_map), std::move(input_vector_specs),
      std::make_unique<std::vector<OtherClientState> >(
          6, OtherClientState::kAlive),
      std::make_unique<std::vector<AesKey> >(
          6, MakeAesKey("test 32 byte AES encryption key.")),
      std::make_unique<std::vector<AesKey> >(
          6, MakeAesKey("other test 32 byte AES prng key.")),
      std::make_unique<ShamirShare>(),
      std::make_unique<AesKey>(MakeAesKey("test 32 byte AES self prng key. ")),
      std::unique_ptr<SendToServerInterface>(sender),
      std::unique_ptr<StateTransitionListenerInterface>(transition_listener),
      std::make_unique<SessionId>(session_id),
      std::make_unique<AesCtrPrngFactory>());

  EXPECT_CALL(*sender, Send(::testing::_)).Times(0);
  ServerToClientWrapperMessage abort_message;
  abort_message.mutable_abort()->set_early_success(false);

  StatusOr<std::unique_ptr<SecAggClientState> > new_state =
      r2_state.HandleMessage(abort_message);
  ASSERT_TRUE(new_state.ok());
  EXPECT_THAT(new_state.value()->StateName(), Eq("ABORTED"));
  EXPECT_THAT(new_state.value()->ErrorMessage().value(),
              Eq("Aborting because of abort message from the server."));
}

TEST(SecAggClientR2MaskedInputCollInputSetStateTest,
     EarlySuccessMessageCausesTransitionToCompletedState) {
  auto input_map = std::make_unique<SecAggVectorMap>();
  input_map->insert(std::make_pair("test", SecAggVector({2, 4, 6, 8}, 32)));
  auto input_vector_specs =
      std::make_unique<std::vector<InputVectorSpecification> >();
  input_vector_specs->push_back(InputVectorSpecification("test", 4, 32));
  MockSendToServerInterface* sender = new MockSendToServerInterface();
  MockStateTransitionListener* transition_listener =
      new MockStateTransitionListener();
  SecAggClientR2MaskedInputCollInputSetState r2_state(
      1,  // client_id
      4,  // minimum_surviving_neighbors_for_reconstruction
      6,  // number_of_alive_neighbors
      6,  // number_of_neighbors
      std::move(input_map), std::move(input_vector_specs),
      std::make_unique<std::vector<OtherClientState> >(
          6, OtherClientState::kAlive),
      std::make_unique<std::vector<AesKey> >(
          6, MakeAesKey("test 32 byte AES encryption key.")),
      std::make_unique<std::vector<AesKey> >(
          6, MakeAesKey("other test 32 byte AES prng key.")),
      std::make_unique<ShamirShare>(),
      std::make_unique<AesKey>(MakeAesKey("test 32 byte AES self prng key. ")),
      std::unique_ptr<SendToServerInterface>(sender),
      std::unique_ptr<StateTransitionListenerInterface>(transition_listener),
      std::make_unique<SessionId>(session_id),
      std::make_unique<AesCtrPrngFactory>());

  EXPECT_CALL(*sender, Send(::testing::_)).Times(0);
  ServerToClientWrapperMessage abort_message;
  abort_message.mutable_abort()->set_early_success(true);

  StatusOr<std::unique_ptr<SecAggClientState> > new_state =
      r2_state.HandleMessage(abort_message);
  ASSERT_TRUE(new_state.ok());
  EXPECT_THAT(new_state.value()->StateName(), Eq("COMPLETED"));
}

TEST(SecAggClientR2MaskedInputCollInputSetStateTest,
     MaskedInputCollectionRequestIsHandledCorrectlyWhenNoClientsDie) {
  // In this test, the client under test is id 1, and there are 4 clients, all
  // alive.
  auto input_map = std::make_unique<SecAggVectorMap>();
  input_map->emplace("test", SecAggVector({2, 4, 6, 8}, 32));
  std::vector<InputVectorSpecification> input_vector_specs;
  input_vector_specs.push_back(InputVectorSpecification("test", 4, 32));
  MockSendToServerInterface* sender = new MockSendToServerInterface();
  MockStateTransitionListener* transition_listener =
      new MockStateTransitionListener();
  std::vector<AesKey> enc_keys = {
      MakeAesKey("other client encryption key 0000"),
      MakeAesKey("other client encryption key 1111"),
      MakeAesKey("other client encryption key 2222"),
      MakeAesKey("other client encryption key 3333")};
  std::vector<AesKey> other_client_prng_keys = {
      MakeAesKey("other client pairwise prng key 0"), AesKey(),
      MakeAesKey("other client pairwise prng key 2"),
      MakeAesKey("other client pairwise prng key 3")};
  SecAggClientR2MaskedInputCollInputSetState r2_state(
      1,  // client_id
      3,  // minimum_surviving_neighbors_for_reconstruction
      4,  // number_of_alive_neighbors
      4,  // number_of_neighbors
      std::move(input_map),
      std::make_unique<std::vector<InputVectorSpecification> >(
          input_vector_specs),
      std::make_unique<std::vector<OtherClientState> >(
          4, OtherClientState::kAlive),
      std::make_unique<std::vector<AesKey> >(enc_keys),
      std::make_unique<std::vector<AesKey> >(other_client_prng_keys),
      std::make_unique<ShamirShare>(),
      std::make_unique<AesKey>(MakeAesKey("test 32 byte AES self prng key. ")),
      std::unique_ptr<SendToServerInterface>(sender),
      std::unique_ptr<StateTransitionListenerInterface>(transition_listener),
      std::make_unique<SessionId>(session_id),
      std::make_unique<AesCtrPrngFactory>());

  // These are strings because they're for putting into protocol buffers.
  std::vector<std::string> expected_self_key_shares = {
      "shared self prng key for client #000", "",
      "shared self prng key for client #222",
      "shared self prng key for client #333"};
  std::vector<std::string> expected_pairwise_key_shares = {
      "shared pairwise prng key for client0", "",
      "shared pairwise prng key for client2",
      "shared pairwise prng key for client3"};

  ServerToClientWrapperMessage message;
  AesGcmEncryption encryptor;
  for (int i = 0; i < 4; ++i) {
    PairOfKeyShares key_shares_pair;
    key_shares_pair.set_noise_sk_share(expected_pairwise_key_shares[i]);
    key_shares_pair.set_prf_sk_share(expected_self_key_shares[i]);
    std::string serialized_pair = key_shares_pair.SerializeAsString();
    std::string ciphertext =
        encryptor.Encrypt(enc_keys[i], serialized_pair);
    message.mutable_masked_input_request()->add_encrypted_key_shares(
        ciphertext);
  }

  std::vector<AesKey> prng_keys_to_add = {
      MakeAesKey("test 32 byte AES self prng key. "),
      other_client_prng_keys[0]};
  std::vector<AesKey> prng_keys_to_subtract = {other_client_prng_keys[2],
                                               other_client_prng_keys[3]};

  auto map_of_masks =
      MapOfMasks(prng_keys_to_add, prng_keys_to_subtract, input_vector_specs,
                 session_id, AesCtrPrngFactory());

  std::vector<uint64_t> mask_vec = map_of_masks->at("test").GetAsUint64Vector();
  std::vector<uint64_t> input_vec = {2, 4, 6, 8};
  std::vector<uint64_t> sum_vec;
  uint64_t bit_width_bound = 32;

  for (int i = 0; i < 4; ++i) {
    sum_vec.push_back((mask_vec[i] + input_vec[i]) % bit_width_bound);
  }
  MaskedInputVector sum_vec_proto;
  sum_vec_proto.set_encoded_vector(
      SecAggVector(sum_vec, 32).GetAsPackedBytes());
  ClientToServerWrapperMessage expected_message;
  (*expected_message.mutable_masked_input_response()
        ->mutable_vectors())["test"] = sum_vec_proto;

  EXPECT_CALL(*sender, Send(Pointee(EqualsProto(expected_message))));

  StatusOr<std::unique_ptr<SecAggClientState> > new_state =
      r2_state.HandleMessage(message);
  ASSERT_TRUE(new_state.ok());
  EXPECT_THAT(new_state.value()->StateName(), Eq("R3_UNMASKING"));
}

TEST(SecAggClientR2MaskedInputCollInputSetStateTest,
     MaskedInputCollectionRequestIsHandledCorrectlyWithDeadClient) {
  // In this test, the client under test is id 1, and there are 4 clients.
  // Client 3 has died in this round.
  auto input_map = std::make_unique<SecAggVectorMap>();
  input_map->insert(std::make_pair("test", SecAggVector({2, 4, 6, 8}, 32)));
  std::vector<InputVectorSpecification> input_vector_specs;
  input_vector_specs.push_back(InputVectorSpecification("test", 4, 32));
  MockSendToServerInterface* sender = new MockSendToServerInterface();
  MockStateTransitionListener* transition_listener =
      new MockStateTransitionListener();
  std::vector<AesKey> enc_keys = {
      MakeAesKey("other client encryption key 0000"),
      MakeAesKey("other client encryption key 1111"),
      MakeAesKey("other client encryption key 2222"),
      MakeAesKey("other client encryption key 3333")};
  std::vector<AesKey> other_client_prng_keys = {
      MakeAesKey("other client pairwise prng key 0"), AesKey(),
      MakeAesKey("other client pairwise prng key 2"),
      MakeAesKey("other client pairwise prng key 3")};
  SecAggClientR2MaskedInputCollInputSetState r2_state(
      1,  // client_id
      3,  // minimum_surviving_neighbors_for_reconstruction
      4,  // number_of_alive_neighbors
      4,  // number_of_neighbors
      std::move(input_map),
      std::make_unique<std::vector<InputVectorSpecification> >(
          input_vector_specs),
      std::make_unique<std::vector<OtherClientState> >(
          4, OtherClientState::kAlive),
      std::make_unique<std::vector<AesKey> >(enc_keys),
      std::make_unique<std::vector<AesKey> >(other_client_prng_keys),
      std::make_unique<ShamirShare>(),
      std::make_unique<AesKey>(MakeAesKey("test 32 byte AES self prng key. ")),
      std::unique_ptr<SendToServerInterface>(sender),
      std::unique_ptr<StateTransitionListenerInterface>(transition_listener),
      std::make_unique<SessionId>(session_id),
      std::make_unique<AesCtrPrngFactory>());

  std::vector<std::string> expected_self_key_shares = {
      "shared self prng key for client #000", "",
      "shared self prng key for client #222",
      "shared self prng key for client #333"};
  std::vector<std::string> expected_pairwise_key_shares = {
      "shared pairwise prng key for client0", "",
      "shared pairwise prng key for client2",
      "shared pairwise prng key for client3"};

  ServerToClientWrapperMessage message;
  AesGcmEncryption encryptor;
  for (int i = 0; i < 3; ++i) {  // Exclude client 3.
    PairOfKeyShares key_shares_pair;
    key_shares_pair.set_noise_sk_share(expected_pairwise_key_shares[i]);
    key_shares_pair.set_prf_sk_share(expected_self_key_shares[i]);
    std::string serialized_pair = key_shares_pair.SerializeAsString();
    std::string ciphertext =
        encryptor.Encrypt(enc_keys[i], serialized_pair);
    message.mutable_masked_input_request()->add_encrypted_key_shares(
        ciphertext);
  }
  message.mutable_masked_input_request()->add_encrypted_key_shares("");

  std::vector<AesKey> prng_keys_to_add = {
      MakeAesKey("test 32 byte AES self prng key. "),
      other_client_prng_keys[0]};
  std::vector<AesKey> prng_keys_to_subtract = {other_client_prng_keys[2]};

  auto map_of_masks =
      MapOfMasks(prng_keys_to_add, prng_keys_to_subtract, input_vector_specs,
                 session_id, AesCtrPrngFactory());

  std::vector<uint64_t> mask_vec = map_of_masks->at("test").GetAsUint64Vector();
  std::vector<uint64_t> input_vec = {2, 4, 6, 8};
  std::vector<uint64_t> sum_vec;
  uint64_t bit_width_bound = 32;

  for (int i = 0; i < 4; ++i) {
    sum_vec.push_back((mask_vec[i] + input_vec[i]) % bit_width_bound);
  }
  MaskedInputVector sum_vec_proto;
  sum_vec_proto.set_encoded_vector(
      SecAggVector(sum_vec, 32).GetAsPackedBytes());
  ClientToServerWrapperMessage expected_message;
  (*expected_message.mutable_masked_input_response()
        ->mutable_vectors())["test"] = sum_vec_proto;

  EXPECT_CALL(*sender, Send(Pointee(EqualsProto(expected_message))));

  StatusOr<std::unique_ptr<SecAggClientState> > new_state =
      r2_state.HandleMessage(message);
  ASSERT_TRUE(new_state.ok());
  EXPECT_THAT(new_state.value()->StateName(), Eq("R3_UNMASKING"));
}

TEST(SecAggClientR2MaskedInputCollInputSetStateTest,
     MaskedInputCollectionRequestCausesAbortIfTooManyDeadClients) {
  // In this test, the client under test is id 1, and there are 4 clients.
  // Clients 3 and 4 died, and we need 3 clients to continue, so we should
  // abort.
  auto input_map = std::make_unique<SecAggVectorMap>();
  input_map->insert(std::make_pair("test", SecAggVector({2, 4, 6, 8}, 32)));
  std::vector<InputVectorSpecification> input_vector_specs;
  input_vector_specs.push_back(InputVectorSpecification("test", 4, 32));
  MockSendToServerInterface* sender = new MockSendToServerInterface();
  MockStateTransitionListener* transition_listener =
      new MockStateTransitionListener();
  std::vector<AesKey> enc_keys = {
      MakeAesKey("other client encryption key 0000"),
      MakeAesKey("other client encryption key 1111"),
      MakeAesKey("other client encryption key 2222"),
      MakeAesKey("other client encryption key 3333")};
  std::vector<AesKey> other_client_prng_keys = {
      MakeAesKey("other client pairwise prng key 0"), AesKey(),
      MakeAesKey("other client pairwise prng key 2"),
      MakeAesKey("other client pairwise prng key 3")};
  SecAggClientR2MaskedInputCollInputSetState r2_state(
      1,  // client_id
      3,  // minimum_surviving_neighbors_for_reconstruction
      4,  // number_of_alive_neighbors
      4,  // number_of_neighbors
      std::move(input_map),
      std::make_unique<std::vector<InputVectorSpecification> >(
          input_vector_specs),
      std::make_unique<std::vector<OtherClientState> >(
          4, OtherClientState::kAlive),
      std::make_unique<std::vector<AesKey> >(enc_keys),
      std::make_unique<std::vector<AesKey> >(other_client_prng_keys),
      std::make_unique<ShamirShare>(),
      std::make_unique<AesKey>(MakeAesKey("test 32 byte AES self prng key. ")),
      std::unique_ptr<SendToServerInterface>(sender),
      std::unique_ptr<StateTransitionListenerInterface>(transition_listener),
      std::make_unique<SessionId>(session_id),
      std::make_unique<AesCtrPrngFactory>());

  std::vector<std::string> expected_self_key_shares = {
      "shared self prng key for client #000", "",
      "shared self prng key for client #222",
      "shared self prng key for client #333"};
  std::vector<std::string> expected_pairwise_key_shares = {
      "shared pairwise prng key for client0", "",
      "shared pairwise prng key for client2",
      "shared pairwise prng key for client3"};

  ServerToClientWrapperMessage message;
  AesGcmEncryption encryptor;
  for (int i = 0; i < 2; ++i) {  // Exclude clients 3 & 4
    PairOfKeyShares key_shares_pair;
    key_shares_pair.set_noise_sk_share(expected_pairwise_key_shares[i]);
    key_shares_pair.set_prf_sk_share(expected_self_key_shares[i]);
    std::string serialized_pair = key_shares_pair.SerializeAsString();
    std::string ciphertext =
        encryptor.Encrypt(enc_keys[i], serialized_pair);
    message.mutable_masked_input_request()->add_encrypted_key_shares(
        ciphertext);
  }
  message.mutable_masked_input_request()->add_encrypted_key_shares("");
  message.mutable_masked_input_request()->add_encrypted_key_shares("");

  std::string error_string =
      "There are not enough clients to complete this protocol session. "
      "Aborting.";
  ClientToServerWrapperMessage expected_message;
  expected_message.mutable_abort()->set_diagnostic_info(error_string);
  EXPECT_CALL(*sender, Send(Pointee(EqualsProto(expected_message))));

  StatusOr<std::unique_ptr<SecAggClientState> > new_state =
      r2_state.HandleMessage(message);
  ASSERT_THAT(new_state.ok(), Eq(true));
  EXPECT_THAT(new_state.value()->StateName(), Eq("ABORTED"));
  EXPECT_THAT(new_state.value()->ErrorMessage().value(), Eq(error_string));
}

}  // namespace
}  // namespace secagg
}  // namespace fcp
