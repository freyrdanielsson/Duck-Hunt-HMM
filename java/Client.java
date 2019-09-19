import java.io.BufferedReader;
import java.io.PrintStream;
import java.util.StringTokenizer;

/**
 * Encapsulates client functionality (except agent intelligence)
 */
class Client {
    /**
     * Create a client with a player
     *
     * The client is connected to the server through streams
     */
    public Client(Player pPlayer, BufferedReader pInputStream,
            PrintStream pOutputStream) {
        mPlayer = pPlayer;
        mInputStream = pInputStream;
        mOutputStream = pOutputStream;
        mState = null;
    }

    /**
     * Run the client
     */
    public void run() throws Exception {
        // Process messages until the stream is closed
        while (processMessage())
            ;

        if (Main.gVerbose)
            System.err.println("Final score: " + mState.myScore());
    }

    /**
     * Process a message from the server
     */
    private boolean processMessage() throws Exception {
        // Read message from stream
        String lString;
        if ((lString = mInputStream.readLine()) == null)
            return false;

        // Parse message
        StringTokenizer lMessage = new StringTokenizer(lString);
        String lMessageType = lMessage.nextToken();

        if (lMessageType.equals("GAME")) {
            // Get my player id and how many players there are in total
            int lWhoIAm = Integer.parseInt(lMessage.nextToken());
            int lNumPlayers = Integer.parseInt(lMessage.nextToken());
            mState = new GameState(lWhoIAm, lNumPlayers);
        } else if (lMessageType.equals("SCORE")) {
            // Read the score for each player
            int[] lScores = new int[mState.getNumPlayers()];
            for (int i = 0; i < lScores.length; ++i)
                lScores[i] = Integer.parseInt(lMessage.nextToken());
            mState.setScores(lScores);

            if (Main.gVerbose)
                System.err.println("My score: " + mState.myScore());
        } else if (lMessageType.equals("ROUND")) {
            // Get current round and number of birds
            int lRound = Integer.parseInt(lMessage.nextToken());
            int lNumBirds = Integer.parseInt(lMessage.nextToken());
            mState.newRound(lRound, lNumBirds);
        } else if (lMessageType.equals("MOVES")) {
            int lNumMoves = Integer.parseInt(lMessage.nextToken());
            int lNewObservations[] = new int[mState.getNumBirds()];

            // Read moves line by line
            for (int i = 0; i < lNumMoves; ++i) {
                if ((lString = mInputStream.readLine()) == null)
                    throw new Exception(
                            "getline failed while reading MOVES in readMessage");

                StringTokenizer lMoves = new StringTokenizer(lString);
                for (int b = 0; b < mState.getNumBirds(); ++b)
                    lNewObservations[b] = Integer.parseInt(lMoves.nextToken());

                // Add the observed moves to the birds
                mState.addMoves(lNewObservations);

                if (lMoves.hasMoreTokens())
                    throw new Exception(
                            "Trailing input for MOVES data in readMessage");
            }
        } else if (lMessageType.equals("SHOOT")) {
            // Read deadline in milliseconds
            int lMs = Integer.parseInt(lMessage.nextToken());

            // Ask the player what to do
            Deadline lDue = new Deadline(lMs);
            Action lAction = mPlayer.shoot(mState, lDue);
            if (lDue.remainingMs() < 0)
                throw new Exception("Player timed out during SHOOT");

            // Mark any new moves as processed
            mState.resetNumNewTurns();

            // Send response
            mOutputStream.println(lAction.getBirdNumber() + " "
                    + lAction.getMovement());
        } else if (lMessageType.equals("GUESS")) {
            // Read deadline in milliseconds
            int lMs = Integer.parseInt(lMessage.nextToken());

            // Ask the player what to do
            Deadline lDue = new Deadline(lMs);
            int lGuesses[] = mPlayer.guess(mState, lDue);
            if (lDue.remainingMs() < 0)
                throw new Exception("Player timed out during GUESS");

            if (lGuesses == null || lGuesses.length != mState.getNumBirds())
                throw new Exception(
                        "Player returned invalid number of birds in GUESS");

            // Mark any new moves as processed
            mState.resetNumNewTurns();

            // Send response
            for (int g : lGuesses)
                mOutputStream.print(g + " ");
            mOutputStream.println();
        } else if (lMessageType.equals("HIT")) {
            // Read which bird we hit
            int lBird = Integer.parseInt(lMessage.nextToken());
            int lMs = Integer.parseInt(lMessage.nextToken());

            // Tell the player
            Deadline lDue = new Deadline(lMs);
            mPlayer.hit(mState, lBird, lDue);
            if (lDue.remainingMs() < 0)
                throw new Exception("Player timed out during HIT");
        } else if (lMessageType.equals("REVEAL")) {
            // Read the species of the birds
            int lRevealedSpecies[] = new int[mState.getNumBirds()];
            for (int i = 0; i < mState.getNumBirds(); ++i)
                lRevealedSpecies[i] = Integer.parseInt(lMessage.nextToken());

            int lMs = Integer.parseInt(lMessage.nextToken());

            // Tell the player
            Deadline lDue = new Deadline(lMs);
            mPlayer.reveal(mState, lRevealedSpecies, lDue);
            if (lDue.remainingMs() < 0)
                throw new Exception("Player timed out during REVEAL");
        } else if (lMessageType.equals("TIMEOUT")) {
            throw new Exception("Received TIMEOUT from server");
        } else if (lMessageType.equals("GAMEOVER")) {
            if (Main.gVerbose)
                System.err.println("Received GAMEOVER from server");
            return false;
        } else {
            throw new Exception("Failed to parse message in readMessage:\n"
                    + lString);
        }

        if (lMessage.hasMoreTokens())
            throw new Exception("Trailing input for " + lMessageType
                    + " in readMessage");

        // Return false if the stream is broken or closed
        return true;
    }

    private Player mPlayer;
    private GameState mState;
    private BufferedReader mInputStream;
    private PrintStream mOutputStream;
};