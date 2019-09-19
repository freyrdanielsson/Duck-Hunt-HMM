import java.io.BufferedReader;
import java.io.IOException;
import java.io.PrintStream;
import java.io.StringReader;
import java.util.Arrays;
import java.util.Scanner;
import java.util.StringTokenizer;

/**
 * The GameServer runs the game and communicates with the client through text.
 * The communications protocol looks like this:
 *
 * GAME PlayerID NumPlayers
 * ROUND Round NumBirds
 * MOVES NumObservations
 * <Observation of bird 0> <Observation of bird 1> ...
 * <Observation of bird 0> <Observation of bird 1> ...
 * ...
 * SHOOT Deadline
 * --wait for response--
 * HIT Bird Deadline (only sent if the player hit the bird)
 * ...
 * GUESS Deadline
 * --wait for response--
 * REVEAL <specie of bird 0> <specie of bird 1> ... Deadline
 * SCORE <Score of player 0> <Score of player 1> ..
 *
 * TIMEOUT (happens when player fails to meet the deadline)
 *
 * Notes:
 * Deadlines are expressed as how many milliseconds the client is allowed to use
 * before responding.
 * Rounds start at 0.
 * There is only one environment per game.
 * The protocol supports several players but the GameServer currently does not.
 */

class GameServer
{

    private class SPlayer
    {
        SPlayer(BufferedReader pInputStream, PrintStream pOutputStream, int pID)
        {
            mInputStream = pInputStream;
            mOutputStream = pOutputStream;
            mID = pID;
            mNumSent = 0;
            mScore = 0;
            mGameOver = false;
        }

        public BufferedReader mInputStream;
        public PrintStream mOutputStream;

        int mID;
        int mNumSent;
        int mScore;
        boolean mGameOver;
    }

    private class BirdSequence
    {
        public int mSpecies;
        public int[] mActions;
    }

    public GameServer(BufferedReader pInputStream, PrintStream pOutputStream)
    {
        mMaxRounds = 2;
        mMaxTurns = 100;
        mTimeForShoot = 2000;
        mTimeForHit = 200;
        mTimeForGuess = 10000;
        mTimeForReveal = 1000;
        mPlayers = new SPlayer[1];
        mPlayers[0] = new SPlayer(pInputStream, pOutputStream, 0);
    }

    public void load(Readable pStream)
    {
        // Parse the environment file
        Scanner lScanner = new Scanner(pStream);
        mMaxRounds = lScanner.nextInt();
        mEnvironment = new BirdSequence[mMaxRounds][];
        for (int r = 0; r < mMaxRounds; ++r)
        {
            // Create new Round
            int lNumBirds = lScanner.nextInt();
            mEnvironment[r] = new BirdSequence[lNumBirds];
            for (int b = 0; b < lNumBirds; ++b)
            {
                // Create sequence
                mEnvironment[r][b] = new BirdSequence();

                // Read species
                mEnvironment[r][b].mSpecies = lScanner.nextInt();

                // Read observations
                mEnvironment[r][b].mActions = new int[100];
                for (int i = 0; i < 100; ++i)
                    mEnvironment[r][b].mActions[i] = lScanner.nextInt();
            }
        }
        lScanner.close();
    }

