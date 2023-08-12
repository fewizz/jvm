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
		return append(str, 0, str.length);
	}

	public StringBuilder append(char[] str, int offset, int len) {
		if(offset < 0 || len < 0 || offset + len > str.length) {
			throw new IndexOutOfBoundsException();
		}

		int remaining = this.buffer_.length - length_;
		if(remaining < len) {
			char prevBuffer[] = this.buffer_;

			this.buffer_ = new char[this.buffer_.length + len];
			System.arraycopy(
				prevBuffer, 0, this.buffer_, 0, prevBuffer.length
			);
		}

		System.arraycopy(str, offset, buffer_, length_, len);
		this.length_ += len;

		return this;
	}

	public StringBuilder append(char c) {
		return append(new char[]{ c });
	}

	public StringBuilder append(int i) {
		return append(Integer.toString(i, 10));
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

	public int indexOf(String str) {
		return indexOf(str, 0);
	}

	public int indexOf(String str, int fromIndex) {
		int strlen = str.length();

		if(length_ - fromIndex < strlen) {
			return -1;
		}

		int ending = length_ - strlen + 1;
		for(int i = fromIndex; i < ending; ++i) {
			boolean found = true;

			for(int j = 0; j < strlen; ++j) {
				char a = this.buffer_[i + j];
				char b = str.charAt(j);
				if(a != b) {
					found = false;
					break;
				}
			}

			if(found) {
				return i;
			}
		}

		return -1;
	}

	public StringBuilder delete(int start, int end) {
		if(
			start < 0 || end < 0 ||
			start > length_ || end > length_ ||
			start > end
		) {
			throw new StringIndexOutOfBoundsException();
		}
		System.arraycopy(
			this.buffer_, end,
			this.buffer_, start,
			this.length_ - end
		);
		this.length_ -= end - start;
		return this;
	}

	public String substring(int start, int end) {
		if(
			start < 0 || end < 0 ||
			start > length_ || end > length_ ||
			start > end
		) {
			throw new StringIndexOutOfBoundsException();
		}
		return new String(this.buffer_, start, end - start);
	}

}