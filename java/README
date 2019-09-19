# Java skeleton for duck hunt dd2380

# Compile
javac *.java

# Run
# The agent can be run in two different modes:
# 2. Server - act as the judge by sending predefined observations one at a time
#    and asking the client to respond 
# 3. Client - get observations from standard input and output actions to
#    standard output (this is the default mode)

# The server and client can be run in separate terminals and communicate
# through pipes. Create the pipes first. (we recommend cygwin for windows users)
mkfifo player2server server2player

# Terminal 1:
java Main verbose server < player2server > server2player

# Terminal 2:
java Main verbose > player2server < server2player

# Or you may run both instances in the same terminal
java Main server < player2server | java Main verbose > player2server

# You can test a different environment like this
java Main server load ParadiseEmissions.in < player2server | java Main verbose > player2server
