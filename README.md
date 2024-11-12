# DNA Extension for PostgreSQL

## Prerequisites

Ensure that PostgreSQL is installed on your machine. You also need to have PostgreSQL development tools available (e.g., `pg_config` should be in your environment).

## Compilation of the Extension

To compile and install the extension, follow these steps:

1. Clean up any previous build files:
   ```bash
   make clean
   ```

2. Compile the source code:
   ```bash
   make
   ```

3. Install the extension into PostgreSQL:
   ```bash
   sudo make install
   ```

## Installing the Extension in PostgreSQL

After installing, you need to activate the extension in your PostgreSQL database. Connect to your database using `psql`:

```bash
psql -d <your_database_name>
```

Then, run the following command to create the extension:

```sql
CREATE EXTENSION dna;
```

## Running the Tests

To verify the functionality of the extension, execute the provided `test.sql` file:

```bash
psql -d <your_database_name> -f test.sql
```

This file contains tests for the `dna`, `kmer`, and `qkmer` data types. The results of the tests will be displayed in the console.

## File Structure

- `dna.c`: Source code for the `dna` type, which stores DNA sequences compressed using 2 bits per nucleotide.
- `kmer.c`: Source code for the `kmer` type, representing k-mers compressed with 2 bits per nucleotide, with a maximum length of 32.
- `qkmer.c`: Source code for the `qkmer` type, which supports IUPAC codes and compresses each nucleotide using 4 bits.
- `dna--1.0.sql`: SQL script to create the types and functions in PostgreSQL.
- `test.sql`: Test file containing examples of insertions and queries to verify the extension’s functionality.
- `Makefile`: File used for compilation and installation of the extension.

## Usage Examples

### Inserting DNA Sequences

```sql
INSERT INTO sequences (dna_seq) VALUES ('ACGTACGT');
```

### Inserting K-mers

```sql
INSERT INTO sequences (kmer_seq) VALUES ('ACGT');
```

### Inserting Q-kmers

```sql
INSERT INTO sequences (qkmer_seq) VALUES ('RYSWKM');
```

### Converting Types to Text

```sql
SELECT dna_seq::text, kmer_seq::text, qkmer_seq::text FROM sequences;
```

## Cleanup

To remove the generated build files, use:

```bash
make clean
```

To uninstall the extension from PostgreSQL:

```sql
DROP EXTENSION IF EXISTS dna CASCADE;
```

## Notes

- The maximum length for `dna` and `kmer` types is 32 nucleotides.
- The `qkmer` type supports IUPAC codes for ambiguous nucleotides.

---

You can save this content as `README.md` for your project.
