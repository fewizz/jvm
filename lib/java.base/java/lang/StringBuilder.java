package java.lang;

public final class StringBuilder {

	private char buffer_[] = null;
	private int length_ = 0;

	public StringBuilder() { this(16); }

	public StringBuilder(int capacity) {
		this.buffer_ = new char[capacity];
	}

	public StringBuilder(String str) {
		char str0[] = str.toCharArray();
		this.buffer_ = new char[str0.length + 16];
		System.arraycopy(str0, 0, this.buffer_, 0, str0.length);
		this.length_ = str0.length;
	}

	private StringBuilder appendRaw(String str) {
		return append(str.toCharArray());
	}

	public StringBuilder append(String str) {
		return str == null ? appendRaw("null") : appendRaw(str);
	}

	public StringBuilder append(char[] str) {
		int remaining = buffer_.length - length_;
		if(remaining < str.length) {
			char prevBuffer[] = this.buffer_;

			this.buffer_ = new char[str.length];
			System.arraycopy(
				prevBuffer, 0, this.buffer_, 0, prevBuffer.length
			);
		}

		System.arraycopy(str, 0, buffer_, length_, str.length);
		this.length_ += str.length;

		return this;
	}

	public StringBuilder append(char c) {
		return append(new char[]{ c });
	}

	public StringBuilder append(int i) {
		return this; // TODO
	}

	@Override
	public String toString() {
		return new String(this.buffer_, 0, this.length_);
	}

	public int length() {
		return this.length_;
	}

	public int capacity() {
		return this.buffer_.length;
	}

}