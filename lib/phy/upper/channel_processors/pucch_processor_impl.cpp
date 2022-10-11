/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "pucch_processor_impl.h"

using namespace srsgnb;

pucch_processor_result pucch_processor_impl::process(const resource_grid_reader&  grid,
                                                     const format1_configuration& config)
{
  // Prepare channel estimation.
  dmrs_pucch_processor::config_t estimator_config;
  estimator_config.format               = pucch_format::FORMAT_1;
  estimator_config.slot                 = config.common.slot;
  estimator_config.cp                   = config.common.cp;
  estimator_config.start_symbol_index   = config.start_symbol_index;
  estimator_config.nof_symbols          = config.nof_symbols;
  estimator_config.starting_prb         = config.common.starting_prb + config.common.bwp_start_rb;
  estimator_config.intra_slot_hopping   = config.common.second_hop_prb.has_value();
  estimator_config.second_hop_prb       = config.common.second_hop_prb.has_value()
                                              ? (config.common.second_hop_prb.value() + config.common.bwp_start_rb)
                                              : estimator_config.starting_prb;
  estimator_config.initial_cyclic_shift = config.initial_cyclic_shift;
  estimator_config.time_domain_occ      = config.time_domain_occ;
  estimator_config.n_id                 = config.common.n_id;
  estimator_config.n_id_0               = config.common.n_id_0;
  estimator_config.ports                = config.common.ports;

  // Unused channel estimator parameters for this format.
  estimator_config.group_hopping   = pucch_group_hopping::NEITHER;
  estimator_config.nof_prb         = 0;
  estimator_config.additional_dmrs = false;

  // Perform channel estimation.
  channel_estimator->estimate(estimates, grid, estimator_config);

  // Prepare detector configuration.
  pucch_detector::format1_configuration detector_config;

  // Actual message detection.
  pucch_uci_message message = detector->detect(grid, estimates, detector_config);

  // Prepare result.
  pucch_processor_result result;
  result.csi     = estimates.get_channel_state_information();
  result.message = message;

  return result;
}

pucch_processor_result pucch_processor_impl::process(const resource_grid_reader&  grid,
                                                     const format2_configuration& config)
{
  pucch_processor_result result;
  result.message.full_payload =
      span<uint8_t>(result.message.data)
          .first(config.common.nof_sr + config.common.nof_harq_ack + config.common.nof_csi_part1);
  result.message.harq_ack = result.message.full_payload.first(config.common.nof_harq_ack);
  result.message.csi_part1 =
      result.message.full_payload.subspan(config.common.nof_harq_ack, config.common.nof_csi_part1);
  result.message.sr =
      result.message.full_payload.subspan(config.common.nof_harq_ack + config.common.nof_sr, config.common.nof_sr);

  dmrs_pucch_processor::config_t estimator_config;
  channel_estimator->estimate(estimates, grid, estimator_config);

  result.csi = estimates.get_channel_state_information();

  span<log_likelihood_ratio>               llr = span<log_likelihood_ratio>(temp_llr).first(0);
  pucch_demodulator::format2_configuration demod_config;
  demodulator->demodulate(llr, grid, estimates, demod_config);

  uci_decoder::configuration decoder_config;
  result.message.status = decoder->decode(result.message.full_payload, llr, decoder_config);

  return result;
}