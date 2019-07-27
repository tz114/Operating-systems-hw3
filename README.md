# Operating-systems-hw3

Program to determine whether or not a given m x n board can solve the knights tour problem using pthreads.
The program simulates valid moves creating a child thread if multiple moves are detected for a given board configuration.
The program tracks deadends and joins duplicate configurations and outputs them when encountered as well as all deadend board configurations.

Knights tour problem:Can a knight move over all squares exactly once on a given chess board

Usage: ./hw3.out \<m\> \<n\>
