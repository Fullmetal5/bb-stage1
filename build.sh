# A *very* thrown together build script just so people can use this.
rm -f stage1.bin
cd credits
make clean
cd ../payload
rm credits.elf payload.bin
cd ../stage1/loader/
make clean
cd ..
make clean
cd ..


cd credits
make
cp credits.elf ../payload/
cd ../payload/
powerpc-eabi-strip credits.elf
./make_it.sh credits.elf
cd ../stage1/loader/
make
cd ..
make
cd ..
cat stage1/cleanup.bin stage1/loader/loader.bin payload/payload.bin > stage1.bin
