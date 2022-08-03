package java.lang.invoke;

public class MethodHandles {

	// without access checks for now

	public static Lookup lookup() {
		return new Lookup();
	}

	static final class Lookup {

		public native MethodHandle findStatic(
			Class<?> refc, String name, MethodType type
		) throws NoSuchMethodException, IllegalAccessException;

	}

}