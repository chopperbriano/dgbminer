#include "skein-gate.h"
#include "sph_skein.h"

bool register_skein_algo( algo_gate_t* gate )
{
#if defined (SKEIN_8WAY)
    gate->optimizations = AVX2_OPT | AVX512_OPT;
    gate->scanhash  = (void*)&scanhash_skein_8way;
    gate->hash      = (void*)&skeinhash_8way;
#elif defined (SKEIN_4WAY)
    gate->optimizations = AVX2_OPT | AVX512_OPT | SHA_OPT;
    gate->scanhash  = (void*)&scanhash_skein_4way;
    gate->hash      = (void*)&skeinhash_4way;
#else
    gate->optimizations = AVX2_OPT | AVX512_OPT | SHA_OPT;
    gate->scanhash  = (void*)&scanhash_skein;
    gate->hash      = (void*)&skeinhash;
#endif
    return true;
};

/* register_skein2_algo removed: skein2 is not a DGB algo. */


