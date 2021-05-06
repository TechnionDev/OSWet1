var=$(date +"%FORMAT_STRING")
now=$(date +"%Y%m%d_%H%M")
printf "%s\n" $now


zip submission_$now.zip Commands.cpp Jobs.cpp SmallShell.cpp Utils.cpp main.cpp signals.cpp \
    Commands.h Constants.h Exceptions.h Jobs.h SmallShell.h Utils.h signals.h \
    submitters.txt Makefile

