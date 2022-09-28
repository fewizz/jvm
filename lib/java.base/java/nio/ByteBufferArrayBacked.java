package java.nio;

public class ByteBufferArrayBacked extends ByteBuffer {

	private final byte[] data_;

	ByteBufferArrayBacked(int capacity) {
		super(capacity, false, 0);
		data_ = new byte[capacity];
	}

	@Override
	public final int capacity() {
		return data_.length;
	}

	@Override
	public byte get() {
		byte r = data_[position_];
		position_++;
		return r;
	}

	@Override
	public ByteBuffer put(byte b) {
		data_[position_] = b;
		++position_;
		return this;
	}

	@Override
	public byte get(int index) {
		return data_[index];
	}

	@Override
	public ByteBuffer put(int index, byte b) {
		data_[index] = b;
		return this;
	}

	@Override
	public boolean isDirect() {
		return false;
	}

	@Override
	public boolean isReadOnly() {
		return false;
	}

}