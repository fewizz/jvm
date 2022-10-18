package java.lang;

public class Thread implements Runnable {
	private final Runnable runnable_;

	public Thread(Runnable runnable) {
		this.runnable_ = runnable;
	}

	public native void start();

	@Override
	public void run() {
		this.runnable_.run();
	}

}