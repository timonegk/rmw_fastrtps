// Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <unordered_set>

#include "rmw/impl/cpp/macros.hpp"

#include "event_helpers.hpp"
#include "rmw_fastrtps_shared_cpp/custom_event_info.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "types/event_types.hpp"

static const std::unordered_set<rmw_event_type_t> g_rmw_event_type_set{
  RMW_EVENT_LIVELINESS_CHANGED,
  RMW_EVENT_REQUESTED_DEADLINE_MISSED,
  RMW_EVENT_LIVELINESS_LOST,
  RMW_EVENT_OFFERED_DEADLINE_MISSED,
  RMW_EVENT_MESSAGE_LOST,
  RMW_EVENT_OFFERED_QOS_INCOMPATIBLE,
  RMW_EVENT_REQUESTED_QOS_INCOMPATIBLE
};

namespace rmw_fastrtps_shared_cpp
{
namespace internal
{

bool is_event_supported(rmw_event_type_t event_type)
{
  return g_rmw_event_type_set.count(event_type) == 1;
}

rmw_qos_policy_kind_t dds_qos_policy_to_rmw_qos_policy(
  eprosima::fastdds::dds::QosPolicyId_t policy_id)
{
  using eprosima::fastdds::dds::QosPolicyId_t;

  switch (policy_id) {
    case QosPolicyId_t::DURABILITY_QOS_POLICY_ID:
      return RMW_QOS_POLICY_DURABILITY;
    case QosPolicyId_t::DEADLINE_QOS_POLICY_ID:
      return RMW_QOS_POLICY_DEADLINE;
    case QosPolicyId_t::LIVELINESS_QOS_POLICY_ID:
      return RMW_QOS_POLICY_LIVELINESS;
    case QosPolicyId_t::RELIABILITY_QOS_POLICY_ID:
      return RMW_QOS_POLICY_RELIABILITY;
    case QosPolicyId_t::HISTORY_QOS_POLICY_ID:
      return RMW_QOS_POLICY_HISTORY;
    case QosPolicyId_t::LIFESPAN_QOS_POLICY_ID:
      return RMW_QOS_POLICY_LIFESPAN;
    default:
      return RMW_QOS_POLICY_INVALID;
  }
}

}  // namespace internal

rmw_ret_t
__rmw_init_event(
  const char * identifier,
  rmw_event_t * rmw_event,
  const char * topic_endpoint_impl_identifier,
  void * data,
  rmw_event_type_t event_type)
{
  RMW_CHECK_ARGUMENT_FOR_NULL(identifier, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_ARGUMENT_FOR_NULL(rmw_event, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_ARGUMENT_FOR_NULL(topic_endpoint_impl_identifier, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_ARGUMENT_FOR_NULL(data, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    topic endpoint,
    topic_endpoint_impl_identifier,
    identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  if (!internal::is_event_supported(event_type)) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING("provided event_type is not supported by %s", identifier);
    return RMW_RET_UNSUPPORTED;
  }

  rmw_event->implementation_identifier = topic_endpoint_impl_identifier;
  rmw_event->data = data;
  rmw_event->event_type = event_type;

  return RMW_RET_OK;
}

rmw_ret_t
__rmw_event_set_callback(
  rmw_event_t * rmw_event,
  rmw_event_callback_t callback,
  const void * user_data)
{
  auto custom_event_info = static_cast<CustomEventInfo *>(rmw_event->data);

  auto listener = custom_event_info->getListener();

  return listener->set_on_new_event_callback(
    rmw_event->event_type, user_data, callback);
}

}  // namespace rmw_fastrtps_shared_cpp
