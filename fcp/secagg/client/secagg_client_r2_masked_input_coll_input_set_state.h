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

#ifndef FCP_SECAGG_CLIENT_SECAGG_CLIENT_R2_MASKED_INPUT_COLL_INPUT_SET_STATE_H_
#define FCP_SECAGG_CLIENT_SECAGG_CLIENT_R2_MASKED_INPUT_COLL_INPUT_SET_STATE_H_

#include <cstdint>
#include <memory>
#include <set>
#include <string>

#include "fcp/base/monitoring.h"
#include "fcp/secagg/client/other_client_state.h"
#include "fcp/secagg/client/secagg_client_r2_masked_input_coll_base_state.h"
#include "fcp/secagg/client/secagg_client_state.h"
#include "fcp/secagg/client/send_to_server_interface.h"
#include "fcp/secagg/client/state_transition_listener_interface.h"
#include "fcp/secagg/shared/aes_key.h"
#include "fcp/secagg/shared/aes_prng_factory.h"
#include "fcp/secagg/shared/compute_session_id.h"
#include "fcp/secagg/shared/input_vector_specification.h"
#include "fcp/secagg/shared/map_of_masks.h"
#include "fcp/secagg/shared/secagg_messages.pb.h"

namespace fcp {
namespace secagg {

// This class represents the client's Round 2: Masked Input Collection state
// with the input already set. This state should transition to the
// Round 3: Unmasking state, but can also transition directly to the Completed
// or Aborted states.

class SecAggClientR2MaskedInputCollInputSetState
    : public SecAggClientR2MaskedInputCollBaseState {
 public:
  SecAggClientR2MaskedInputCollInputSetState(
      uint32_t client_id, uint32_t minimum_surviving_clients_for_reconstruction,
      uint32_t number_of_alive_clients, uint32_t number_of_clients,
      std::unique_ptr<SecAggVectorMap> input_map,
      std::unique_ptr<std::vector<InputVectorSpecification> >
          input_vector_specs,
      std::unique_ptr<std::vector<OtherClientState> > other_client_states,
      std::unique_ptr<std::vector<AesKey> > other_client_enc_keys,
      std::unique_ptr<std::vector<AesKey> > other_client_prng_keys,
      std::unique_ptr<ShamirShare> own_self_key_share,
      std::unique_ptr<AesKey> self_prng_key,
      std::unique_ptr<SendToServerInterface> sender,
      std::unique_ptr<StateTransitionListenerInterface> transition_listener,
      std::unique_ptr<SessionId> session_id,
      std::unique_ptr<AesPrngFactory> prng_factory,
      AsyncAbort* async_abort = nullptr);

  ~SecAggClientR2MaskedInputCollInputSetState() override;

  StatusOr<std::unique_ptr<SecAggClientState> > HandleMessage(
      const ServerToClientWrapperMessage& message) override;

  // Returns the name of this state, "R2_MASKED_INPUT_COLL_INPUT_SET".
  std::string StateName() const override;

 private:
  const uint32_t client_id_;
  const uint32_t minimum_surviving_clients_for_reconstruction_;
  uint32_t number_of_alive_clients_;
  const uint32_t number_of_clients_;
  std::unique_ptr<SecAggVectorMap> input_map_;
  std::unique_ptr<std::vector<InputVectorSpecification> > input_vector_specs_;
  std::unique_ptr<std::vector<OtherClientState> > other_client_states_;
  std::unique_ptr<std::vector<AesKey> > other_client_enc_keys_;
  std::unique_ptr<std::vector<AesKey> > other_client_prng_keys_;
  std::unique_ptr<ShamirShare> own_self_key_share_;
  std::unique_ptr<AesKey> self_prng_key_;
  std::unique_ptr<SessionId> session_id_;
  std::unique_ptr<AesPrngFactory> prng_factory_;
};

}  // namespace secagg
}  // namespace fcp

#endif  // FCP_SECAGG_CLIENT_SECAGG_CLIENT_R2_MASKED_INPUT_COLL_INPUT_SET_STATE_H_
