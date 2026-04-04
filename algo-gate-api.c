#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <memory.h>
#include <unistd.h>
#include "algo-gate-api.h"
#include <windows.h>
#include "algo/qubit/qubit-gate.h"
#include "miner.h"

extern enum algos opt_algo;
extern BOOL opt_benchmark;
extern struct work_restart* work_restart;


void do_nothing   () {}
BOOL return_true  () { return TRUE;  }
BOOL return_false () { return FALSE; }
void *return_null () { return NULL;  }

void algo_not_tested()
{
  applog( LOG_WARNING,"Algo %s has not been tested live. It may not work",
          algo_names[opt_algo] );
  applog(LOG_WARNING,"and bad things may happen. Use at your own risk.");
}

void four_way_not_tested()
{
  applog( LOG_WARNING,"Algo %s has not been tested using 4way. It may not", algo_names[opt_algo] );
  applog( LOG_WARNING,"work or may be slower. Please report your results.");
}

void algo_not_implemented()
{
  applog(LOG_ERR,"Algo %s has not been Implemented.",algo_names[opt_algo]);
}

// default null functions
// deprecated, use generic as default
int null_scanhash()
{
   applog(LOG_WARNING,"SWERR: undefined scanhash function in algo_gate");
   return 0;
}
void mm128_bswap32_80(void* d, void* s)
{
    ((uint32_t*)d)[0] = bswap_32(((uint32_t*)s)[0]);
    ((uint32_t*)d)[1] = bswap_32(((uint32_t*)s)[1]);
    ((uint32_t*)d)[2] = bswap_32(((uint32_t*)s)[2]);
    ((uint32_t*)d)[3] = bswap_32(((uint32_t*)s)[3]);
    ((uint32_t*)d)[4] = bswap_32(((uint32_t*)s)[4]);
    ((uint32_t*)d)[5] = bswap_32(((uint32_t*)s)[5]);
    ((uint32_t*)d)[6] = bswap_32(((uint32_t*)s)[6]);
    ((uint32_t*)d)[7] = bswap_32(((uint32_t*)s)[7]);
    ((uint32_t*)d)[8] = bswap_32(((uint32_t*)s)[8]);
    ((uint32_t*)d)[9] = bswap_32(((uint32_t*)s)[9]);
    ((uint32_t*)d)[10] = bswap_32(((uint32_t*)s)[10]);
    ((uint32_t*)d)[11] = bswap_32(((uint32_t*)s)[11]);
    ((uint32_t*)d)[12] = bswap_32(((uint32_t*)s)[12]);
    ((uint32_t*)d)[13] = bswap_32(((uint32_t*)s)[13]);
    ((uint32_t*)d)[14] = bswap_32(((uint32_t*)s)[14]);
    ((uint32_t*)d)[15] = bswap_32(((uint32_t*)s)[15]);
    ((uint32_t*)d)[16] = bswap_32(((uint32_t*)s)[16]);
    ((uint32_t*)d)[17] = bswap_32(((uint32_t*)s)[17]);
    ((uint32_t*)d)[18] = bswap_32(((uint32_t*)s)[18]);
    ((uint32_t*)d)[19] = bswap_32(((uint32_t*)s)[19]);
}
// Default generic scanhash can be used in many cases. Not to be used when
// prehashing can be done or when byte swapping the data can be avoided.
int scanhash_generic( struct work *work, uint32_t max_nonce,
                      uint64_t *hashes_done, struct thr_info *mythr )
{
   uint32_t edata[20] /*__attribute__((aligned(64)))*/;
   uint32_t hash[8] /*__attribute__((aligned(64)))*/;
   uint32_t *pdata = work->data;
   uint32_t *ptarget = work->target;
   const uint32_t first_nonce = pdata[19];
   const uint32_t last_nonce = max_nonce - 1;
   uint32_t n = first_nonce;
   const int thr_id = mythr->id;
   const BOOL bench = opt_benchmark;

   mm128_bswap32_80( edata, pdata );
   do
   {
      edata[19] = n;
      if ( likely( algo_gate.hash( hash, edata, thr_id ) ) )
      if ( unlikely( valid_hash( hash, ptarget ) && !bench ) )
      {
         pdata[19] = bswap_32( n );
         submit_solution( work, hash, mythr );
      }
      n++;
   } while ( n < last_nonce && !work_restart[thr_id].restart );
   *hashes_done = n - first_nonce;
   pdata[19] = n;
   return 0;
}

#if defined(__AVX2__)

//int scanhash_4way_64_64( struct work *work, uint32_t max_nonce,
//                      uint64_t *hashes_done, struct thr_info *mythr )

//int scanhash_4way_64_640( struct work *work, uint32_t max_nonce,
//                      uint64_t *hashes_done, struct thr_info *mythr )

