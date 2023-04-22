package java.lang;

public final class Byte extends Number {

	private static native Class<Byte> getPrimitiveClass();

	public static final Class<Byte> TYPE = getPrimitiveClass();
	
	private final byte value_;

	public Byte(byte value) {
		this.value_ = value;
	}

	@Override
	public byte byteValue() {
		return value_;
	}

	@Override
	public short shortValue() {
		return value_;
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