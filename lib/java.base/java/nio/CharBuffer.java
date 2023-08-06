package java.nio;

public abstract class CharBuffer extends Buffer {

	protected CharBuffer(int capacity) {
		super(capacity);
	}

	public static CharBuffer allocate(int capacity) {
		return new CharBufferArrayBacked(capacity);
	}

	public static CharBuffer wrap(char[] array) {
		return new CharBufferArrayBacked(array);
	}

	public abstract char get();
	public abstract CharBuffer put(char c);

	public abstract char get(int index);
	public abstract CharBuffer put(int index, char c);

	public CharBuffer get(int index, char[] dst) {
		if((limit() - index) < dst.length) {
			throw new BufferUnderflowException();
		}
		for(int i = 0; i < dst.length; ++i) {
			dst[i] = get(index + i);
		}
		return this;
	}

	@Override
	public abstract char[] array();

	@Override
	public abstract int arrayOffset();

}