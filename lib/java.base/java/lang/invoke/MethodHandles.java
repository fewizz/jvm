package java.lang.invoke;

public class MethodHandles {

	// without access checks for now

	public static Lookup lookup() {
		return new Lookup();
	}

	public static final class Lookup {

		public native MethodHandle findStatic(
			Class<?> refc, String name, MethodType type
		) throws NoSuchMethodException, IllegalAccessException;

		public native MethodHandle findVirtual(
			Class<?> refc, String name, MethodType type
		) throws NoSuchMethodException, IllegalAccessException;

		public native MethodHandle findConstructor(
			Class<?> refc, MethodType type
		)
		throws NoSuchMethodException, IllegalAccessException;

		public native MethodHandle findSpecial(
			Class<?> refc, String name, MethodType type, Class<?> specialCaller
		) throws NoSuchMethodException, IllegalAccessException;

	}

}