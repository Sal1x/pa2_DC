# говно из методы которое нужно чтобы либа с временем заработала
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/path/to/pa2/dir";
### пустая строка
LD_PRELOAD=/full/path/to/libruntime.so ./pa2 –p 2 10 20

LD_PRELOAD="/mnt/c/Users/Daniil/iCloudDrive/ИТМО/3 kursen/The newest distributed/pa2_DC/pa2_DC/lib64/libruntime.so" ./a.out –p 2 10 20

чтобы скомплиить: clang -std=c99 -Wall -pedantic *.c -Llib64 -lruntime
потом вот это пишем где путь к ПАПКЕ с либой
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:\"/mnt/c/Users/Daniil/iCloudDrive/ИТМО/3 kursen/The newest distributed/pa2_DC/pa2_DC/lib64\""
потом запускаем

export LIBRARY_PATH="$LIBRARY_PATH:\"/Users/salix/Desktop/ITMO/PA2/pa2_DC/lib64\""
clang -std=c99 -Wall -pedantic *.c -Llib64 -lruntime -lstdc
