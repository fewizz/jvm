package java.lang.invoke;

public class MethodHandles {

	public static native Lookup lookup();

	public static Lookup publicLookup() {
		return new Lookup(Object.class);
	}

	public static final class Lookup {
		Class<?> caller_;

		private Lookup(Class<?> caller) {
			this.caller_ = caller;
		}

		public native MethodHandle findGetter(
			Class<?> refc, String name, Class<?> type
		) throws NoSuchFieldException, IllegalAccessException;

		public native MethodHandle findSetter(
			Class<?> refc, String name, Class<?> type
		) throws NoSuchFieldException, IllegalAccessException;

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