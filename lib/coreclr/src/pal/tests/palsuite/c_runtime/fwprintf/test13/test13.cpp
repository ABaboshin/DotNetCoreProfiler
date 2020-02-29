// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

/*============================================================================
**
** Source:      test13.c
**
** Purpose:     Tests the (uppercase) hexadecimal specifier (%X).
**              This test is modeled after the sprintf series.
**
**
**==========================================================================*/

#include <palsuite.h>
#include "../fwprintf.h"

/* 
 * Depends on memcmp, strlen, fopen, fseek and fgets.
 */

int __cdecl main(int argc, char *argv[])
{
    int neg = -42;
    int pos = 0x1234AB;
    INT64 l = I64(0x1234567887654321);
    
    if (PAL_Initialize(argc, argv) != 0)
    {
        return(FAIL);
    }

    DoNumTest(convert("foo %X"), pos, "foo 1234AB");
    DoNumTest(convert("foo %lX"), pos, "foo 1234AB");
    DoNumTest(convert("foo %hX"), pos, "foo 34AB");
    DoNumTest(convert("foo %LX"), pos, "foo 1234AB");
    DoI64Test(convert("foo %I64X"), l, "0x1234567887654321",
        "foo 1234567887654321", "foo 0x1234567887654321");
    DoNumTest(convert("foo %7X"), pos, "foo  1234AB");
    DoNumTest(convert("foo %-7X"), pos, "foo 1234AB ");
    DoNumTest(convert("foo %.1X"), pos, "foo 1234AB");
    DoNumTest(convert("foo %.7X"), pos, "foo 01234AB");
    DoNumTest(convert("foo %07X"), pos, "foo 01234AB");
    DoNumTest(convert("foo %#X"), pos, "foo 0X1234AB");
    DoNumTest(convert("foo %+X"), pos, "foo 1234AB");
    DoNumTest(convert("foo % X"), pos, "foo 1234AB");
    DoNumTest(convert("foo %+X"), neg, "foo FFFFFFD6");
    DoNumTest(convert("foo % X"), neg, "foo FFFFFFD6");

    PAL_Terminate();
    return PASS;
}
