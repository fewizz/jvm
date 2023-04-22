package java.lang;

public final class Boolean {

	private static native Class<Boolean> getPrimitiveClass();

	public static final Class<Boolean> TYPE = getPrimitiveClass();

	private final boolean value_;

	public Boolean(boolean value) {
		this.value_ = value;
	}

	public boolean booleanValue() {
		return this.value_;
	}

}