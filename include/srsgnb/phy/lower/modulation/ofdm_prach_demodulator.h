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

#include "srsgnb/phy/support/prach_buffer.h"
#include "srsgnb/ran/phy_time_unit.h"
#include "srsgnb/ran/prach/prach_preamble_format.h"
#include "srsgnb/ran/subcarrier_spacing.h"

namespace srsgnb {

/// \brief OFDM PRACH demodulator.
///
/// Performs OFDM demodulation for all the symbols contained in a PRACH frequency- and time-domain occasion as per
/// TS38.211 Section 5.3.2.
///
class ofdm_prach_demodulator
{
public:
  /// Collects the necessary parameters to demodulate a PRACH frequency- and time-domain occasion.
  struct configuration {
    /// PRACH preamble format.
    preamble_format format;
    /// PRACH frequency location (see \ref prach_context::rb_offset).
    unsigned rb_offset;
    /// PUSCH subcarrier spacing.
    subcarrier_spacing pusch_scs;
  };

  /// Default destructor.
  virtual ~ofdm_prach_demodulator() = default;

  /// \brief Demodulates a PRACH occasion as described in TS38.211 Section 5.3.2.
  ///
  /// An assertion is triggered if the input size is less than the PRACH window size. The PRACH window includes a
  /// cyclic prefix of length \f$N_{CP,l}^{RA}\f$) and a time-domain sequence of length \f$N_u\f$.
  ///
  /// \param[out] buffer    Frequency-domain signal corresponding to one PRACH receive occasion.
  /// \param[in] input      PRACH receive window baseband samples.
  /// \param[in] slot_index Slot index within the subframe to demodulate.
  virtual void demodulate(prach_buffer& buffer, span<const cf_t> input, const configuration& config) = 0;
};
} // namespace srsgnb