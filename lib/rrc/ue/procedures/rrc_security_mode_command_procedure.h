/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "../rrc_ue_context.h"
#include "rrc_ue_event_manager.h"
#include "srsgnb/asn1/rrc_nr/rrc_nr.h"
#include "srsgnb/rrc/rrc_du.h"
#include "srsgnb/rrc/rrc_ue.h"
#include "srsgnb/support/async/async_task.h"
#include "srsgnb/support/async/eager_async_task.h"

namespace srsgnb {
namespace srs_cu_cp {

struct rrc_security_context {
  security::sec_as_key k;
  std::array<bool, 3>  supported_int_algos;
  std::array<bool, 3>  supported_enc_algos;
};

/// \brief Handles the setup of AS security keys in the RRC UE.
/// TODO Add seqdiag
class rrc_security_mode_command_procedure
{
public:
  rrc_security_mode_command_procedure(rrc_ue_context_t&                           context_,
                                      rrc_security_context                        sec_ctx,
                                      const byte_buffer&                          du_to_cu_container_,
                                      rrc_ue_security_mode_command_proc_notifier& rrc_ue_notifier_,
                                      rrc_ue_event_manager&                       ev_mng_,
                                      srslog::basic_logger&                       logger_);

  void operator()(coro_context<async_task<void>>& ctx);

  static const char* name() { return "RRC Security Mode Command Procedure"; }

private:
  /// \remark Select security algorithms based on the UE capabilities
  /// provided by the NGAP.
  bool select_security_algo();

  /// \remark Send RRC Security Mode Command, see section 5.3.3 in TS 36.331
  void send_rrc_security_mode_command();

  rrc_ue_context_t                        context;
  rrc_security_context                    sec_ctx;
  const asn1::rrc_nr::rrc_setup_request_s request;
  const byte_buffer&                      du_to_cu_container;
  const asn1::rrc_nr::pdcp_cfg_s          srb1_pdcp_cfg;

  rrc_ue_security_mode_command_proc_notifier& rrc_ue;    // handler to the parent RRC UE object
  rrc_ue_event_manager&                       event_mng; // event manager for the RRC UE entity
  srslog::basic_logger&                       logger;

  rrc_transaction               transaction;
  eager_async_task<rrc_outcome> task;

  security::integrity_algorithm int_algo  = {};
  security::ciphering_algorithm ciph_algo = {};

  const unsigned rrc_smc_timeout_ms =
      1000; // arbitrary timeout for RRC SMC procedure, UE will be removed if timer fires
};

/// \brief Fills ASN.1 RRC Setup struct.
/// \param[out] rrc_smc The RRC security mode command ASN.1 struct to fill.
/// \param[in] int_algo The selected integrity protection algorithm.
/// \param[in] ciph_algo The selected ciphering algorithm.
/// \param[in] rrc_transaction_id The RRC transaction id.
inline void fill_asn1_rrc_smc_msg(asn1::rrc_nr::security_mode_cmd_s&   rrc_smc,
                                  const security::integrity_algorithm& int_algo,
                                  const security::ciphering_algorithm& ciph_algo,
                                  uint8_t                              rrc_transaction_id)
{
  using namespace asn1::rrc_nr;
  security_mode_cmd_ies_s& smc_ies = rrc_smc.crit_exts.set_security_mode_cmd();
  rrc_smc.rrc_transaction_id       = rrc_transaction_id;

  // Set security algorithms
  security_cfg_smc_s&       security_cfg_smc       = smc_ies.security_cfg_smc;
  security_algorithm_cfg_s& security_algorithm_cfg = security_cfg_smc.security_algorithm_cfg;

  security_algorithm_cfg.integrity_prot_algorithm_present = true;
  switch (int_algo) {
    case security::integrity_algorithm::nia0:
      security_algorithm_cfg.integrity_prot_algorithm = integrity_prot_algorithm_e::nia0;
      break;
    case security::integrity_algorithm::nia1:
      security_algorithm_cfg.integrity_prot_algorithm = integrity_prot_algorithm_e::nia1;
      break;
    case security::integrity_algorithm::nia2:
      security_algorithm_cfg.integrity_prot_algorithm = integrity_prot_algorithm_e::nia2;
      break;
    case security::integrity_algorithm::nia3:
      security_algorithm_cfg.integrity_prot_algorithm = integrity_prot_algorithm_e::nia3;
      break;
  }
  switch (ciph_algo) {
    case security::ciphering_algorithm::nea0:
      security_algorithm_cfg.ciphering_algorithm = ciphering_algorithm_e::nea0;
      break;
    case security::ciphering_algorithm::nea1:
      security_algorithm_cfg.ciphering_algorithm = ciphering_algorithm_e::nea1;
      break;
    case security::ciphering_algorithm::nea2:
      security_algorithm_cfg.ciphering_algorithm = ciphering_algorithm_e::nea2;
      break;
    case security::ciphering_algorithm::nea3:
      security_algorithm_cfg.ciphering_algorithm = ciphering_algorithm_e::nea3;
      break;
  }
}

} // namespace srs_cu_cp
} // namespace srsgnb