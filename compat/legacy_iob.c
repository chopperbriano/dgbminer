/*
 * Legacy CRT compatibility shim.
 *
 * The bundled libcurl_a.lib was built against an older MSVC runtime (pre-UCRT)
 * that referenced __iob_func() returning a pointer to a 3-element FILE array
 * containing stdin/stdout/stderr. Modern UCRT replaced this with
 * __acrt_iob_func(int index). This shim provides the legacy entry point so
 * the old libcurl can link against modern CRT.
 *
 * See https://stackoverflow.com/q/30412951 for background.
 */

#include <stdio.h>

#ifdef _MSC_VER

FILE _iob[] = { {0}, {0}, {0} };

__declspec(dllexport) FILE* __cdecl __iob_func(void)
{
    _iob[0] = *stdin;
    _iob[1] = *stdout;
    _iob[2] = *stderr;
    return _iob;
}

#endif /* _MSC_VER */
