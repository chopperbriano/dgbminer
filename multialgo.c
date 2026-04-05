// DigiByte MultiAlgo awareness, isolated in its own translation unit so
// it can't interact (via link-layout / cache-line / optimizer quirks)
// with the main cpu-miner.c hot path, which previously broke TUI rendering
// for unknown reasons when this logic lived there directly.
//
// Public API:
//   ma_set_block_algo(expected, running)   // called from gbt_work_decode
//   ma_should_skip_submit()                // called from submit_solution
//   ma_get_block_algo()                    // read-only accessor for the TUI
//   ma_get_mismatch()                      // read-only accessor for the TUI

#include <string.h>
#include "multialgo.h"
#include "miner.h"

// Storage is STATIC (file-scope) and only mutated through the public API.
// No inline accessors, no inline setters. Every access goes through a
// non-inline function in this file. Deliberately verbose.

static char s_block_algo[16] = {0};
static int  s_mismatch       = 0;
static int  s_warned_once    = 0;

void ma_set_block_algo(const char *expected, const char *running)
{
    if (!expected || !*expected) return;

    size_t n = strlen(expected);
    if (n >= sizeof s_block_algo) n = sizeof s_block_algo - 1;
    memcpy(s_block_algo, expected, n);
    s_block_algo[n] = 0;

    int new_mismatch = (running && strcasecmp(expected, running) != 0) ? 1 : 0;
    if (new_mismatch != s_mismatch) {
        // reset the one-shot warning when state changes
        s_warned_once = 0;
    }
    s_mismatch = new_mismatch;
}

int ma_should_skip_submit(void)
{
    return s_mismatch;
}

const char *ma_get_block_algo(void)
{
    return s_block_algo;
}

int ma_get_mismatch(void)
{
    return s_mismatch;
}

int ma_take_warning_slot(void)
{
    // Called by the caller that logged the mismatch warning. Returns 1
    // exactly once per (state change); subsequent calls return 0 until
    // ma_set_block_algo flips the state and resets the slot.
    if (s_warned_once) return 0;
    s_warned_once = 1;
    return 1;
}
