package java.nio;

public abstract class ByteBuffer
extends Buffer implements Comparable<ByteBuffer>
{

	private final boolean readOnly_;
	private final byte[] backingArray_;
	private final int offset_;
	protected ByteOrder endianness_ = ByteOrder.BIG_ENDIAN;

	protected ByteBuffer(
		int capacity, boolean readOnly, byte[] backingArray, int offset
	) {
		super(capacity);
		this.readOnly_ = readOnly;
		this.backingArray_ = backingArray;
		this.offset_ = offset;
	}

	public static native ByteBuffer allocateDirect(int capacity);

	public static native ByteBuffer allocate(int capacity);

	public static native ByteBuffer wrap(byte[] array, int offset, int length);

	public static native ByteBuffer wrap(byte[] array);

	@Override
	public abstract ByteBuffer slice();

	@Override
	public abstract ByteBuffer slice(int index, int length);

	@Override
	public abstract ByteBuffer duplicate();

	public abstract ByteBuffer asReadOnlyBuffer();

	public abstract byte get();

	public abstract ByteBuffer put(byte b);

	public abstract byte get(int index);

	public abstract ByteBuffer put(int index, byte b);

	public final boolean hasArray() {
		return this.backingArray_ != null && !this.readOnly_;
	}

	public final byte[] array() {
		if(this.backingArray_ == null) {
			throw new UnsupportedOperationException();
		}
		if(this.readOnly_) {
			throw new ReadOnlyBufferException();
		}
		return this.backingArray_;
	}

	public final int arrayOffset() {
		if(this.backingArray_ == null) {
			throw new UnsupportedOperationException();
		}
		if(this.readOnly_) {
			throw new ReadOnlyBufferException();
		}
		return this.offset_;
	}

	@Override
	public ByteBuffer position(int newPosition) {
		super.position(newPosition);
		return this;
	}

	@Override
	public ByteBuffer limit(int newLimit) {
		super.limit(newLimit);
		return this;
	}

	@Override
	public ByteBuffer mark() {
		super.mark();
		return this;
	}

	@Override
	public ByteBuffer reset() {
		super.reset();
		return this;
	}

	@Override
	public ByteBuffer clear() {
		super.clear();
		return this;
	}

	public ByteBuffer flip() {
		super.flip();
		return this;
	}

	public ByteBuffer rewind() {
		super.rewind();
		return this;
	}

	public abstract ByteBuffer compact();

	@Override
	public abstract boolean isDirect();

	@Override
	public String toString() {
		return super.toString() + 
			"(pos = " + this.position() +
			", limit = " + this.limit() +
			", capacity = " + this.capacity() + ")";
	}

	@Override
	public int hashCode() {
		int result = 1;
		for(int x = this.position(); x < this.limit(); ++x) {
			result = result * 31 + (int) this.get(x);
		}
		return result;
	}

	public final ByteOrder order() {
		return this.endianness_;
	}

	public final ByteBuffer order(ByteOrder bo) {
		this.endianness_ = bo;
		return this;
	}
	

}