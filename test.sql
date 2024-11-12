-- ===========================================
-- Tests for PostgreSQL extensions "dna" and "kmer"
-- ===========================================

-- 1. Drop tables and extensions if they exist
DROP TABLE IF EXISTS sequences;
DROP EXTENSION IF EXISTS dna CASCADE;

-- 2. Create extensions
CREATE EXTENSION dna;

-- 3. Create a table with columns of type `dna`, `kmer`, and `qkmer`
CREATE TABLE sequences (
    id serial PRIMARY KEY,
    dna_seq dna,
    kmer_seq kmer,
    qkmer_seq qkmer
);
-- ===========================================
-- Tests for the PostgreSQL extension "qkmer"
-- ===========================================

-- 4. Insert valid QKmer sequences with different combinations of IUPAC codes
INSERT INTO sequences (qkmer_seq) VALUES ('ACGT');
INSERT INTO sequences (qkmer_seq) VALUES ('RYSWKM'); -- IUPAC codes for ambiguous nucleotides
INSERT INTO sequences (qkmer_seq) VALUES ('BDHVN'); -- Additional IUPAC codes for ambiguity
INSERT INTO sequences (qkmer_seq) VALUES ('AAACCCGGGTTT'); -- Sequence shorter than 16 nucleotides
INSERT INTO sequences (qkmer_seq) VALUES ('AATTGGCCAAGGTTCC'); -- Sequence of exactly 16 nucleotides

-- 5. Select data for verification
SELECT id, qkmer_seq::text AS qkmer_sequence FROM sequences;

-- 6. Tests for `qkmer_constructor`
-- Create qkmers using the `qkmer_constructor` function
INSERT INTO sequences (qkmer_seq) VALUES (qkmer_constructor('ACGTACGTACGTACGT'));
INSERT INTO sequences (qkmer_seq) VALUES (qkmer_constructor('ACGTNRYM'));

-- Verify qkmers inserted via `qkmer_constructor`
SELECT id, qkmer_seq::text AS constructed_qkmer_sequence FROM sequences;

-- 7. Tests for `qkmer_in` and `qkmer_out`
-- Implicit conversion from `text` to `qkmer`
INSERT INTO sequences (qkmer_seq) VALUES ('TTTTGGGGCCCCAAAA');
SELECT id, qkmer_seq::text AS qkmer_out_test FROM sequences WHERE qkmer_seq = 'TTTTGGGGCCCCAAAA';

-- Explicit conversion from `qkmer` to `text`
SELECT id, qkmer_seq::text AS qkmer_to_text FROM sequences WHERE id = 6;

-- 8. Tests for `qkmer_send` and `qkmer_recv`
-- Use `send` and `recv` functions to verify binary transmission

-- Simulate sending and receiving (use a binary application for verification)
SELECT qkmer_send(qkmer_seq) AS qkmer_binary FROM sequences WHERE id = 7;
SELECT qkmer_recv(qkmer_send(qkmer_seq))::text AS qkmer_round_trip FROM sequences WHERE id = 7;

-- 9. Attempt to insert invalid QKmer sequences
-- The lines below should raise errors.
INSERT INTO sequences (qkmer_seq) VALUES ('ACGTXYZ'); -- Invalid characters for qkmer
INSERT INTO sequences (qkmer_seq) VALUES (''); -- Empty sequence (should raise an error)
INSERT INTO sequences (qkmer_seq) VALUES (repeat('A', 33)); -- Length exceeding 32

-- 10. Select data after updates and tests
SELECT * FROM sequences;

-- 11. Reverse conversion
-- Convert a QKmer sequence to `text` and then back to `qkmer`
SELECT id, (qkmer_seq::text)::qkmer AS qkmer_converted_back FROM sequences WHERE id = 2;

-- 12. Display final results for QKmers
SELECT id, qkmer_seq::text AS qkmer_final_sequence FROM sequences;

-- 4. Insert valid DNA sequences
INSERT INTO sequences (dna_seq) VALUES ('AAAAAAAAAAAA');
INSERT INTO sequences (dna_seq) VALUES ('ACGT');
INSERT INTO sequences (dna_seq) VALUES ('GGGGCCCCAAAA');
INSERT INTO sequences (dna_seq) VALUES ('TGCATGCA');
INSERT INTO sequences (dna_seq) VALUES ('acgtacgt'); -- Test uppercase conversion

-- 5. Select data for verification
SELECT id, dna_seq::text AS dna_sequence FROM sequences;

-- 6. Insert valid k-mers (max length 32)
INSERT INTO sequences (kmer_seq) VALUES ('ACGT');
INSERT INTO sequences (kmer_seq) VALUES ('GATTACA');
INSERT INTO sequences (kmer_seq) VALUES ('GGGGCCCCAAAATTTT');
INSERT INTO sequences (kmer_seq) VALUES ('TTTTGGGGCCCCAAAA');
INSERT INTO sequences (kmer_seq) VALUES ('ccggtt'); -- Test uppercase conversion

-- 7. Select k-mers for verification
SELECT id, kmer_seq::text AS kmer_sequence FROM sequences;

-- 8. Attempt to insert invalid DNA and k-mer sequences
-- The lines below should raise errors.
INSERT INTO sequences (dna_seq) VALUES ('ACGTXYZ'); -- Invalid characters
INSERT INTO sequences (kmer_seq) VALUES ('ACGTXYZ'); -- Invalid characters
INSERT INTO sequences (kmer_seq) VALUES (''); -- Empty sequence (should raise an error)
INSERT INTO sequences (kmer_seq) VALUES (repeat('A', 33)); -- Length exceeding 32

-- 9. Tests for casting between `text`, `dna`, and `kmer`

-- Implicit cast from `text` to `dna` and `kmer`
INSERT INTO sequences (dna_seq, kmer_seq) VALUES ('TTTTGGGGCCCCAAAA', 'ACGTCGTA');

-- Explicit cast from `text` to `dna` and `kmer`
INSERT INTO sequences (dna_seq, kmer_seq) VALUES (dna('CCCCGGGGTTTTAAAA'), kmer('TGCATGCA'));

-- Explicit cast from `dna` and `kmer` to `text`
SELECT id, dna_seq::text AS dna_text, kmer_seq::text AS kmer_text FROM sequences;

-- Selection after update
SELECT id, dna_seq::text AS updated_dna, kmer_seq::text AS updated_kmer FROM sequences;

-- Deleting a row
DELETE FROM sequences WHERE id = 3;

-- Selection after deletion
SELECT id, dna_seq::text AS remaining_dna, kmer_seq::text AS remaining_kmer FROM sequences;

-- 11. Test for empty sequences
-- Insert an empty DNA sequence (if allowed)
INSERT INTO sequences (dna_seq) VALUES (''); -- Should be accepted if allowed

-- 12. Reverse conversion

-- Convert a DNA sequence to `text` and then back to `dna`
SELECT id, (dna_seq::text)::dna AS dna_converted_back FROM sequences WHERE id = 12;

-- Convert a k-mer to `text` and then back to `kmer`
SELECT id, (kmer_seq::text)::kmer AS kmer_converted_back FROM sequences WHERE id = 14;

-- 13. Display final results
-- Show all sequences to verify data integrity
SELECT * FROM sequences;
