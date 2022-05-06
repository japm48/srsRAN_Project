
#include "mac_dl_processor.h"
#include "sched_config_helpers.h"
#include "srsgnb/mac/mac_cell_result.h"
#include "srsgnb/scheduler/sched_configuration_helpers.h"
#include "srsgnb/scheduler/scheduler_configurator.h"

using namespace srsgnb;

static void fill_sched_cell_config_msg(cell_configuration_request_message& sched_cell_cfg,
                                       const mac_cell_configuration&       cell_cfg,
                                       const ssb_assembler&                ssb_cfg)
{
  sched_cell_cfg = make_sched_cell_configuration_request_message(cell_cfg);

  // Copy SSB parameters
  // TODO: Remove these parameters from ssb_config.
  sched_cell_cfg.ssb_config.L_max           = ssb_cfg.get_ssb_L_max();
  sched_cell_cfg.ssb_config.ssb_case        = ssb_cfg.get_ssb_case();
  sched_cell_cfg.ssb_config.paired_spectrum = ssb_cfg.get_ssb_paired_spectrum();
}

mac_dl_processor::mac_dl_processor(mac_common_config_t&    cfg_,
                                   mac_sched_configurator& sched_cfg_,
                                   mac_scheduler&        sched_,
                                   du_rnti_table&          rnti_table_) :
  cfg(cfg_), logger(cfg.logger), sched_cfg(sched_cfg_), ue_mng(rnti_table_), sched_obj(sched_)
{}

bool mac_dl_processor::has_cell(du_cell_index_t cell_index) const
{
  return cell_index < MAX_NOF_DU_CELLS and cells[cell_index] != nullptr;
}

void mac_dl_processor::add_cell(const mac_cell_configuration& cell_cfg)
{
  srsran_assert(not has_cell(cell_cfg.cell_index), "Overwriting existing cell is invalid.");
  // Create one cell.
  cells[cell_cfg.cell_index] = std::make_unique<mac_dl_cell_processor>(cfg, cell_cfg, sched_obj, ue_mng);

  // Fill sched config msg and pass it to the scheduler.
  cell_configuration_request_message sched_msg{};
  fill_sched_cell_config_msg(sched_msg, cell_cfg, cells[cell_cfg.cell_index]->get_ssb_configuration());
  sched_obj.handle_cell_configuration_request(sched_msg);
}

void mac_dl_processor::remove_cell(du_cell_index_t cell_index)
{
  // TODO: remove cell from scheduler.

  // Remove cell from cell manager.
  cells[cell_index].reset();
}

async_task<bool> mac_dl_processor::add_ue(const mac_ue_create_request_message& request)
{
  return launch_async([this, request](coro_context<async_task<bool> >& ctx) {
    CORO_BEGIN(ctx);

    // 1. Change to respective DL executor
    CORO_AWAIT(execute_on(cfg.dl_exec_mapper.executor(request.cell_index)));

    // 2. Insert UE and DL bearers
    ue_mng.add_ue(request);

    // 3. Create UE in scheduler.
    log_proc_started(logger, request.ue_index, request.crnti, "Sched UE create");
    CORO_AWAIT(sched_cfg.handle_ue_creation_request(request));
    log_proc_completed(logger, request.ue_index, request.crnti, "Sched UE create");

    // 4. Change back to CTRL executor before returning
    CORO_AWAIT(execute_on(cfg.ctrl_exec));

    CORO_RETURN(true);
  });
}

async_task<void> mac_dl_processor::remove_ue(const mac_ue_delete_request_message& request)
{
  return launch_async([this, request](coro_context<async_task<void> >& ctx) {
    CORO_BEGIN(ctx);

    // 1. Change to respective DL executor
    CORO_AWAIT(execute_on(cfg.dl_exec_mapper.executor(request.cell_index)));

    // 2. Remove UE from scheduler
    CORO_AWAIT(sched_cfg.handle_ue_deletion_request(request));

    // 3. Remove UE associated DL channels
    ue_mng.remove_ue(request.ue_index);

    // 4. Change back to CTRL executor before returning
    CORO_AWAIT(execute_on(cfg.ctrl_exec));

    CORO_RETURN();
  });
}

async_task<bool> mac_dl_processor::reconfigure_ue(const mac_ue_reconfiguration_request_message& request)
{
  return launch_async([this, request](coro_context<async_task<bool> >& ctx) {
    CORO_BEGIN(ctx);

    // 1. Change to respective DL executor
    CORO_AWAIT(execute_on(cfg.dl_exec_mapper.executor(request.cell_index)));

    // 2. Remove UE DL bearers
    ue_mng.remove_bearers(request.ue_index, request.bearers_to_rem);

    // 3. AddMod UE DL bearers
    ue_mng.addmod_bearers(request.ue_index, request.bearers_to_addmod);

    // 4. Configure UE in Scheduler
    log_proc_started(logger, request.ue_index, request.crnti, "Sched UE Config");
    CORO_AWAIT(sched_cfg.handle_ue_reconfiguration_request(request));
    log_proc_completed(logger, request.ue_index, request.crnti, "Sched UE Config");

    // 5. Change back to CTRL executor before returning
    CORO_AWAIT(execute_on(cfg.ctrl_exec));

    CORO_RETURN(true);
  });
}