int scanhash_4way_64in_32out( struct work *work, uint32_t max_nonce,
                      uint64_t *hashes_done, struct thr_info *mythr )
{
   uint32_t hash32[8*4] __attribute__ ((aligned (64)));
   uint32_t vdata[20*4] __attribute__ ((aligned (64)));
   uint32_t lane_hash[8] __attribute__ ((aligned (64)));
   uint32_t *hash32_d7 = &(hash32[ 7*4 ]);
   uint32_t *pdata = work->data;
   const uint32_t *ptarget = work->target;
   const uint32_t first_nonce = pdata[19];
   const uint32_t last_nonce = max_nonce - 4;
   __m256i  *noncev = (__m256i*)vdata + 9;
   uint32_t n = first_nonce;
   const int thr_id = mythr->id;
   const uint32_t targ32_d7 = ptarget[7];
   const BOOL bench = opt_benchmark;

   mm256_bswap32_intrlv80_4x64( vdata, pdata );
   // overwrite byte swapped nonce with original byte order for proper
   // incrementing. The nonce only needs to byte swapped if it is to be
   // sumbitted.
   *noncev = mm256_intrlv_blend_32(
                   _mm256_set_epi32( n+3, 0, n+2, 0, n+1, 0, n, 0 ), *noncev );
   do
   {
      if ( likely( algo_gate.hash( hash32, vdata, thr_id ) ) )
      for ( int lane = 0; lane < 4; lane++ )
      if ( unlikely( hash32_d7[ lane ] <= targ32_d7 && !bench ) )
      {
         extr_lane_4x32( lane_hash, hash32, lane, 256 );
         if ( valid_hash( lane_hash, ptarget ) )
         {
            pdata[19] = bswap_32( n + lane );
            submit_solution( work, lane_hash, mythr );
         }
      }
      *noncev = _mm256_add_epi32( *noncev,
                                  m256_const1_64( 0x0000000400000000 ) );
      n += 4;
   } while ( likely( ( n <= last_nonce ) && !work_restart[thr_id].restart ) );
   pdata[19] = n;
   *hashes_done = n - first_nonce;
   return 0;
}

//int scanhash_8way_32_32( struct work *work, uint32_t max_nonce,
//                      uint64_t *hashes_done, struct thr_info *mythr )

#endif

#if defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512DQ__) && defined(__AVX512BW__)

//int scanhash_8way_64_64( struct work *work, uint32_t max_nonce,
//                      uint64_t *hashes_done, struct thr_info *mythr )

//int scanhash_8way_64_640( struct work *work, uint32_t max_nonce,
//                      uint64_t *hashes_done, struct thr_info *mythr )

int scanhash_8way_64in_32out( struct work *work, uint32_t max_nonce,
                      uint64_t *hashes_done, struct thr_info *mythr )
{
   uint32_t hash32[8*8] __attribute__ ((aligned (128)));
   uint32_t vdata[20*8] __attribute__ ((aligned (64)));
   uint32_t lane_hash[8] __attribute__ ((aligned (64)));
   uint32_t *hash32_d7 = &(hash32[7*8]);
   uint32_t *pdata = work->data;
   const uint32_t *ptarget = work->target;
   const uint32_t first_nonce = pdata[19];
   const uint32_t last_nonce = max_nonce - 8;
   __m512i  *noncev = (__m512i*)vdata + 9;
   uint32_t n = first_nonce;
   const int thr_id = mythr->id;
   const uint32_t targ32_d7 = ptarget[7];
   const BOOL bench = opt_benchmark;

   mm512_bswap32_intrlv80_8x64( vdata, pdata );
   *noncev = mm512_intrlv_blend_32(
              _mm512_set_epi32( n+7, 0, n+6, 0, n+5, 0, n+4, 0,
                                n+3, 0, n+2, 0, n+1, 0, n,   0 ), *noncev );
   do
   {
      if ( likely( algo_gate.hash( hash32, vdata, thr_id ) ) )
      for ( int lane = 0; lane < 8; lane++ )
      if ( unlikely( ( hash32_d7[ lane ] <= targ32_d7 ) && !bench ) )
      {
         extr_lane_8x32( lane_hash, hash32, lane, 256 );
         if ( likely( valid_hash( lane_hash, ptarget ) ) )
         {
            pdata[19] = bswap_32( n + lane );
            submit_solution( work, lane_hash, mythr );
         }
      }
      *noncev = _mm512_add_epi32( *noncev,
                                  m512_const1_64( 0x0000000800000000 ) );
      n += 8;
   } while ( likely( ( n < last_nonce ) && !work_restart[thr_id].restart ) );
   pdata[19] = n;
   *hashes_done = n - first_nonce;
   return 0;
}

//int scanhash_16way_32_32( struct work *work, uint32_t max_nonce,
//                      uint64_t *hashes_done, struct thr_info *mythr )

