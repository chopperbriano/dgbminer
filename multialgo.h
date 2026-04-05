#ifndef MULTIALGO_H__
#define MULTIALGO_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

void         ma_set_block_algo(const char *expected, const char *running);
int          ma_should_skip_submit(void);
const char * ma_get_block_algo(void);
int          ma_get_mismatch(void);
int          ma_take_warning_slot(void);

// RPC connection state tracking. Call ma_rpc_failed() whenever an
// upstream RPC call returns an error and ma_rpc_ok() whenever one
// succeeds. ma_is_disconnected() is true iff the most recent call
// was a failure.
void         ma_rpc_failed(void);
void         ma_rpc_ok(void);
int          ma_is_disconnected(void);

#ifdef __cplusplus
}
#endif

#endif
