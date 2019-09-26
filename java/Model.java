import java.util.*;

public class Model {
    double[][] A;
    double[][] B;
    double[] Pi;
    int n, m;

    Model(int n, int m) {
        this.n = n;
        this.m = m;
        this.A = new double[n][n];
        this.B = new double[n][m];
        this.Pi = new double[n];

        Random r = new Random();
        double sum = 0;

        for (int i = 0; i < n; i++) {
            sum = 0;
            for (int j = 0; j < n; j++) {
                double rand = r.nextDouble();
                sum += rand;
                A[i][j] = rand;
            }
            for (int j = 0; j < n; j++) {
                A[i][j] /= sum;
            }
        }

        for (int i = 0; i < n; i++) {
            sum = 0;
            for (int j = 0; j < m; j++) {
                double rand = r.nextDouble();
                sum += rand;
                B[i][j] = rand;
            }
            for (int j = 0; j < m; j++) {
                B[i][j] /= sum;
            }
        }

        for (int i = 0; i < n; i++) {
            sum = 0;
            double rand = r.nextDouble();
            sum += rand;
            Pi[i] = rand;
        }
        for (int i = 0; i < n; i++) {
            Pi[i] /= sum;
        }

    }

    public double estimateEmissionSequence(int[] O) {
        double[] Alpha = new double[n];
        
        for (int i = 0; i < n; i++) {
            //Alpha[i] = Math.exp(Math.log(Pi[i]) + Math.log(B[i][O[0]]));
            Alpha[i] = Pi[i] * B[i][O[0]];
        }

        double[] AlphaTmp = new double[n];
        for (int k = 1; k < O.length; k++) {
            for (int i = 0; i < n; i++) {
                double tmp = 0;
                for (int j = 0; j < n; j++) {
                    //tmp += Math.exp(Math.log(Alpha[j]) + Math.log(A[j][i]));
                    tmp += Alpha[j] * A[j][i];
                }
                //AlphaTmp[i] = Math.exp(Math.log(tmp) + Math.log(B[i][O[k]]));
                AlphaTmp[i] = tmp * B[i][O[k]];
            }

            for (int i = 0; i < n; i++) {
                Alpha[i] = AlphaTmp[i];
            }
        }
        double marginalize = 0.0;
        for (int i = 0; i < n; i++) {
            marginalize += Alpha[i];
        }

        //System.err.println(marginalize);
        //System.err.println(O.length);

        return marginalize;
    }

    public double[] getNextEmission(double[][] Alpha) {

        double[] emissionProb = new double[m];

        for (int k = 0; k < m; k++) {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    emissionProb[k] += A[j][i] * B[i][k] * Alpha[j][Alpha[j].length - 1];
                }
            }
        }

        double max = 0;
        int emission = -1;
        for (int k = 0; k < m; k++) {
            if (max < emissionProb[k]) {
                max = emissionProb[k];
                emission = k;
            }
        }
        double[] result = new double[2];
        result[0] = max;
        result[1] = emission;
        return result;
    }

    public double logScale(double[] C) {
        double logP = 0;
        for (int k = 0; k < m; k++) {
            logP += Math.log(C[k]);
        }
        return -logP;
    }

    public double[][] estimate(int[] O) {
        int l = O.length;

        double[][] Alpha = new double[n][l];
        double[][] Beta = new double[n][l];
        double[][][] diGamma = new double[n][n][l];
        double[][] Gamma = new double[n][l];
        double[] C = new double[l];

        double oldProb = Double.NEGATIVE_INFINITY;

        for (int t = 0; t < 10; t++) {

            // ALPHA PASS
            // initialize alpha
            C[0] = 0;
            for (int i = 0; i < n; i++) {
                Alpha[i][0] = Pi[i] * B[i][O[0]];
                C[0] += Alpha[i][0];
            }

            // Scale
            C[0] = 1 / C[0];
            for (int i = 0; i < n; i++) {
                Alpha[i][0] *= C[0];
            }

            // finish alpha-pass
            for (int k = 1; k < l; k++) {
                C[k] = 0;
                for (int i = 0; i < n; i++) {
                    Alpha[i][k] = 0;
                    for (int j = 0; j < n; j++) {
                        Alpha[i][k] += Alpha[j][k - 1] * A[j][i];
                    }
                    Alpha[i][k] *= B[i][O[k]];
                    C[k] += Alpha[i][k];
                }
                // Scale
                C[k] = 1 / C[k];
                for (int i = 0; i < n; i++) {
                    Alpha[i][k] *= C[k];
                }
            }

            // BETA PASS
            // usually Beta[i,T-1] = 1 but we scale.
            for (int i = 0; i < n; i++) {
                Beta[i][l - 1] = C[l - 1];
            }

            // finish beta-pass
            for (int k = l - 2; k >= 0; k--) {
                for (int i = 0; i < n; i++) {
                    Beta[i][k] = 0;
                    for (int j = 0; j < n; j++) {
                        // Scale w same C as aplha
                        Beta[i][k] += A[i][j] * B[j][O[k + 1]] * Beta[j][k + 1];
                    }
                    Beta[i][k] *= C[k];
                }
            }

            // diGAMMA GAMMA PASS
            // compute edge case
            for (int i = 0; i < n; i++) {
                Gamma[i][l - 1] = Alpha[i][l - 1];
            }

            for (int k = 0; k < l - 1; k++) {
                for (int i = 0; i < n; i++) {
                    Gamma[i][k] = 0;
                    for (int j = 0; j < n; j++) {
                        diGamma[i][j][k] = Alpha[i][k] * A[i][j] * B[j][O[k + 1]] * Beta[j][k + 1];
                        Gamma[i][k] += diGamma[i][j][k];
                    }
                }
            }

            // estimate initial distribution
            for (int i = 0; i < n; i++) {
                Pi[i] = Gamma[i][0];
            }

            // estimate transition matrix
            for (int i = 0; i < n; i++) {
                double d = 0;
                for (int k = 0; k < l - 1; k++) {
                    d += Gamma[i][k];
                }
                for (int j = 0; j < n; j++) {
                    double num = 0;
                    for (int k = 0; k < l - 1; k++) {
                        num += diGamma[i][j][k];
                    }
                    A[i][j] = num / d;
                }
            }

            // estimate emission matrix
            for (int i = 0; i < n; i++) {
                double d = 0;
                for (int k = 0; k < l; k++) {
                    d += Gamma[i][k];
                }
                for (int j = 0; j < m; j++) {
                    double num = 0;
                    for (int k = 0; k < l; k++) {
                        num += O[k] == j ? Gamma[i][k] : 0;
                    }
                    B[i][j] = num / d;
                }
            }
            double newPr = logScale(C);
            if (newPr < oldProb)
                break; // break if not improving, could put some threshold aswell
            oldProb = newPr;
        }

        return Alpha;
    }
}