#endif



int null_hash()
{
   applog(LOG_WARNING,"SWERR: null_hash unsafe null function");
   return 0;
};

void init_algo_gate( algo_gate_t* gate )
{
   gate->miner_thread_init       = (void*)&return_true;
   gate->scanhash                = (void*)&scanhash_generic;
   gate->hash                    = (void*)&null_hash;
   gate->get_new_work            = (void*)&std_get_new_work;
   gate->work_decode             = (void*)&std_le_work_decode;
   gate->decode_extra_data       = (void*)&do_nothing;
   gate->gen_merkle_root         = (void*)&sha256d_gen_merkle_root;
   gate->build_stratum_request   = (void*)&std_le_build_stratum_request;
   gate->malloc_txs_request      = (void*)&std_malloc_txs_request;
   gate->submit_getwork_result   = (void*)&std_le_submit_getwork_result;
   gate->build_block_header      = (void*)&std_build_block_header;
   gate->build_extraheader       = (void*)&std_build_extraheader;
   gate->set_work_data_endian    = (void*)&do_nothing;
   gate->resync_threads          = (void*)&do_nothing;
   gate->do_this_thread          = (void*)&return_true;
   gate->longpoll_rpc_call       = (void*)&std_longpoll_rpc_call;
   gate->get_work_data_size      = (void*)&std_get_work_data_size;
   gate->optimizations           = EMPTY_SET;
   gate->ntime_index             = STD_NTIME_INDEX;
   gate->nbits_index             = STD_NBITS_INDEX;
   gate->nonce_index             = STD_NONCE_INDEX;
   gate->work_cmp_size           = STD_WORK_CMP_SIZE;
}

// Ignore warnings for not yet defined register functions
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

// Called once by main
BOOL register_algo_gate( int algo, algo_gate_t *gate )
{
  BOOL rc = FALSE;

  if ( NULL == gate )
  {
    applog(LOG_ERR,"FAIL: algo_gate registration failed, NULL gate\n");
    return FALSE;
  }

  init_algo_gate( gate );

  switch ( algo )
  {
    case ALGO_QUBIT:        rc = register_qubit_algo         ( gate ); break;
    case ALGO_SCRYPT:       rc = register_scrypt_algo        ( gate ); break;
    case ALGO_SHA256D:      rc = register_sha256d_algo       ( gate ); break;
    case ALGO_SHA256DT:     rc = register_sha256dt_algo      ( gate ); break;
    case ALGO_SHA256Q:      rc = register_sha256q_algo       ( gate ); break;
    case ALGO_SHA256T:      rc = register_sha256t_algo       ( gate ); break;
    case ALGO_SKEIN:        rc = register_skein_algo         ( gate ); break;
    case ALGO_SKEIN2:       rc = register_skein2_algo        ( gate ); break;
    case ALGO_ODO:          rc = register_odo_algo           ( gate ); break;
   default:
      applog(LOG_ERR,"BUG: unregistered algorithm %s.\n", algo_names[opt_algo] );
      return FALSE;
  } // switch

  if ( !rc )
  {
    applog(LOG_ERR, "FAIL: %s algorithm failed to initialize\n", algo_names[opt_algo] );
    return FALSE;
  }
  return TRUE;
}

// restore warnings
//#pragma GCC diagnostic pop

void exec_hash_function( int algo, void *output, const void *pdata )
{
  algo_gate_t gate;   
  gate.hash = (void*)&null_hash;
  register_algo_gate( algo, &gate );
  gate.hash( output, pdata, 0 );  
}

#define PROPER (1)
#define ALIAS  (0)

// The only difference between the alias and the proper algo name is the
// proper name is the one that is defined in ALGO_NAMES. There may be
// multiple aliases that map to the same proper name.
// New aliases can be added anywhere in the array as long as NULL is last.
// Alphabetic order of alias is recommended.
const char* const algo_alias_map[][2] =
{
//   alias                proper
  { "1",             "qubit"     },
  { "2",             "scrypt"    },
  { "3",             "sha256d"   },
  { "sha256",        "sha256d"   },
  { "4",             "skein"     },
  { "5",             "odo"       },
  { "odocrypt",      "odo"       },
  { NULL,            NULL        }
};

// if arg is a valid alias for a known algo it is updated with the proper
// name. No validation of the algo or alias is done, It is the responsinility
// of the calling function to validate the algo after return.
void get_algo_alias( char** algo_or_alias )
{
  int i;
  for ( i=0; algo_alias_map[i][ALIAS]; i++ )
    if ( !strcasecmp( *algo_or_alias, algo_alias_map[i][ ALIAS ] ) )
    {
      // found valid alias, return proper name
      *algo_or_alias = (char*)( algo_alias_map[i][ PROPER ] );
      return;
    }
}

#undef ALIAS
#undef PROPER

