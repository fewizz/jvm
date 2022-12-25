package java.lang;

public final class Float extends Number implements Comparable<Float> {

	private float value_;

	public Float(float value) {
		this.value_ = value;
	}

	public static native boolean isNaN(float v);

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
		return (long) value_;
	}

	@Override
	public float floatValue() {
		return value_;
	}

	@Override
	public double doubleValue() {
		return value_;
	}

	public static native int floatToIntBits(float value);

	public static native int floatToRawIntBits(float value);

	@Override
	public int compareTo(Float anotherFloat) {
		return compare(value_, anotherFloat.value_);
	}

	public static int compare(float f1, float f2) {
		boolean n1 = f1 != f1;
		boolean n2 = f2 != f2;

		if(n1) {
			return n2 ? 0 : 1;
		}

		if(n2) {
			return n1 ? 0 : -1;
		}

		int bits1 = Float.floatToIntBits(f1);
		int bits2 = Float.floatToIntBits(f2);
		return
			bits1 == bits2 ? 0 :
			bits1 >  bits2 ? 1 : -1;
	}

}