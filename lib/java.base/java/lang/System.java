package java.lang;

public final class System {

	public static native long nanoTime();

	public static native void arraycopy(
		Object src, int srcPos, Object dest, int destPos, int length
	);

	public static void exit(int status) {
		Runtime.getRuntime().exit(status);
	}

}