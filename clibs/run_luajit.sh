export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../depend/luajit/lib
ulimit -HSn 100000
ulimit -c unlimited
../depend/luajit/bin/luajit-2.1.0-beta3
