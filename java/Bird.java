import java.util.Vector;

/**
 * Represents a bird with history of movements
 */
class Bird {
    // /length of the sequence of past actions of the duck
    public int getSeqLength() {
        return mSeq.size();
    }

    // /returns the last action of the bird
    public int getLastObservation() {
        return getObservation(mSeq.size() - 1);
    }

    // /returns one action in the sequence of actions of the bird
    public int getObservation(int i) {
        return mSeq.get(i);
    }

    // /returns true if the bird is dead
    public boolean isDead() {
        return getLastObservation() == Constants.MOVE_DEAD;
    }

    // /returns true if the duck was dead at time step i
    public boolean wasDead(int i) {
        return getObservation(i) == Constants.MOVE_DEAD;
    }

    // /returns true if the bird is alive
    public boolean isAlive() {
        return !isDead();
    }

    // /returns true if the duck was alive at time step i
    public boolean wasAlive(int i) {
        return !wasDead(i);
    }

    /**
     * The following methods are used by the Client class You should not use
     * them yourself!
     */
    public void addObservation(int pMovement) {
        if (!mSeq.isEmpty() && mSeq.lastElement() == Constants.MOVE_DEAD)
            mSeq.add(Constants.MOVE_DEAD);
        else
            mSeq.add(pMovement);
    }

    public void kill() {
        mSeq.set(mSeq.size() - 1, Constants.MOVE_DEAD);
    }

    private Vector<Integer> mSeq = new Vector<Integer>();
}
