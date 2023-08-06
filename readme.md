simple java virtual machine, interpreter

## compilation
requires:
* `clang`, `javac`, `lld`
* projects in a parent directory: `core`, `posix-wrapper`, `class-file`, `encoding`, `print`

scripts:
* `compile_jvm.sh` - compiles jvm
* `compile_liv.sh` - compiles std library
* `compile.sh` - both