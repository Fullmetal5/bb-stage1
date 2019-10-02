gcc -g zpipe.c -o zpipe -lz
gcc -g pack_payload.c -o pack_payload

./zpipe < $1 > boot.elf.zlib
truncate -s %4 boot.elf.zlib
./pack_payload PONY boot.elf.zlib payload.bin
rm -f boot.elf.zlib out.bin
