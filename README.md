# Risc-V 32-bit Assembler

Accepts a RISC-V assembly program as input and converts it into an assembled binary executable.

![Example Usage](example.png)


The assembler includes the header file `unistd.h`, and uses POSIX `getopt` for
parsing CLI arguments.

## Stages

1. Lexing - Break the input string of characters into lexical tokens
1. Parsing - Assemble the lexical tokens into statements of the grammar
    - It is valid to branch/jump to a label we have not yet parsed, so we cannot
      calculate the offset of any targets yet
1. First Pass - Reverse the list produced by the parser and calculate any label
   offsets
1. Second Pass - Emit the compiled binary instructions

## Debug Mode

Give the assembler the `-v` flag when invoking it and it will print extra
debugging information, such as the symbol table and the line number of each
generated instruction.

The intent is that a person will be reading this output, so it should be
pretty-printed.

## TODO

- Assembler Directives
- `la` may need to only be one instruction, right now it assumes two are necessary
- Testing
