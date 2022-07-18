package java.lang;

public class Integer extends Number {
	
	private final int value_;

	public Integer(int value) {
		this.value_ = value;
	}

	public static native String toHexString(int i);

	@Override
	public byte byteValue() {
		return (byte) value_;
	}

	@Override
	public short shortValue() {
		return (short) value_;
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
