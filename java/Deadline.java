import java.util.Date;

public class Deadline {
    private long mTime;

    public Deadline() {
        mTime = (new Date()).getTime();
    }

    public Deadline(long pTime) {
        mTime = (new Date()).getTime() + pTime;
    }

    long remainingMs() {
        return mTime - (new Date()).getTime();
    }
}