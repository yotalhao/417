/*
 * qkmer.c
 *
 * PostgreSQL extension for the QKmer type, storing each nucleotide using 4 bits with support for IUPAC codes.
 *
 */

#include "postgres.h"
#include "fmgr.h"
#include "libpq/pqformat.h"
#include "utils/builtins.h"

/* Definition of the QKmer type with length */
typedef struct {
    int16 length; // Length of the sequence
    uint64 high;  // For the first 16 nucleotides
    uint64 low;   // For the next 16 nucleotides
} QKmer;

/* Macros for accessing the QKmer type */
#define PG_GETARG_QKMER_P(n)    ((QKmer *) PG_GETARG_POINTER(n))
#define PG_RETURN_QKMER_P(x)    PG_RETURN_POINTER(x)

/* Function to convert a nucleotide to bits */
static inline uint8
nucleotide_to_bits(char nucleotide) {
    switch (toupper(nucleotide)) {
        case 'A': return 0b0001;
        case 'C': return 0b0010;
        case 'G': return 0b0100;
        case 'T': return 0b1000;
        case 'R': return 0b0101;
        case 'Y': return 0b1010;
        case 'S': return 0b0110;
        case 'W': return 0b1001;
        case 'K': return 0b1100;
        case 'M': return 0b0011;
        case 'B': return 0b1110;
        case 'D': return 0b1101;
        case 'H': return 0b1011;
        case 'V': return 0b0111;
        case 'N': return 0b1111;
        case 0b0000: return 'N'; // Added case for the 0-bit pattern
        default:
            ereport(ERROR, (errmsg("Invalid nucleotide in qkmer: %c", nucleotide)));
            return 0;
    }
}

/* Function to convert bits to a nucleotide */
static inline char
bits_to_nucleotide(uint8 bits) {
    switch (bits & 0b1111) {
        case 0b0001: return 'A';
        case 0b0010: return 'C';
        case 0b0100: return 'G';
        case 0b1000: return 'T';
        case 0b0101: return 'R';
        case 0b1010: return 'Y';
        case 0b0110: return 'S';
        case 0b1001: return 'W';
        case 0b1100: return 'K';
        case 0b0011: return 'M';
        case 0b1110: return 'B';
        case 0b1101: return 'D';
        case 0b1011: return 'H';
        case 0b0111: return 'V';
        case 0b1111: return 'N';
        default:
            ereport(ERROR, (errmsg("Invalid bit pattern in qkmer: %d", bits)));
            return 'N';
    }
}

/* Input function */
PG_FUNCTION_INFO_V1(qkmer_in);
Datum
qkmer_in(PG_FUNCTION_ARGS) {
    char *str = PG_GETARG_CSTRING(0);
    int len = strlen(str);
    QKmer *result;
    int i;
    uint8 bits;

    if (len <= 0 || len > 32) {
        ereport(ERROR, (errmsg("The length of qkmer must be between 1 and 32 nucleotides")));
    }

    result = (QKmer *) palloc(sizeof(QKmer));
    result->length = (int16) len;
    result->high = 0;
    result->low = 0;

    for (i = 0; i < len; i++) {
        bits = nucleotide_to_bits(str[i]);
        if (i < 16) {
            result->high <<= 4;
            result->high |= bits;
        } else {
            result->low <<= 4;
            result->low |= bits;
        }
    }

    /* Align bits to the most significant positions */
    if (len <= 16) {
        result->high <<= (4 * (16 - len));
    } else if (len < 32) {
        result->low <<= (4 * (32 - len));
    }

    PG_RETURN_POINTER(result);
}

/* Constructor function */
PG_FUNCTION_INFO_V1(qkmer_constructor);
Datum
qkmer_constructor(PG_FUNCTION_ARGS) {
    text *input_text = PG_GETARG_TEXT_P(0);
    char *str = text_to_cstring(input_text);
    int len = strlen(str);
    QKmer *result;
    int i;
    uint8 bits;

    if (len <= 0 || len > 32) {
        ereport(ERROR, (errmsg("The length of qkmer must be between 1 and 32 nucleotides")));
    }

    result = (QKmer *) palloc(sizeof(QKmer));
    result->length = (int16) len;
    result->high = 0;
    result->low = 0;

    for (i = 0; i < len; i++) {
        bits = nucleotide_to_bits(str[i]);
        if (i < 16) {
            result->high <<= 4;
            result->high |= bits;
        } else {
            result->low <<= 4;
            result->low |= bits;
        }
    }

    /* Align bits to the most significant positions */
    if (len <= 16) {
        result->high <<= (4 * (16 - len));
    } else if (len < 32) {
        result->low <<= (4 * (32 - len));
    }

    PG_RETURN_POINTER(result);
}

/* Output function */
PG_FUNCTION_INFO_V1(qkmer_out);
Datum
qkmer_out(PG_FUNCTION_ARGS) {
    QKmer *qkmer = PG_GETARG_QKMER_P(0);
    int len = qkmer->length;
    char *result = (char *) palloc(len + 1);
    int i;
    uint8 bits;

    for (i = 0; i < len; i++) {
        if (i < 16) {
            bits = (qkmer->high >> ((15 - i) * 4)) & 0xF;
        } else {
            bits = (qkmer->low >> ((31 - i) * 4)) & 0xF;
        }
        result[i] = bits_to_nucleotide(bits);
    }
    result[len] = '\0';

    PG_RETURN_CSTRING(result);
}

/* Send function */
PG_FUNCTION_INFO_V1(qkmer_send);
Datum
qkmer_send(PG_FUNCTION_ARGS) {
    QKmer *qkmer = PG_GETARG_QKMER_P(0);
    StringInfoData buf;

    pq_begintypsend(&buf);
    pq_sendint16(&buf, qkmer->length);
    pq_sendint64(&buf, qkmer->high);
    pq_sendint64(&buf, qkmer->low);

    PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/* Receive function */
PG_FUNCTION_INFO_V1(qkmer_recv);
Datum
qkmer_recv(PG_FUNCTION_ARGS) {
    StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
    QKmer *result = (QKmer *) palloc(sizeof(QKmer));

    result->length = pq_getmsgint(buf, 2);
    result->high = pq_getmsgint64(buf);
    result->low = pq_getmsgint64(buf);

    PG_RETURN_POINTER(result);
}

/* Cast function from text to qkmer */
PG_FUNCTION_INFO_V1(qkmer_cast_from_text);
Datum
qkmer_cast_from_text(PG_FUNCTION_ARGS) {
    text *txt = PG_GETARG_TEXT_P(0);
    char *str = text_to_cstring(txt);

    Datum result = DirectFunctionCall1(qkmer_in, CStringGetDatum(str));
    PG_RETURN_DATUM(result);
}

/* Cast function from qkmer to text */
PG_FUNCTION_INFO_V1(qkmer_cast_to_text);
Datum
qkmer_cast_to_text(PG_FUNCTION_ARGS) {
    QKmer *qkmer = PG_GETARG_QKMER_P(0);
    Datum result = DirectFunctionCall1(qkmer_out, PointerGetDatum(qkmer));

    PG_RETURN_TEXT_P(cstring_to_text(DatumGetCString(result)));
}
