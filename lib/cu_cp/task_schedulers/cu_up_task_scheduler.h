/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/adt/slotted_array.h"
#include "srsran/cu_cp/cu_cp_types.h"
#include "srsran/support/async/fifo_async_task_scheduler.h"
#include "srsran/support/executors/task_executor.h"
#include "srsran/support/timers.h"

namespace srsran {
namespace srs_cu_cp {

/// \brief Service provided by CU-CP to schedule async tasks for a given CU-UP.
class cu_up_task_scheduler
{
public:
  explicit cu_up_task_scheduler(timer_manager&        timers_,
                                task_executor&        exec_,
                                unsigned              max_nof_cu_ups,
                                srslog::basic_logger& logger_);
  ~cu_up_task_scheduler() = default;

  // CU-UP task scheduler
  void handle_cu_up_async_task(cu_up_index_t cu_up_index, async_task<void>&& task);

  unique_timer   make_unique_timer();
  timer_manager& get_timer_manager();

private:
  timer_manager&        timers;
  task_executor&        exec;
  srslog::basic_logger& logger;

  // task event loops indexed by cu_up_index
  slotted_array<fifo_async_task_scheduler, MAX_NOF_CU_UPS> cu_up_ctrl_loop;
};

} // namespace srs_cu_cp
} // namespace srsran
