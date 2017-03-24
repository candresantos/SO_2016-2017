#!/bin/sh
#Grupo 35
for i in $*
do
	OUTPUT=testes/out/$(echo $i | awk -F'/' '{print $NF}')
	./bin/sostore $i $OUTPUT"_mine"
	./prof $i $OUTPUT
done

for i in $*
do
	OUTPUT=testes/out/$(echo $i | awk -F'/' '{print $NF}')
	sort $OUTPUT > $OUTPUT".temp"
	sort $OUTPUT"_mine" > $OUTPUT"_mine.temp"
	if(diff $OUTPUT".temp" $OUTPUT"_mine.temp" > /dev/null)
	then
		echo "IGUAIS"
	else
		echo "DIFERENTES"
	fi
	rm $OUTPUT".temp"
	rm $OUTPUT"_mine.temp"
done
