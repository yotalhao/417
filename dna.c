/*
 * dna_extension.c
 *
 * PostgreSQL extension for the DNA type, storing each nucleotide using 2 bits.
 *
 * Author: Youssef Talhaoui 
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "postgres.h"
#include "fmgr.h"
#include "kmer.c"
#include "qkmer.c"
#include "libpq/pqformat.h"
#include "utils/fmgrprotos.h"
#include "utils/elog.h"
#include "utils/builtins.h"
#include "utils/varlena.h"
#include "varatt.h"
PG_MODULE_MAGIC;

/* Definition of the DNA type */
typedef struct {
    int32 vl_len_; /* Total length of the object in bytes (varlena header) */
    int32 length;  /* Number of nucleotides */
    uint8 data[FLEXIBLE_ARRAY_MEMBER]; /* Compressed nucleotide data */
} DNA;

/* Macros for accessing the DNA type */
#define PG_GETARG_DNA_P(n)      ((DNA *) PG_GETARG_VARLENA_P(n))
#define PG_RETURN_DNA_P(x)      PG_RETURN_POINTER(x)

/* Function declarations */
Datum dna_in(PG_FUNCTION_ARGS);
Datum dna_out(PG_FUNCTION_ARGS);
Datum dna_recv(PG_FUNCTION_ARGS);
Datum dna_send(PG_FUNCTION_ARGS);
Datum dna_constructor(PG_FUNCTION_ARGS);
Datum dna_cast_from_text(PG_FUNCTION_ARGS);
Datum dna_cast_to_text(PG_FUNCTION_ARGS);

/* Function to compress nucleotides into bytes */
static void
compress_nucleotides(const char *str, size_t len, uint8 *data)
{
    size_t i;
    memset(data, 0, (len + 3) / 4); // Ensure data is initialized to zero

    for (i = 0; i < len; i++) {
        char c;
        uint8_t code;
        size_t byte_pos;
        size_t bit_pos;

        c = toupper((unsigned char) str[i]);

        switch (c) {
            case 'A': code = 0b00; break;
            case 'C': code = 0b01; break;
            case 'G': code = 0b10; break;
            case 'T': code = 0b11; break;
            default:
                ereport(ERROR,
                        (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                         errmsg("Invalid nucleotide '%c' at position %zu", c, i + 1)));
                return;
        }

        byte_pos = i / 4;
        bit_pos = (3 - (i % 4)) * 2;
        data[byte_pos] |= code << bit_pos;
    }
}

/* Function to convert byte data to nucleotide string */
static char *
dna_to_string(const DNA *dna)
{
    size_t len = dna->length;
    char *str = (char *) palloc(len + 1);
    size_t i;
    const uint8 *data_ptr = dna->data;

    for (i = 0; i < len; i++) {
        size_t byte_pos;
        size_t bit_pos;
        uint8_t code;
        char nucleotide;

        byte_pos = i / 4;
        bit_pos = (3 - (i % 4)) * 2;
        code = (data_ptr[byte_pos] >> bit_pos) & 0b11;

        switch (code) {
            case 0b00: nucleotide = 'A'; break;
            case 0b01: nucleotide = 'C'; break;
            case 0b10: nucleotide = 'G'; break;
            case 0b11: nucleotide = 'T'; break;
            default:
                ereport(ERROR,
                        (errmsg("Invalid nucleotide code '%d' at position %zu", code, i + 1)));
                return NULL;
        }
        str[i] = nucleotide;
    }
    str[len] = '\0';
    return str;
}

/* Constructor function for the DNA type */
PG_FUNCTION_INFO_V1(dna_constructor);
Datum
dna_constructor(PG_FUNCTION_ARGS)
{
    text *input_text = PG_GETARG_TEXT_P(0);
    char *str = text_to_cstring(input_text);
    size_t len = strlen(str);
    size_t data_size;
    size_t total_size;
    DNA *result;

    data_size = (len + 3) / 4;
    total_size = VARHDRSZ + sizeof(int32) + data_size * sizeof(uint8);

    result = (DNA *) palloc0(total_size);
    SET_VARSIZE(result, total_size);
    result->length = (int32) len;

    compress_nucleotides(str, len, result->data);

    PG_RETURN_POINTER(result);
}

/* Input function */
PG_FUNCTION_INFO_V1(dna_in);
Datum
dna_in(PG_FUNCTION_ARGS)
{
    char *str = PG_GETARG_CSTRING(0);
    size_t len = strlen(str);
    size_t data_size;
    size_t total_size;
    DNA *result;

    data_size = (len + 3) / 4;
    total_size = VARHDRSZ + sizeof(int32) + data_size * sizeof(uint8);

    result = (DNA *) palloc0(total_size);
    SET_VARSIZE(result, total_size);
    result->length = (int32) len;

    compress_nucleotides(str, len, result->data);

    PG_RETURN_POINTER(result);
}

/* Output function */
PG_FUNCTION_INFO_V1(dna_out);
Datum
dna_out(PG_FUNCTION_ARGS)
{
    DNA *dna = PG_GETARG_DNA_P(0);
    char *str;

    str = dna_to_string(dna);

    PG_RETURN_CSTRING(str);
}

/* Receive function */
PG_FUNCTION_INFO_V1(dna_recv);
Datum
dna_recv(PG_FUNCTION_ARGS)
{
    StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
    DNA *result;
    int32 len;
    size_t data_size;
    size_t total_size;

    len = pq_getmsgint(buf, 4);

    data_size = (len + 3) / 4;
    total_size = VARHDRSZ + sizeof(int32) + data_size * sizeof(uint8);

    result = (DNA *) palloc0(total_size);
    SET_VARSIZE(result, total_size);
    result->length = len;

    pq_copymsgbytes(buf, (char *) result->data, data_size);

    PG_RETURN_POINTER(result);
}

/* Send function */
PG_FUNCTION_INFO_V1(dna_send);
Datum
dna_send(PG_FUNCTION_ARGS)
{
    DNA *dna = PG_GETARG_DNA_P(0);
    StringInfoData buf;
    size_t data_size;

    pq_begintypsend(&buf);

    pq_sendint(&buf, dna->length, 4);

    data_size = (dna->length + 3) / 4;
    pq_sendbytes(&buf, (char *) dna->data, data_size);

    PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/* Cast function from text to dna */
PG_FUNCTION_INFO_V1(dna_cast_from_text);
Datum
dna_cast_from_text(PG_FUNCTION_ARGS)
{
    Datum result = dna_constructor(fcinfo);
    return result;
}

/* Cast function from dna to text */
PG_FUNCTION_INFO_V1(dna_cast_to_text);
Datum
dna_cast_to_text(PG_FUNCTION_ARGS)
{
    DNA *dna = PG_GETARG_DNA_P(0);
    char *str = dna_to_string(dna);
    text *result = cstring_to_text(str);

    PG_RETURN_TEXT_P(result);
}
