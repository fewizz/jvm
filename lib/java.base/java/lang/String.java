package java.lang;

public final class String {

	private final char[] value_;
	
	public String(char[] value) {
		this.value_ = new char[value.length];
		System.arraycopy(value, 0, this.value_, 0, value.length);
	}

	public int length() {
		return value_.length;
	}

	public boolean isEmpty() {
		return length() == 0;
	}

	public char charAt(int index) {
		return value_[index];
		// TODO throws IndexOutOfBoundsException
	}

	
	public boolean startsWith(String prefix) {
		int prefix_len = prefix.length();

		if(prefix_len > length()) {
			return false;
		}

		for(int i = 0; i < prefix_len; ++i) {
			if(this.charAt(i) != prefix.charAt(i)) {
				return false;
			}
		}

		return true;
	}

	@Override
	public int hashCode() {
		int result = 0;
		int mul = 1;

		for(int x = length() - 1; x >= 0; --x) {
			result += value_[x] * mul;
			mul *= 31;
		}

		return result;
	}

	@Override
	public String toString() {
		return this;
	}

	public char[] toCharArray() {
		char[] copy = new char[value_.length];
		System.arraycopy(value_, 0, copy, 0, value_.length);
		return copy;
	}

}