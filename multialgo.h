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

#ifdef __cplusplus
}
#endif

#endif
