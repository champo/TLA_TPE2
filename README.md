ITBA_TLA_12_cfg
==================

utility to create recursive descent parsers for context free grammars

Instructions
============

To compile, just run `make`.
To run, `./genASDR input_file [output_file]`, where the `input_file` has the extension `gr`. If `output_file` is not specified, `ASDR.c` will be used.

Once that is done, do `gcc -o ASDR ASDR.c` to compile the analyzer. And `./ASDR word` to check whether `word` belongs to the grammar in `input_file`.
