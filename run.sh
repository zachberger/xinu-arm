QEMU_INSTALL=./qemu-rpi-install/bin
GCC_INSTALL=./cross-tools/local-prefix/bin
COMPILE_DIR=./xinu-arm/compile

if [ $# = 0 ] 
then
    $QEMU_INSTALL/qemu-system-arm -M raspi -m 128M -nographic -cpu arm1176 -kernel $COMPILE_DIR/xinu.bin
elif [ $# = 1 ] && [ "$1" = "graphic" ]
then
    $QEMU_INSTALL/qemu-system-arm -M raspi -m 128M -cpu arm1176 -kernel $COMPILE_DIR/xinu.bin -serial stdio
elif [ $# = 1 ] && [ "$1" = "gdb" ]
then
    $GCC_INSTALL/arm-none-eabi-gdb $COMPILE_DIR/xinu.elf
elif [ $# = 2 ] && [ "$1" = "debug" ]
then
    $QEMU_INSTALL/qemu-system-arm -M raspi -m 128M -nographic -cpu arm1176 -kernel $COMPILE_DIR/xinu.bin -gdb tcp::"$2"
elif [ $# = 2 ] && [ "$1" = "graphic-debug" ]
then
    $QEMU_INSTALL/qemu-system-arm -M raspi -m 128M -cpu arm1176 -kernel $COMPILE_DIR/xinu.bin -serial stdio -gdb tcp::"$2"
elif [ "$1" = "make" ]
then
    pushd $COMPILE_DIR
    if [ $# = 2 ] && [ "$2" = "debug" ]
    then
        make debug
    elif [ $# = 2 ] && [ "$2" = "clean" ]
    then
        make clean
    elif [ $# = 1 ]
    then
        make
    else
        echo "bad arguments"
    fi
    popd
else
    echo "bad arguments"
fi