    public void run()
    {
        // Load default game if nothing is loaded
        if (mEnvironment == null)
        {
            System.err.println("No environment loaded");
            System.exit(-1);
        }

        if (Main.gVerbose)
            System.err.println("Starting game with " + mPlayers.length
                    + (mPlayers.length == 1 ? " player" : " players"));

        // Send start of game
        for (int i = 0; i < mPlayers.length; ++i)
            mPlayers[i].mOutputStream.println("GAME " + i + " " + mPlayers.length);

        // The players take turns shooting
        int lActivePlayer = 0;

        // Play all rounds
        for (int r = 0; r < mEnvironment.length; ++r)
        {
            // Generate birds for this round
            int lNumBirds = mEnvironment[r].length;
            mBirds = new Bird[lNumBirds];
            mBirdSpecies = new int[lNumBirds];
            for (int b = 0; b < lNumBirds; ++b)
            {
                // Set species
                mBirdSpecies[b] = mEnvironment[r][b].mSpecies;

                // Add the first observation
                mBirds[b] = new Bird();
                mBirds[b].addObservation(mEnvironment[r][b].mActions[0]);
            }

            if (Main.gVerbose)
                System.err.println("Starting round " + r + " with " + mBirds.length + " birds");

            // Send start of round
            for (SPlayer lPlayer : mPlayers)
            {
                lPlayer.mNumSent = 0;
                sendRound(lPlayer, r);
            }

            // Let the players take turns shooting, start with one observation
            for (int i = 1; i < mMaxTurns; ++i)
            {
                // Let the birds fly
                for (int b = 0; b < mEnvironment[r].length; ++b)
                    mBirds[b].addObservation(mEnvironment[r][b].mActions[i]);

                SPlayer lPlayer = mPlayers[lActivePlayer];
                lActivePlayer = (lActivePlayer + 1) % mPlayers.length;

                // Send birds
                sendBirds(lPlayer);

                // Let the player shoot
                playerShoot(lPlayer);

                // Stop if we have no players left
                if (playersLeft() == 0)
                    return;

                // End the round if all birds are dead
                boolean lAnyAlive = false;
                for (Bird lBird : mBirds)
                {
                    if (!lBird.isDead())
                    {
                        lAnyAlive = true;
                        break;
                    }
                }
                if (lAnyAlive == false)
                    break;
            }
            // End of round

            // Send any trailing observations
            for (SPlayer lPlayer : mPlayers)
                sendBirds(lPlayer);

            // Send scores to all players
            for (SPlayer lPlayer : mPlayers)
                sendScores(lPlayer);

            // Let the players guess species
            for (SPlayer lPlayer : mPlayers)
                playerGuess(lPlayer);

            // Send scores to all players
            for (SPlayer lPlayer : mPlayers)
                sendScores(lPlayer);

            // Stop if we have no players left
            if (playersLeft() == 0)
                return;
        }

        if (Main.gVerbose)
        {
            System.err.print("Final scores:");
            for (SPlayer lPlayer : mPlayers)
                System.err.print(" " + lPlayer.mScore);
            System.err.println();
        }
    }

    private void playerShoot(SPlayer pPlayer)
    {
        if (pPlayer.mGameOver)
            return;

        // Ask the player to shoot
        Deadline lDue = new Deadline(mTimeForShoot);
        pPlayer.mOutputStream.println("SHOOT " + lDue.remainingMs());

        if (Main.gVerbose)
            System.err.println("Waiting for player to shoot");

        // Read message from stream
        String lString;
        try
        {
            if ((lString = pPlayer.mInputStream.readLine()) == null)
            {
                System.err.println("getline failed for player " + pPlayer.mID);
                pPlayer.mGameOver = true;
                return;
            }
        }
        catch (IOException e)
        {
            System.err.println("getline failed for player " + pPlayer.mID);
            pPlayer.mGameOver = true;
            return;
        }

        if (Main.gVerbose)
            System.err.println("Got message from player: " + lString);

        if (lDue.remainingMs() < 0)
        {
            System.err.println("Player " + pPlayer.mID + " timed out");
            removePlayer(pPlayer, "TIMEOUT");
            return;
        }

        // Parse the message
        StringTokenizer lIn = new StringTokenizer(lString);
        int lBird, lMovement;
        try
        {
            lBird = Integer.parseInt(lIn.nextToken());
            lMovement = Integer.parseInt(lIn.nextToken());
        }
        catch (Exception e)
        {
            System.err.println("Failed to parse action for player " + pPlayer.mID);
            pPlayer.mGameOver = true;
            return;
        }

        if (lBird >= 0 && lBird < mBirds.length)
        {
            int lTrueMovement = mBirds[lBird].getLastObservation();
            if (lMovement == lTrueMovement && lMovement != Constants.MOVE_DEAD)
            {
                // Mark the bird as dead
                mBirds[lBird].kill();
                pPlayer.mScore += 1;
                int lSpecies = mBirdSpecies[lBird];
                if (lSpecies == Constants.SPECIES_BLACK_STORK)
                {
                    // Hitting the black stork means disqualification
                    pPlayer.mScore = 0;
                    removePlayer(pPlayer, "GAMEOVER");
                    return;
                }

                // Tell the player that it hit the bird
                // The time is only measured in the client since we don't ask for a response
                pPlayer.mOutputStream.println("HIT " + lBird + " " + mTimeForHit);
            }
            else
            {
                pPlayer.mScore -= 1;
            }
        }
    }

