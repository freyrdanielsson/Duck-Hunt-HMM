# C++ skeleton for duck hunt dd2380

# Compile
g++ -std=c++11 *pp

# Run
# The agent can be run in two different modes:
# 2. Server - act as the judge by sending predefined observations one at a time
#    and asking the client to respond 
# 3. Client - get observations from standard input and output actions to
#    standard output (this is the default mode)

# The server and client can be run in separate terminals and communicate
# through pipes. Create the pipes first (we recommend Cygwin for Windows users).
mkfifo player2server server2player

# Terminal 1:
./Skeleton verbose server < player2server > server2player

# Terminal 2:
./Skeleton verbose > player2server < server2player

#Where "Skeleton" is the compiled skeleton program. 

# Or you may run both instances in the same terminal.
./Skeleton server < player2server | ./Skeleton verbose > player2server

# You can test a different environment like this (if you do not want to make changes inside the program).
./Skeleton server load ParadiseEmissions.in < player2server | java Main verbose > player2server
