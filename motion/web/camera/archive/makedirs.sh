#!/bin/sh

for camera in smederij kalverstal robot1 robot2 ; do
  for dag in zondag maandag dinsdag woensdag donderdag vrijdag zaterdag ; do
    for ampm in am pm ; do
        mkdir $camera/$dag/$ampm/extra
    done
  done
done
