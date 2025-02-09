/*
 * Copyright 2021 Google LLC
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
#ifndef FCP_CLIENT_OPSTATS_OPSTATS_LOGGER_H_
#define FCP_CLIENT_OPSTATS_OPSTATS_LOGGER_H_

#include <string>

#include "fcp/client/opstats/opstats_db.h"
#include "fcp/client/stats.h"
#include "fcp/protos/federated_api.pb.h"
#include "fcp/protos/opstats.pb.h"

namespace fcp {
namespace client {
namespace opstats {

// Base no-op class for the OpStats logger.
class OpStatsLogger {
 public:
  OpStatsLogger() {}

  explicit OpStatsLogger(bool opstats_enabled)
      : opstats_enabled_(opstats_enabled), db_(std::make_unique<OpStatsDb>()) {}

  virtual ~OpStatsLogger() {}

  // Log a checkin accepted event and the corresponding task name.
  virtual void AddCheckinAcceptedEventWithTaskName(
      const std::string& task_name) {}

  // Log an event.
  virtual void AddEvent(OperationalStats::Event::EventKind event) {}

  // Log an event and corresponding error message.
  virtual void AddEventWithErrorMessage(
      OperationalStats::Event::EventKind event,
      const std::string& error_message) {}

  // Log info associated with a dataset created for a given collection. If this
  // is called multiple times for the same collection, the example counts and
  // sizes should be aggregated.
  virtual void UpdateDatasetStats(const std::string& collection_uri,
                                  int additional_example_count,
                                  int64_t additional_example_size_bytes) {}

  // Log network stats, replacing any old stats for the run.
  virtual void SetNetworkStats(const NetworkStats& network_stats) {}

  // Log the retry window, replacing any old retry window. Ignore any retry
  // token in the retry window message.
  virtual void SetRetryWindow(
      google::internal::federatedml::v2::RetryWindow retry_window) {}

  // Get the underlying opstats database.
  virtual OpStatsDb* GetOpStatsDb() { return db_.get(); }

  // Whether opstats is enabled.
  virtual bool IsOpStatsEnabled() const { return opstats_enabled_; }

  // Syncs all logged events to storage.
  virtual absl::Status CommitToStorage() { return absl::OkStatus(); }

 private:
  bool opstats_enabled_;
  std::unique_ptr<OpStatsDb> db_;
};

}  // namespace opstats
}  // namespace client
}  // namespace fcp

#endif  // FCP_CLIENT_OPSTATS_OPSTATS_LOGGER_H_
