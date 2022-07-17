package java.lang;

public abstract class Enum<E extends Enum<E>> {

	private String name;
	private int ordinal;

	protected Enum(String name, int ordinal) {
		this.name = name;
		this.ordinal = ordinal;
	}

	public static <T extends Enum<T>> T
	valueOf(Class<T> enumClass, String name) {
		return __valueOf(enumClass, name); // TODO
	}

	private static native <T extends Enum<T>> T
	__valueOf(Class<T> enumClass, String name);

}
