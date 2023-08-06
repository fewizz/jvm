package java.lang;

public final class Void {

	private static native Class<Void> getPrimitiveClass();

	public static final Class<Void> TYPE = getPrimitiveClass();

}