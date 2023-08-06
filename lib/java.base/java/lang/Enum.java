package java.lang;

public abstract class Enum<E extends Enum<E>> {

	private String name_;
	private int ordinal_;

	protected Enum(String name, int ordinal) {
		this.name_ = name;
		this.ordinal_ = ordinal;
	}

	public static native <T extends Enum<T>> T
	valueOf(Class<T> enumClass, String name);

}
