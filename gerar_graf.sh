#!/bin/bash

let i=0
for doc in grafos_mix/*.txt; do
   echo "python graficos.py \"${doc}\" grafos_mix/"
done
