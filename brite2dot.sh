#!/bin/bash

britefile=$1
dotfile=`echo "${britefile}" | sed -n 's/brite/dot/p'`
echo "graph G{" > ${dotfile}
sed -n '/Nodes:/,/^$/p' ${britefile} | tail -n +2 | sed '/^$/d' | awk '{printf "%d;\n",$1}' >> ${dotfile}
sed -n '/Edges:/,/^$/p' ${britefile} | tail -n +2 | sed '/^$/d' | awk '{printf "%d->%d;\n",$2,$3}' >> ${dotfile}
echo "}" >> ${dotfile}
