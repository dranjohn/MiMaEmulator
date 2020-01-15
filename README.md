MiMa Emulator
=============

The MiMa (minimal machine) is a model of a computer designed by the Karlsruhe Institute of Technology to explain the inner working of computer hardware architecture.

This emulation features:

  1. Emulation of the MiMa
  2. Modifiable micro program with compiler
  3. Modifiable initial state of the MiMas memory with compiler
  4. A command line interface (WIP)
  5. A graphical user interface (TODO)

These commands currently work in the CLI:
  * exit - Exits the CLI
  * microprogram
    * microprogram compile  \<name> \<fileName> - compiles a microprogram
    * microprogram show \<name> \<lowerLimit> \<upperLimit> - prints a microprogram to the CLI
  * mima
    * mima compile \<name> \<fileName> \<microprogramName> - create a minimal machine with given name from a file containing program code and the given microprogram.
    * mima show \<name> - print the minimal machine to the CLI
    * mima emulate \<name> \<cycle|instruction|lifetime> - lets the minimal machine emulate a cycle/instruction/lifetime.