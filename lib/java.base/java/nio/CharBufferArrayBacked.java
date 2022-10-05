package java.nio;

public class CharBufferArrayBacked extends CharBuffer {

	private final char[] data_;

	CharBufferArrayBacked(int capacity) {
		super(capacity);
		this.data_ = new char[capacity];
	}

	@Override
	public boolean isDirect() {
		return false;
	}

	@Override
	public boolean isReadOnly() {
		return false;
	}

	@Override
	public int capacity() {
		return data_.length;
	}

	@Override
	public char get() {
		if(!(this.position_ < this.limit_)) {
			throw new BufferUnderflowException();
		}
		return data_[this.position_++];
	}

	@Override
	public CharBuffer put(char c) {
		if(!(this.position_ < this.limit_)) {
			throw new BufferUnderflowException();
		}
		data_[this.position_++] = c;
		return this;
	}

	@Override
	public char get(int index) {
		if(index < 0 || !(index < this.limit_)) {
			throw new IndexOutOfBoundsException();
		}
		return data_[index];
	}

	@Override
	public CharBuffer put(int index, char c) {
		if(index < 0 || !(index < this.limit_)) {
			throw new IndexOutOfBoundsException();
		}
		data_[index] = c;
		return this;
	}

}