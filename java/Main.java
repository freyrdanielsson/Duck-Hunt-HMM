import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class Main
{

    public static boolean gVerbose = false;

    /**
     * @param args
     * @throws IOException
     */
    public static void main(String[] args) throws Exception {
        // Parse parameters
        boolean lCreateServer = false;
        String lLoadFilename = "SouthEmissions.in";

        for (int i = 0; i < args.length; ++i) {
            String param = args[i];

            if (param.equals("server") || param.equals("s")) {
                lCreateServer = true;
            } else if (param.equals("verbose") || param.equals("v")) {
                gVerbose = true;
            } else if (param.equals("load") || param.equals("l")) {
                ++i;
                if (i < args.length)
                    lLoadFilename = args[i];
                else
                {
                    System.err.println("Observations file must be given as an argument");
                    System.exit(-1);
                }
            } else {
                System.err.println("Unknown parameter: '" + args[i] + "'");
                System.exit(-1);
            }
        }

        /**
         * Start the program either as a server or a client
         */
        if (lCreateServer)
        {
            // Create a server
            GameServer lGameServer = new GameServer(
                    new BufferedReader(new InputStreamReader(System.in)),
                    System.out);

            if (lLoadFilename != null)
            {
                if (gVerbose)
                    System.err.println("Loading '" + lLoadFilename + "'");
                lGameServer.load(new FileReader(lLoadFilename));
            }

            // Run the server
            lGameServer.run();
        }
        else
        {
            // Create the player
            Player lPlayer = new Player();

            // Create a client with the player
            Client lClient = new Client(
                    lPlayer,
                    new BufferedReader(new InputStreamReader(System.in)),
                    System.out);

            // Run the client
            lClient.run();
        }
    }
}
