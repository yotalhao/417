/*
 * kmer.c
 *
 * PostgreSQL extension for the Kmer type, storing each nucleotide as 2 bits in a uint64_t.
 *
 * Author: Youssef Talhaoui
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "postgres.h"
#include "fmgr.h"
#include "libpq/pqformat.h"
#include "utils/elog.h"
#include "utils/builtins.h"

/* Definition of the Kmer type */
typedef struct {
    uint8 length;   /* Length of the kmer (max 32) */
    uint64 value;   /* Compressed binary representation of nucleotides */
} Kmer;

/* Macros to access the Kmer type */
#define PG_GETARG_KMER_P(n)     ((Kmer *) PG_GETARG_POINTER(n))
#define PG_RETURN_KMER_P(x)     PG_RETURN_POINTER(x)

/* Declaration of functions */
Datum kmer_in(PG_FUNCTION_ARGS);
Datum kmer_out(PG_FUNCTION_ARGS);
Datum kmer_recv(PG_FUNCTION_ARGS);
Datum kmer_send(PG_FUNCTION_ARGS);
Datum kmer_constructor(PG_FUNCTION_ARGS);
Datum kmer_cast_from_text(PG_FUNCTION_ARGS);
Datum kmer_cast_to_text(PG_FUNCTION_ARGS);

/* Function to compress a DNA sequence into a kmer */
static void
compress_kmer(const char *str, uint8 length, uint64 *value)
{
    uint64 result = 0;
    uint8 i;

    for (i = 0; i < length; i++) {
        char c = toupper((unsigned char) str[i]);
        uint64 code;

        switch (c) {
            case 'A': code = 0b00; break;
            case 'C': code = 0b01; break;
            case 'G': code = 0b10; break;
            case 'T': code = 0b11; break;
            default:
                ereport(ERROR,
                        (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                         errmsg("Invalid nucleotide: '%c'.", c)));
        }

        result = (result << 2) | code;
    }

    *value = result;
}

/* Function to convert a kmer to a string */
static char *
kmer_to_string(const Kmer *kmer)
{
    char *str = (char *) palloc(kmer->length + 1);
    uint8 i;

    for (i = 0; i < kmer->length; i++) {
        uint8 shift = (kmer->length - i - 1) * 2;
        uint8 nucleotide = (kmer->value >> shift) & 0b11;

        switch (nucleotide) {
            case 0b00: str[i] = 'A'; break;
            case 0b01: str[i] = 'C'; break;
            case 0b10: str[i] = 'G'; break;
            case 0b11: str[i] = 'T'; break;
        }
    }

    str[kmer->length] = '\0';
    return str;
}

/* Input function */
PG_FUNCTION_INFO_V1(kmer_in);
Datum
kmer_in(PG_FUNCTION_ARGS)
{
    char *str = PG_GETARG_CSTRING(0);
    size_t len = strlen(str);
    Kmer *result;

    if (len == 0 || len > 32) {
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                 errmsg("The length of the kmer must be between 1 and 32 nucleotides.")));
    }

    result = (Kmer *) palloc(sizeof(Kmer));
    result->length = (uint8) len;
    compress_kmer(str, result->length, &result->value);

    PG_RETURN_KMER_P(result);
}

/* Output function */
PG_FUNCTION_INFO_V1(kmer_out);
Datum
kmer_out(PG_FUNCTION_ARGS)
{
    Kmer *kmer = PG_GETARG_KMER_P(0);
    char *str = kmer_to_string(kmer);

    PG_RETURN_CSTRING(str);
}

/* Receive function */
PG_FUNCTION_INFO_V1(kmer_recv);
Datum
kmer_recv(PG_FUNCTION_ARGS)
{
    StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
    Kmer *result = (Kmer *) palloc(sizeof(Kmer));

    result->length = pq_getmsgbyte(buf);
    if (result->length == 0 || result->length > 32) {
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_BINARY_REPRESENTATION),
                 errmsg("Invalid length for the kmer.")));
    }

    result->value = pq_getmsgint64(buf);
    PG_RETURN_KMER_P(result);
}

/* Send function */
PG_FUNCTION_INFO_V1(kmer_send);
Datum
kmer_send(PG_FUNCTION_ARGS)
{
    Kmer *kmer = PG_GETARG_KMER_P(0);
    StringInfoData buf;

    pq_begintypsend(&buf);
    pq_sendbyte(&buf, kmer->length);
    pq_sendint64(&buf, kmer->value);

    PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/* Constructor function for the Kmer type */
PG_FUNCTION_INFO_V1(kmer_constructor);
Datum
kmer_constructor(PG_FUNCTION_ARGS)
{
    text *input_text = PG_GETARG_TEXT_P(0);
    char *str;
    size_t len;
    Kmer *result;

    /* Convert the text to a C string */
    str = text_to_cstring(input_text);
    len = strlen(str);

    /* Check the length of the kmer */
    if (len == 0 || len > 32) {
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                 errmsg("The length of the kmer must be between 1 and 32 nucleotides.")));
    }

    /* Allocate memory for the Kmer */
    result = (Kmer *) palloc(sizeof(Kmer));
    result->length = (uint8) len;

    /* Compress the sequence into a Kmer */
    compress_kmer(str, result->length, &result->value);

    PG_RETURN_KMER_P(result);
}

/* Cast function from text to kmer */
PG_FUNCTION_INFO_V1(kmer_cast_from_text);
Datum
kmer_cast_from_text(PG_FUNCTION_ARGS)
{
    Datum result = kmer_constructor(fcinfo);
    PG_RETURN_DATUM(result);
}

/* Cast function from kmer to text */
PG_FUNCTION_INFO_V1(kmer_cast_to_text);
Datum
kmer_cast_to_text(PG_FUNCTION_ARGS)
{
    Kmer *kmer = PG_GETARG_KMER_P(0);
    char *str = kmer_to_string(kmer);
    text *result = cstring_to_text(str);

    PG_RETURN_TEXT_P(result);
}
