package java.nio;

public abstract class ByteBuffer extends Buffer
{

	private final boolean readOnly_;
	protected ByteOrder endianness_ = ByteOrder.BIG_ENDIAN;

	protected ByteBuffer(
		int capacity, boolean readOnly, int offset
	) {
		super(capacity);
		this.readOnly_ = readOnly;
	}

	public static native ByteBuffer allocateDirect(int capacity);

	public static native ByteBuffer allocate(int capacity);

	public static native ByteBuffer wrap(byte[] array, int offset, int length);

	public static native ByteBuffer wrap(byte[] array);

	public abstract byte get();

	public abstract ByteBuffer put(byte b);

	public abstract byte get(int index);

	public abstract ByteBuffer put(int index, byte b);

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