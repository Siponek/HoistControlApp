#!/bin/bash
echo "Installer by Szymon Zinkowicz / Maciej Bogdalski"
echo "Instalation in progess..."
# make bin/ directory for executables
mkdir bin
# compile source files 
#  output executables
#   linking math library
gcc src/masterProcess.c -lm -o bin/masterProcess.exe
gcc src/userConsole.c -lm -o bin/userConsole.exe
gcc src/watchdog.c -lm -o bin/watchdog.exe
touch run.sh
# adding executable rights for script
chmod +x run.sh;
# creating script to run programs: run.sh
echo "#!/bin/bash" > run.sh
echo "konsole  -e \"./bin/masterProcess.exe; bash\"" >> run.sh
echo "konsole  -e \"./bin/userConsole.exe; bash\"" >> run.sh
echo "konsole  -e \"./bin/watchdog.exe; bash\"" >> run.sh

echo "gnome-terminal -- sh -c \"./bin/inspector; bash\"" >> run.sh
echo "echo return value: \$?" >> run.sh

# ?
echo Installation finished. Executable script created: run.sh
# ?