package java.lang;

public final class Long extends Number {

	private final long value_;

	public Long(long value) {
		this.value_ = value;
	}

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
		return (int) value_;
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
