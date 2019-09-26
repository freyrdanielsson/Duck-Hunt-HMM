import java.util.ArrayList;
import java.util.*;


class Player {

    ArrayList < Model > [] speciesModels;
    ArrayList < Model > [] birdModels;
    int n, m;

    public Player() {
        this.n = 5;
        this.m = 9;
        this.speciesModels = new ArrayList[6];
        for (int i = 0; i < 6; i++) {
            this.speciesModels[i] = new ArrayList <Model>();
        }
    }

    public int[] getObsSeq(Bird bird) {
        int cnt = 0;
        for (int i = 0; i < bird.getSeqLength(); i++) {
            if (bird.wasAlive(i)) cnt++;
        }

        int[] O = new int[cnt];
        for (int i = 0; i < cnt; i++) {
            if (bird.wasAlive(i)) O[i] = bird.getObservation(i); // Does it matter if he is alive..? 
        }
        return O;
    }

    public Action shoot(GameState pState, Deadline pDue) {

        int nBirds = pState.getNumBirds();
        int openSeason = 100 - nBirds;

        if (pState.getBird(0).getSeqLength() == 1) {
            // each bird will have openSeason nr of models.
            birdModels = new ArrayList[nBirds];
            for (int i = 0; i < nBirds; i++) {
                birdModels[i] = new ArrayList < Model > (0);
            }
            System.err.println("Start Round " + pState.getRound());
        }

        int victim = -1;
        double victimOdds = 0.75;
        int nextBirdMove = -1;

        // For each bird
        for (int b = 0; b < nBirds; b++) {
            Bird bird = pState.getBird(b);
            int nObs = bird.getSeqLength();

            if (nObs >= openSeason && bird.isAlive()) {
                Model model = new Model(n, m);
                int[] O = getObsSeq(bird);

                double[][] Alpha = model.estimate(O);
                birdModels[b].add(model);

                if (pState.getRound() > 0) {

                    /* double[] maxSpecie = new double[Constants.COUNT_SPECIES];
                    double norm = 0;
                    //double[] avgL = new double[Constants.COUNT_SPECIES];

                    // Ones to choice, and their likelyhood
                    int specie = -1;
                    double bestOfAllS = 0;

                    for (int i = 0; i < Constants.COUNT_SPECIES; i++) {
                        double specieMaxP = 0;

                        // IDEA: keep also in which round the model was collected
                        // and then just do estimation from the latest model for the bird in each round
                        for (Model m: speciesModels[i]) {
                            double tmpSpecieMaxP = m.estimateEmissionSequence(O);
                            if (specieMaxP <= tmpSpecieMaxP) {
                                specieMaxP = tmpSpecieMaxP;
                            }
                        }
                        //avgL[i] = avg / COUNT_SPECIES;
                        maxSpecie[i] = specieMaxP;
                        norm += specieMaxP;
                    }

                    for (int i = 0; i < Constants.COUNT_SPECIES; i++) {
                        maxSpecie[i] /= norm;
                    }

                    for (int i = 0; i < Constants.COUNT_SPECIES; i++) {
                        if (bestOfAllS < maxSpecie[i]) {
                            bestOfAllS = maxSpecie[i];
                            specie = i;
                        }
                    }

                    if (speciesModels[Constants.SPECIES_BLACK_STORK].isEmpty() || specie == Constants.SPECIES_BLACK_STORK || (maxSpecie[Constants.SPECIES_BLACK_STORK] > 0.1)) {
                        continue;
                    }

                    double[] action = model.getNextEmission(Alpha);

                    if (action[0] >= victimOdds) {
                        victimOdds = action[0];
                        victim = b;
                        nextBirdMove = (int) action[1];
                    } */
                }
            }
        }
        return cDontShoot;
        //return new Action(victim, nextBirdMove);
    }


    public int[] guess(GameState pState, Deadline pDue) {

        int[] specieGuess = new int[pState.getNumBirds()];

        if (pState.getRound() == 0) {
            Random rand = new Random();
            // Don't guess the stork, it be rare, hence -1
            for (int i = 0; i < pState.getNumBirds(); i++) {
                specieGuess[i] = rand.nextInt(Constants.COUNT_SPECIES - 1);
                return specieGuess;
            }
        }


        for (int b = 0; b < pState.getNumBirds(); b++) {
            Bird bird = pState.getBird(b);
            int[] O = getObsSeq(bird);
            double[] maxSpecie = new double[Constants.COUNT_SPECIES];
            double norm = 0;

            // Ones to choice, and their likelyhood
            int specie = -1;
            double bestOfAllS = 0;

            for (int i = 0; i < Constants.COUNT_SPECIES; i++) {
                double specieMaxP = 0;

                for (Model model: speciesModels[i]) {
                    double tmpSpecieMaxP = model.estimateEmissionSequence(O);
                    if (specieMaxP < tmpSpecieMaxP) {
                        maxSpecie[i] = tmpSpecieMaxP;
                    }
                }
            }

            // Normalize
            double sum = 0;
            for (int i = 0; i < Constants.COUNT_SPECIES; i++) {
                sum += maxSpecie[i];
            }
            for (int i = 0; i < Constants.COUNT_SPECIES; i++) {
                maxSpecie[i] /= sum;
            }

            for (int i = 0; i < Constants.COUNT_SPECIES; i++) {
                if (bestOfAllS < maxSpecie[i]) {
                    bestOfAllS = maxSpecie[i];
                    specie = i;
                }
            }

            /* System.err.println("Best: " + specie);
            for (int i = 0; i < Constants.COUNT_SPECIES; i++) {
                System.err.print(maxSpecie[i] + " ");
            }
            System.err.println(); */

            specieGuess[b] = specie;
        }

        return specieGuess;
    }

    /**
     * If you hit the bird you were trying to shoot, you will be notified through
     * this function.
     *
     * @param pState the GameState object with observations etc
     * @param pBird  the bird you hit
     * @param pDue   time before which we must have returned
     */
    public void hit(GameState pState, int pBird, Deadline pDue) {
        System.err.println("HIT BIRD!!!");
    }

    /**
     * If you made any guesses, you will find out the true species of those birds
     * through this function.
     *
     * @param pState   the GameState object with observations etc
     * @param pSpecies the vector with species
     * @param pDue     time before which we must have returned
     */
    public void reveal(GameState pState, int[] pSpecies, Deadline pDue) {
        for (int i = 0; i < pSpecies.length; i++) {
            if (pSpecies[i] == Constants.SPECIES_BLACK_STORK)
                System.err.println("put in stork, bird nr: " + i);

            if (pSpecies[i] < 0)
                continue;

            for (Model model: birdModels[i]) {
                speciesModels[pSpecies[i]].add(model);
            }
        }
    }

    public static final Action cDontShoot = new Action(-1, -1);
}