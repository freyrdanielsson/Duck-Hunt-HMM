public class Action {

    /**
     * Construct a bird action object
     *
     * \param pBirdNumber the bird index \param pMovement the movement of the
     * bird
     */
    public Action(int pBirdNumber, int pMovement) {
        mBirdNumber = pBirdNumber;
        mMovement = pMovement;
    }

    // /the index of the bird this action corresponds too
    int getBirdNumber() {
        return mBirdNumber;
    }

    // /the movement of the bird

    // /can be either BIRD_STOPPED or one of the eight basic directions
    int getMovement() {
        return mMovement;
    }

    // /represents a no-shoot action
    boolean isDontShoot() {
        return (mBirdNumber == -1);
    }

    // /prints the content of this action object
    public String toString() {
        if (isDontShoot())
            return "DONT SHOOT";
        else {
            if (mMovement == Constants.MOVE_DEAD)
                return mBirdNumber + " DEAD";
            else {
                String move_names[] = {
                        " UP LEFT", " UP", " UP RIGHT",
                        " LEFT", " STOPPED", " RIGHT",
                        " DOWN LEFT", " DOWN", " DOWN RIGHT" };
                return mBirdNumber + move_names[mMovement];
            }
        }
    }

    @Override
    public boolean equals(Object pRH) {
        // Self comparison?
        if (this == pRH)
            return true;

        // Wrong type?
        if (!(pRH instanceof Action))
            return false;

        // Check actual data
        Action lRH = (Action) pRH;
        return mBirdNumber == lRH.mBirdNumber && mMovement == lRH.mMovement;
    }

    private int mBirdNumber;
    private int mMovement;
}
