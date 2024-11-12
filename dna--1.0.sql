
-- Input/Output Functions
CREATE OR REPLACE FUNCTION dna_in(cstring)
  RETURNS dna
  AS 'MODULE_PATHNAME', 'dna_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION dna_out(dna)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'dna_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Receive/Send Functions
CREATE OR REPLACE FUNCTION dna_recv(internal)
  RETURNS dna
  AS 'MODULE_PATHNAME', 'dna_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION dna_send(dna)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'dna_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Type Definition
CREATE TYPE dna (
  internallength = variable,
  input          = dna_in,
  output         = dna_out,
  receive        = dna_recv,
  send           = dna_send,
  alignment      = int4
);
-- Constructor Function
CREATE OR REPLACE FUNCTION dna_constructor(text)
  RETURNS dna
  AS 'MODULE_PATHNAME', 'dna_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;


-- Casts
CREATE OR REPLACE FUNCTION dna(text)
  RETURNS dna
  AS 'MODULE_PATHNAME', 'dna_cast_from_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION text(dna)
  RETURNS text
  AS 'MODULE_PATHNAME', 'dna_cast_to_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (text AS dna) WITH FUNCTION dna(text) AS IMPLICIT;
CREATE CAST (dna AS text) WITH FUNCTION text(dna);
-----------------------------------------------------------------------

-- Input/Output Functions
CREATE OR REPLACE FUNCTION kmer_in(cstring)
  RETURNS kmer
  AS 'MODULE_PATHNAME', 'kmer_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kmer_out(kmer)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'kmer_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Receive/Send Functions
CREATE OR REPLACE FUNCTION kmer_recv(internal)
  RETURNS kmer
  AS 'MODULE_PATHNAME', 'kmer_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION kmer_send(kmer)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'kmer_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Type Definition
CREATE TYPE kmer (
  internallength = 9,
  input          = kmer_in,
  output         = kmer_out,
  receive        = kmer_recv,
  send           = kmer_send,
  alignment      = double
);

-- Casts
CREATE OR REPLACE FUNCTION kmer(text)
  RETURNS kmer
  AS 'MODULE_PATHNAME', 'kmer_cast_from_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION text(kmer)
  RETURNS text
  AS 'MODULE_PATHNAME', 'kmer_cast_to_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (text AS kmer) WITH FUNCTION kmer(text) AS IMPLICIT;
CREATE CAST (kmer AS text) WITH FUNCTION text(kmer);
-- Constructor Function
CREATE OR REPLACE FUNCTION kmer_constructor(text)
  RETURNS kmer
  AS 'MODULE_PATHNAME', 'kmer_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
---------------------------------------------------------------
-- Input/Output Functions
CREATE OR REPLACE FUNCTION qkmer_in(cstring)
  RETURNS qkmer
  AS 'MODULE_PATHNAME', 'qkmer_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION qkmer_out(qkmer)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'qkmer_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Receive/Send Functions
CREATE OR REPLACE FUNCTION qkmer_recv(internal)
  RETURNS qkmer
  AS 'MODULE_PATHNAME', 'qkmer_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION qkmer_send(qkmer)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'qkmer_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Type Definition
CREATE TYPE qkmer (
  internallength = 24,
  input          = qkmer_in,
  output         = qkmer_out,
  receive        = qkmer_recv,
  send           = qkmer_send,
  alignment      = double
);

-- Casts
CREATE OR REPLACE FUNCTION qkmer(text)
  RETURNS qkmer
  AS 'MODULE_PATHNAME', 'qkmer_cast_from_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION text(qkmer)
  RETURNS text
  AS 'MODULE_PATHNAME', 'qkmer_cast_to_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (text AS qkmer) WITH FUNCTION qkmer(text) AS IMPLICIT;
CREATE CAST (qkmer AS text) WITH FUNCTION text(qkmer);
-- Constructor Function
CREATE OR REPLACE FUNCTION qkmer_constructor(text)
  RETURNS qkmer
  AS 'MODULE_PATHNAME', 'qkmer_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;