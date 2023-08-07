package java.lang;

public class Thread implements Runnable {
	private final Runnable runnable_;

	public Thread(Runnable runnable) {
		this.runnable_ = runnable;
	}

	native public static void sleep(long millis) throws InterruptedException;

	public native void start();

	@Override
	public void run() {
		this.runnable_.run();
	}

}