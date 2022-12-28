package java.lang;

public final class Integer extends Number implements Comparable<Integer> {

	private static native Class<Integer> getPrimitiveClass();

	public static final Class<Integer> TYPE = getPrimitiveClass();
	
	private final int value_;

	public Integer(int value) {
		this.value_ = value;
	}

	public static native String toString(int i, int radix);

	public static native String toHexString(int i);

	public static Integer valueOf(int i) {
		return new Integer(i);
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

	@Override
	public int hashCode() {
		return value_;
	}

	@Override
	public boolean equals(Object obj) {
		if(obj != null && obj instanceof Integer i) {
			return value_ == i.value_;
		}
		return false;
	}

	@Override
	public int compareTo(Integer anotherInteger) {
		return compare(this.value_, anotherInteger.value_);
	}

	public static int compare(int x, int y) {
		return
			x == y ? 0 :
			x > y ? x : y;
	}

}