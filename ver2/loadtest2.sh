#!/bin/bash

number_of_clients=$1
number_of_iterations=$2
sleep_time=$3

counter=1
tstart=$(date +%s%N)
echo "time start = $tstart"
while [ $counter -le $number_of_clients ]
do
    ./client localhost 8080 pass.c $number_of_iterations $sleep_time 1 > pass$counter.txt &
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
successful_responses=0
counter=1
while [ $counter -le $number_of_clients ]
do
    successful_response=$(grep Successful pass$counter.txt | cut -f 2 -d =)
    successful_responses=$(bc -l <<< $successful_responses+$successful_response)
    counter=$((counter+1))
done
echo "Successsful Response = $successful_responses"
throughput1=$(bc -l <<< $successful_responses/$tdiff)
echo "Throughput = $throughput1"

#Timed Out Responses
timed_out_responses=0
counter=1
while [ $counter -le $number_of_clients ]
do
    timed_out_response=$(grep Timed pass$counter.txt | cut -f 2 -d =)
    timed_out_responses=$(bc -l <<< $timed_out_responses+$timed_out_response)
    counter=$((counter+1))
done
echo "Timed Out Response = $timed_out_responses"

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
avg_resp_time=$(bc -l <<< $numerator/$denominator)
echo "Average Response Time in microseconds=$avg_resp_time"
echo "Successful Response=$denominator"

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




