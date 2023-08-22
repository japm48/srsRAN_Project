/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "gtpu_teid.h"
#include "fmt/format.h"
#include <cstdint>
#include <memory>
#include <string>

namespace srsran {

/// Port specified for Encapsulated T-PDUs,
/// TS 29.281 Sec. 4.4.2.3
constexpr unsigned GTPU_PORT = 2152;

/// \brief Configurable parameters for the GTP-U
struct gtpu_config {
  struct gtpu_rx_config {
    gtpu_teid_t local_teid;
  } rx;
  struct gtpu_tx_config {
    gtpu_teid_t peer_teid;
    std::string peer_addr;
    uint16_t    peer_port;
  } tx;
};

} // namespace srsran

//
// Formatters
//
namespace fmt {

// RX config
template <>
struct formatter<srsran::gtpu_config::gtpu_rx_config> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx) -> decltype(ctx.begin())
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const srsran::gtpu_config::gtpu_rx_config& cfg, FormatContext& ctx)
      -> decltype(std::declval<FormatContext>().out())
  {
    return format_to(ctx.out(), "local_teid={}", cfg.local_teid);
  }
};

// TX config
template <>
struct formatter<srsran::gtpu_config::gtpu_tx_config> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx) -> decltype(ctx.begin())
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const srsran::gtpu_config::gtpu_tx_config& cfg, FormatContext& ctx)
      -> decltype(std::declval<FormatContext>().out())
  {
    return format_to(ctx.out(), "peer_teid={} peer_addr={} peer_port={}", cfg.peer_teid, cfg.peer_addr, cfg.peer_port);
  }
};

// GTP-U config
template <>
struct formatter<srsran::gtpu_config> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx) -> decltype(ctx.begin())
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const srsran::gtpu_config& cfg, FormatContext& ctx) -> decltype(std::declval<FormatContext>().out())
  {
    return format_to(ctx.out(), "{} {}", cfg.rx, cfg.tx);
  }
};

} // namespace fmt
