/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "gtpu_pdu_test.h"
#include "../../lib/gtpu/gtpu.h"
#include "srsgnb/support/test_utils.h"
#include "srsgnb/support/timers.h"
#include <gtest/gtest.h>
#include <queue>

using namespace srsgnb;

/// \brief Test correct creation of PDCP TX  entity
TEST_F(gtpu_test, pack_unpack)
{
  srsgnb::test_delimit_logger delimiter("GTP-U unpack/pack test");
  byte_buffer                 tst_vec{gtpu_ping_vec};
  gtpu_header                 hdr;
  bool                        read_ok = gtpu_read_header(hdr, tst_vec, logger);
  ASSERT_EQ(read_ok, true);
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}