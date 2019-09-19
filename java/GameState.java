/**
 * Represents a game state
 */
public class GameState {

    ///returns what round we are currently playing
    int getRound()
    {
        return mRound;
    }

    ///returns the number of birds
    int getNumBirds()
    {
        return mBirds.length;
    }

    ///returns a reference to the i-th bird
    Bird getBird(int i)
    {
        return mBirds[i];
    }

    ///returns the index of your player among all players
    int whoAmI()
    {
        return mWhoIAm;
    }

    ///returns the number of players
    int getNumPlayers()
    {
        return mScores.length;
    }

    ///returns your current score
    int myScore()
    {
        return mScores[mWhoIAm];
    }

    ///returns the score of the i-th player
    int getScore(int i)
    {
        return mScores[i];
    }

    ///returns the number of turns elapsed since last time Shoot was called.
    ///this is the amount of new data available for each bird
    int getNumNewTurns()
    {
        return mNumNewTurns;
    }

    /**
     * The following methods are used by the Client class.
     * Don't use them yourself!
     */
    public GameState(int pWhoIAm, int pNumPlayers)
    {
        mWhoIAm = pWhoIAm;
        mScores = new int[pNumPlayers];
        mRound = -1;
    }

    void newRound(int pRound, int pNumBirds)
    {
        mRound = pRound;
        mBirds = new Bird[pNumBirds];
        for (int b = 0; b < pNumBirds; ++b)
            mBirds[b] = new Bird();
        mNumNewTurns = 0;
    }

    void addMoves(int pMoves[])
    {
        for (int b = 0; b < mBirds.length; ++b)
            mBirds[b].addObservation(pMoves[b]);
        mNumNewTurns += 1;
    }

    void setScores(int pScores[])
    {
        mScores = pScores;
    }

    void resetNumNewTurns()
    {
        mNumNewTurns = 0;
    }

    private int mRound;
    private int mWhoIAm;
    private int mNumNewTurns;
    private Bird[] mBirds;
    private int[] mScores;
}
