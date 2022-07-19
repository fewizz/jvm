package java.nio;

public abstract class ByteBuffer
	extends Buffer implements Comparable<ByteBuffer>
{

	protected ByteBuffer(int capacity) {
		super(capacity);
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

	public ByteBuffer get(byte[] dst, int offset, int length) {
		if(offset + length >= dst.length) {
			throw new IndexOutOfBoundsException();
		}

		if(dst.length > remaining()) {
			throw new BufferUnderflowException();
		}

		int end = length + offset;

		for (int x = offset; x < end; ++x) {
			dst[x] = get();
		}

		return this;
	}

	public ByteBuffer get(byte[] dst) {
		return get(dst, 0, dst.length);
	}

	public ByteBuffer get(int index, byte[] dst, int offset, int length) {
		if(offset + length >= dst.length) {
			throw new IndexOutOfBoundsException();
		}

		if(index + length > limit()) {
			throw new IndexOutOfBoundsException();
		}

		for (int x = 0; x < length; ++x) {
			dst[x + offset] = get(index + x);
		}

		return this;
	}

	public ByteBuffer get(int index, byte[] dst) {
		return get(index, dst, 0, dst.length);
	}

	

}