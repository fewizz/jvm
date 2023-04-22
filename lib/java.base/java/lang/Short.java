package java.lang;

public final class Short extends Number {

	private static native Class<Float> getPrimitiveClass();

	public static final Class<Float> TYPE = getPrimitiveClass();

	private final short value_;

	public Short(short value) {
		this.value_ = value;
	}

	@Override
	public byte byteValue() {
		return (byte) value_;
	}

	@Override
	public short shortValue() {
		return shortValue();
	}

	@Override
	public int intValue() {
		return value_;
	}

	@Override
	public long longValue() {
		return value_;
	}

	@Override
	public float floatValue() {
		return value_;
	}

	@Override
	public double doubleValue() {
		return value_;
	}

}
