package java.lang;

public final class Double
	extends Number
	implements Comparable<Double>
{

	private static native Class<Float> getPrimitiveClass();

	public static final Class<Float> TYPE = getPrimitiveClass();

	private final double value_;

	public Double(double value) {
		this.value_ = value;
	}

	public static Double valueOf(double f) {
		return new Double(f);
	}

	@Override
	public byte byteValue() {
		return (byte) this.value_;
	}

	@Override
	public short shortValue() {
		return (short) this.value_;
	}

	@Override
	public int intValue() {
		return (int) this.value_;
	}

	@Override
	public long longValue() {
		return (long) this.value_;
	}

	@Override
	public float floatValue() {
		return (float) this.value_;
	}

	@Override
	public double doubleValue() {
		return this.value_;
	}

	@Override
	public int hashCode() {
		long v = doubleToLongBits(this.value_);
		return (int)(v^(v>>>32));
	}

	public static int hashCode(double value) {
		long v = doubleToLongBits(value);
		return (int)(v^(v>>>32));
	}

	public boolean equals(Object obj) {
		return
			obj instanceof Double &&
			doubleToLongBits(
				((Double)obj).value_
			) == doubleToLongBits(this.value_);
	}

	public static native long doubleToLongBits(double value);

	public static native long doubleToRawLongBits(double value);

	public static native double longBitsToDouble(long bits);

	@Override
	public int compareTo(Double anotherDouble) {
		return compare(this.value_, anotherDouble.value_);
	}

	public static int compare(double d1, double d2) {
		boolean n1 = d1 != d1;
		boolean n2 = d2 != d2;

		if(n1) {
			return n2 ? 0 : 1;
		}

		if(n2) {
			return n1 ? 0 : -1;
		}

		long bits1 = Double.doubleToLongBits(d1);
		long bits2 = Double.doubleToLongBits(d2);
		return
			bits1 == bits2 ? 0 :
			bits1 >  bits2 ? 1 : -1;
	}

	public static double sum(double a, double b) {
		return a + b;
	}

	public static double max(double a, double b) {
		if(a != a) {
			return a;
		}

		if(b != b) {
			return b;
		}

		return Double.doubleToLongBits(a) > Double.doubleToLongBits(b) ? a : b;
	}

	public static double min(double a, double b) {
		if(a != a) {
			return a;
		}

		if(b != b) {
			return b;
		}

		return Double.doubleToLongBits(a) < Double.doubleToLongBits(b) ? a : b;
	}

}