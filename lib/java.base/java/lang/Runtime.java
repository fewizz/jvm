package java.lang;

public class Runtime {

	private static final Runtime runtime_ = new Runtime();

	private Runtime() {}

	public static Runtime getRuntime() {
		return runtime_;
	}

	public native void exit(int status);

	public native int availableProcessors();

}