    private void playerGuess(SPlayer pPlayer)
    {
        if (pPlayer.mGameOver)
            return;

        // Ask the player to guess
        Deadline lDue = new Deadline(mTimeForGuess);
        pPlayer.mOutputStream.println("GUESS " + lDue.remainingMs());

        if (Main.gVerbose)
            System.err.println("Waiting for player to guess");

        String lString;
        try
        {
            if ((lString = pPlayer.mInputStream.readLine()) == null)
            {
                System.err.println("getline failed for player " + pPlayer.mID);
                pPlayer.mGameOver = true;
                return;
            }
        }
        catch (IOException e)
        {
            System.err.println("getline failed for player " + pPlayer.mID);
            pPlayer.mGameOver = true;
            return;
        }

        if (Main.gVerbose)
            System.err.println("Got message from player: " + lString);

        if (lDue.remainingMs() < 0)
        {
            System.err.println("Player " + pPlayer.mID + " timed out");
            removePlayer(pPlayer, "TIMEOUT");
            return;
        }

        // Parse the message and score the guesses
        StringTokenizer lIn = new StringTokenizer(lString);
        int lScore = 0;
        int[] lRevealing = new int[mBirds.length];
        Arrays.fill(lRevealing, Constants.SPECIES_UNKNOWN);
        boolean lDoReveal = false;
        for (int i = 0; i < mBirds.length; ++i)
        {
            int lGuessedSpecies = -1;
            try
            {
                lGuessedSpecies = Integer.parseInt(lIn.nextToken());
            }
            catch (Exception e)
            {
                System.err.println("Failed to read guess from player for bird " + i);
                pPlayer.mGameOver = true;
                return;
            }

            if (lGuessedSpecies == Constants.SPECIES_UNKNOWN)
                continue;

            int lTrueSpecies = mBirdSpecies[i];
            if (lTrueSpecies == lGuessedSpecies)
                lScore += 1;
            else
                lScore -= 1;

            lRevealing[i] = lTrueSpecies;
            lDoReveal = true;
        }

        if (lIn.hasMoreTokens())
        {
            System.err.println("Trailing output when reading guess:\n" + lString);
            pPlayer.mGameOver = true;
            return;
        }

        if (Main.gVerbose)
            System.err.println("Score for guessing: " + lScore);

        pPlayer.mScore += lScore;

        // Reveal the true species of the birds to the player for the ones he/she made guessed for
        // if the player made any guesses
        if (lDoReveal)
        {
            pPlayer.mOutputStream.print("REVEAL");
            for (int lSpecies : lRevealing)
                pPlayer.mOutputStream.print(" " + lSpecies);
            pPlayer.mOutputStream.println(" " + mTimeForReveal);
            // The time is only measured in the client since we don't ask for a response
        }
    }

    private void removePlayer(SPlayer pPlayer, String pMessage)
    {
        pPlayer.mOutputStream.println(pMessage);
        pPlayer.mGameOver = true;
    }

    private int playersLeft()
    {
        int lPlayersLeft = 0;
        for (SPlayer p : mPlayers)
            if (!p.mGameOver)
                ++lPlayersLeft;
        return lPlayersLeft;
    }

    private void sendRound(SPlayer pPlayer, int pRound)
    {
        if (pPlayer.mGameOver)
            return;

        pPlayer.mOutputStream.println("ROUND " + pRound + " " + mBirds.length);
    }

    private void sendBirds(SPlayer pPlayer)
    {
        if (pPlayer.mGameOver)
            return;

        // Don't send the newest observation
        // It needs to be secret so we have something to check against when shooting
        int lToSend = mBirds[0].getSeqLength() - 1;

        // Abort if there are no new moves to send
        if (pPlayer.mNumSent >= lToSend)
            return;

        // Observations header
        pPlayer.mOutputStream.println("MOVES " + (lToSend - pPlayer.mNumSent));

        // Observations
        for (; pPlayer.mNumSent < lToSend; ++pPlayer.mNumSent)
        {
            for (int i = 0; i < mBirds.length; ++i)
                pPlayer.mOutputStream.print(mBirds[i].getObservation(pPlayer.mNumSent) + " ");
            pPlayer.mOutputStream.println();
        }
    }

    private void sendScores(SPlayer pPlayer)
    {
        if (pPlayer.mGameOver)
            return;

        pPlayer.mOutputStream.print("SCORE");
        for (SPlayer lPlayer : mPlayers)
            pPlayer.mOutputStream.print(" " + lPlayer.mScore);
        pPlayer.mOutputStream.println();
    }

    private int mMaxRounds;
    private int mMaxTurns;

    private long mTimeForShoot;
    private long mTimeForHit;
    private long mTimeForGuess;
    private long mTimeForReveal;

    private SPlayer[] mPlayers;
    private BirdSequence[][] mEnvironment;
    private Bird[] mBirds;
    private int[] mBirdSpecies;
}
