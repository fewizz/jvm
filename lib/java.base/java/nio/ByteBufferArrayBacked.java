package java.nio;

public class ByteBufferArrayBacked extends ByteBuffer {

	private final byte[] data_;

	ByteBufferArrayBacked(int capacity) {
		super(capacity);
		this.data_ = new byte[capacity];
	}

	ByteBufferArrayBacked(byte[] data) {
		super(data.length);
		this.data_ = data;
	}

	@Override
	public byte[] array() {
		return this.data_;
	}

	@Override
	public int arrayOffset() {
		return 0;
	}

	@Override
	public final int capacity() {
		return data_.length;
	}

	@Override
	public byte get() {
		if(!(this.position_ < this.limit_)) {
			throw new BufferUnderflowException();
		}
		byte r = data_[position_];
		position_++;
		return r;
	}

	@Override
	public ByteBuffer put(byte b) {
		if(!(this.position_ < this.limit_)) {
			throw new BufferUnderflowException();
		}
		data_[position_] = b;
		++position_;
		return this;
	}

	@Override
	public byte get(int index) {
		if(index < 0 || !(index < this.limit_)) {
			throw new IndexOutOfBoundsException();
		}
		return data_[index];
	}

	@Override
	public ByteBuffer put(int index, byte b) {
		if(index < 0 || !(index < this.limit_)) {
			throw new IndexOutOfBoundsException();
		}
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