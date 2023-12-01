#!/bin/bash

number_of_clients=$1
number_of_iterations=$2
sleep_time=$3

counter=1
tstart=$(date +%s%N)
echo "time start = $tstart"
while [ $counter -le $number_of_clients ]
do
    ./load_generator_client localhost 8080 pass.c $number_of_iterations $sleep_time > pass$counter.txt &
    counter=$((counter+1))
done
wait
tend=$(date +%s%N)
echo "time end = $tend"
tdiff=$(bc -l <<< $tend-$tstart)
echo "time difference = $tdiff"
tdiff=$(bc -l <<< $tdiff/1000000000)
echo "time difference in ns = $tdiff"

#THROUGHPUT CALCULATION1
throughput1=$(bc -l <<< $1*$2/$tdiff)
echo "Throughput = $throughput1"

#THROUGHPUT CALCULATION
total_throughput=0
counter=1
while [ $counter -le $number_of_clients ]
do
    throughput=$(grep Throughput pass$counter.txt | cut -f 2 -d =)
    total_throughput=$(bc -l <<< $total_throughput+$throughput)
    counter=$((counter+1))
done
echo "Total Throughput=$total_throughput"


#AVERAGE_RESPONSE_TIME CALCULATION
counter=1
numerator=0
denominator=0
while [ $counter -le $number_of_clients ]
do
    r=$(grep Average pass$counter.txt | cut -f 2 -d =)
    n=$(grep Successful pass$counter.txt | cut -f 2 -d =)
    product=$(bc -l <<< $r*$n)
    numerator=$(bc -l <<< $numerator+$product)
    denominator=$(bc -l <<< $denominator+$n)
    counter=$((counter+1))
done
echo "$numerator"
echo "$denominator"
avg_resp_time=$(bc -l <<< $numerator/$denominator)
echo "Average Response Time in microseconds=$avg_resp_time"
echo "Successful Response=$denominator"
